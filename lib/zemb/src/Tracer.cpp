/******************************************************************************
 * This file is part of ZEMB.
 *
 * ZEMB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ZEMB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZEMB.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Project: zemb
 * Author : FergusZeng
 * Email  : cblock@126.com
 * git	  : https://gitee.com/newgolo/embedme.git
 * Copyright 2014~2022 @ ShenZhen ,China
 *******************************************************************************/
#include "Tracer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <iostream>

#include "Socket.h"
#include "StrUtil.h"
#include "SysUtil.h"

#if USE_ROS_LOG
#include <ros/console.h>
#include <ros/ros.h>
#endif

namespace zemb {
Tracer::Tracer() { setvbuf(stdout, nullptr, _IONBF, BUFSIZ); }

Tracer::~Tracer() {
    m_thread.stop();
    Thread::msleep(1000);
    sinkMsg();  // 打印最后的消息
}

void Tracer::print(int level, const char* format, ...) {
    if (level < m_level) { /* 打印级别大于等于当前级别的才允许打印 */
        return;
    }
    auto printBuf = std::make_unique<char[]>(TRACE_MAXLEN);
    char* buf = printBuf.get();
    va_list argp;
    va_start(argp, format);
    int size = TRACE_MAXLEN - 1;
    size = vsnprintf(buf, size, format, argp);
    va_end(argp);
    if (size > 0) {
        if (size >= TRACE_MAXLEN) { /* 超过长度了 */
            buf[TRACE_MAXLEN - 4] = '.';
            buf[TRACE_MAXLEN - 3] = '.';
            buf[TRACE_MAXLEN - 2] = '.';
            buf[TRACE_MAXLEN - 1] = 0;
            size = TRACE_MAXLEN;
        }
        std::string out(buf, size);
        std::unique_lock<std::mutex> lock(m_logMutex);
        m_logVect.push_back(out);
    }
}

Tracer& Tracer::setLevel(int level) {
    std::unique_lock<std::mutex> lock(m_sinkMutex);
    m_level = CLIP(TRACE_LEVEL_MIN, level, TRACE_LEVEL_MAX);
    for (auto& sink : m_sinkVect) {
        sink->setLevel(m_level);
    }
    return *this;
}

int Tracer::getLevel() { return m_level; }

Tracer& Tracer::addSink(std::shared_ptr<TracerSink> sink) {
    // 只允许加入一个STDSink
    if (sink->className() == "STDSink" && m_hasSTDSink) {
        sink->setLevel(m_level);
        return *this;
    }
    sink->setLevel(m_level);
    std::unique_lock<std::mutex> lock(m_sinkMutex);
    m_sinkVect.push_back(std::move(sink));
    return *this;
}

void Tracer::run(const Thread& thread) {
    while (thread.isRunning()) {
        if (m_logVect.empty()) {
            Thread::msleep(10);
            continue;
        }
        sinkMsg();
    }
}

void Tracer::sinkMsg() {
    std::unique_lock<std::mutex> lock(m_logMutex);
    for (auto log : m_logVect) {
        int uid = 0;
        switch (log[1]) {
            case 'D':
                uid = TRACE_LEVEL_DBG;
                break;
            case 'I':
                uid = TRACE_LEVEL_INFO;
                break;
            case 'R':
                uid = TRACE_LEVEL_REL;
                break;
            case 'W':
                uid = TRACE_LEVEL_WARN;
                break;
            case 'E':
                uid = TRACE_LEVEL_ERR;
                break;
            case 'C':
                uid = TRACE_LEVEL_REL;
                log = log.substr(3);
                break;
            case 'L': {
                uid = std::stoi(StrUtil::findString(log, "<", ">").substr(2));
                if (uid <= TRACE_LEVEL_MAX) {
                    continue;
                }
                break;
            }
            default:
                continue;
        }
        std::unique_lock<std::mutex> lock(m_sinkMutex);
        for (auto& sink : m_sinkVect) {
            sink->sink(uid, log);
        }
    }
    m_logVect.clear();
}

void Tracer::start() {
    if (!m_isStart) {
        m_isStart = true;
        m_thread.start(*this);
    }
}

STDSink::STDSink() {}

STDSink::~STDSink() {}

void STDSink::sink(int uid, const std::string& msg) {
    if (uid < TRACE_LEVEL_MIN || uid > TRACE_LEVEL_MAX) {
        return;
    }
    std::string color = "";
    switch (uid) {
        case TRACE_LEVEL_DBG:
            color = "\033[36m\033[1m";
            break;
        case TRACE_LEVEL_WARN:
            color = "\033[33m\033[1m";
            break;
        case TRACE_LEVEL_ERR:
            color = "\033[31m\033[1m";
            break;
        case TRACE_LEVEL_INFO:
            color = "\033[32m\033[1m";
            break;
        case TRACE_LEVEL_REL:
            std::cout << msg << std::endl;
            return;
        default:
            return;
    }
    std::cout << color << msg << "\033[0m" << std::endl;
}

SyslogSink::SyslogSink() {}

SyslogSink::~SyslogSink() {}

void SyslogSink::sink(int uid, const std::string& msg) {
    if (uid < TRACE_LEVEL_MIN || uid > TRACE_LEVEL_MAX) {
        return;
    }
    int priority = LOG_DEBUG;
    switch (uid) {
        case TRACE_LEVEL_WARN:
            priority = LOG_WARNING;
            break;
        case TRACE_LEVEL_ERR:
            priority = LOG_ERR;
            break;
        case TRACE_LEVEL_INFO:
            priority = LOG_NOTICE;
            break;
        case TRACE_LEVEL_REL:
            priority = LOG_INFO;
            break;
        default:
            break;
    }
    syslog(priority, "%s", CSTR(msg));
}

#if USE_ROS_LOG
ROSSink::ROSSink() {}

ROSSink::~ROSSink() {}

void ROSSink::setLevel(int level) {
    ros::console::levels::Level lvl = ros::console::levels::Info;
    switch (level) {
        case TRACE_LEVEL_DBG:
            lvl = ros::console::levels::Debug;
            break;
        case TRACE_LEVEL_WARN:
            lvl = ros::console::levels::Warn;
            break;
        case TRACE_LEVEL_ERR:
            lvl = ros::console::levels::Error;
            break;
        case TRACE_LEVEL_INFO:
            lvl = ros::console::levels::Info;
            break;
        default:
            break;
    }
    if (ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, lvl)) {
        ros::console::notifyLoggerLevelsChanged();
    }
}

void ROSSink::sink(int uid, const std::string& msg) {
    switch (uid) {
        case TRACE_LEVEL_DBG:
            ROS_DEBUG_STREAM(msg);
            break;
        case TRACE_LEVEL_ERR:
            ROS_ERROR_STREAM(msg);
            break;
        case TRACE_LEVEL_INFO:
        case TRACE_LEVEL_REL:
            ROS_INFO_STREAM(msg);
            break;
        case TRACE_LEVEL_WARN:
            ROS_WARN_STREAM(msg);
            break;
        default:
            // ROS_FATAL_STREAM(msg);
            break;
    }
}
#endif
}  // namespace zemb
