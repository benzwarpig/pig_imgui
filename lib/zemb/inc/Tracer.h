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
#ifndef __ZEMB_TRACER_H__
#define __ZEMB_TRACER_H__

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "BaseType.h"
#include "DateTime.h"
#include "Singleton.h"
#include "Thread.h"
#include "ThreadUtil.h"

#define USE_ROS_LOG 0
/**
 *  @file   Tracer.h
 *  @brief  实现调试打印
 *  @note 打印等级高于当前等级的都能打印出来,
 *        如当前级别为1,则DBG(0)等级的信息不会被打印出来,
 *        大于等于当前等级的信息都能打印出来.默认为INFO(1)等级.
 */
#ifndef BUILD_REL_VERSION
#define BUILD_REL_VERSION 0 /**< 打开后只打印REL,WARN和ERR级别的打印 */
#endif

// 打印等级说明:只有大于或等于当前打印等级的消息才能被打印!
#define TRACE_LEVEL_DBG 0  /**< 用于调试信息的打印 */
#define TRACE_LEVEL_INFO 1 /**< 用于提示信息的打印 */
#define TRACE_LEVEL_REL 2  /**< 用于普通信息的打印 */
#define TRACE_LEVEL_WARN 3 /**< 用于警告信息打印 */
#define TRACE_LEVEL_ERR 4  /**< 用于错误信息的打印 */
#define TRACE_LEVEL_MIN TRACE_LEVEL_DBG
#define TRACE_LEVEL_MAX TRACE_LEVEL_ERR

/* linux标准错误字串 */
#define ERRSTR (strerror(errno))
#define ERRMSG(err) (strerror(err))

/**
 * @brief 可变参数用法
 * 1. __VA_ARGS__宏是C99标准定义的可变参数宏,目前也被C++11纳入标准了
 * 2. GNU GCC的可变参数用法：
 * #define TRACE_D(fmt,args...)
 * do{zemb::Tracer::getInstance().print(TRACE_LEVEL_DBG,"<D>"
 * fmt,##args);}while(0)
 * arg...这种用法是GCC特有的可变参数用法,从跨平台方面来说,__VA_ARGS__会好很多
 */

// PRINT族打印：无打印等级控制,可通过颜色识别信息等级.
#define PRINT(fmt, ...)             \
    do {                            \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)

#define PRINT_RED(fmt, ...)                                       \
    do {                                                          \
        printf("\033[31m\033[1m" fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define PRINT_GREEN(fmt, ...)                                     \
    do {                                                          \
        printf("\033[32m\033[1m" fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define PRINT_YELLOW(fmt, ...)                                    \
    do {                                                          \
        printf("\033[33m\033[1m" fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define PRINT_PINK(fmt, ...)                                      \
    do {                                                          \
        printf("\033[35m\033[1m" fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define PRINT_CYAN(fmt, ...)                                      \
    do {                                                          \
        printf("\033[36m\033[1m" fmt "\033[0m\n", ##__VA_ARGS__); \
    } while (0)

#define PRINT_DBG(fmt, ...) \
    { PRINT_GREEN("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define PRINT_INFO(fmt, ...) \
    { PRINT_CYAN("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define PRINT_WARN(fmt, ...) \
    { PRINT_YELLOW("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define PRINT_ERR(fmt, ...) \
    { PRINT_RED("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

// TRACE族打印：有打印等级控制,可通过字符标识和颜色识别信息等级.
// TRACE_x        : 打印日志级别
// TRACE_xxx      : 打印日志级别+函数名+行号
// TRACE_xxx_CLASS: 打印日志级别+类名+函数名+行号
#define TRACE_D(fmt, ...)                                                    \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            TRACE_LEVEL_DBG, "<D>%s " fmt,                                   \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_E(fmt, ...)                                                    \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            TRACE_LEVEL_ERR, "<E>%s " fmt,                                   \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_W(fmt, ...)                                                    \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            TRACE_LEVEL_WARN, "<W>%s " fmt,                                  \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_I(fmt, ...)                                                    \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            TRACE_LEVEL_INFO, "<I>%s " fmt,                                  \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_R(fmt, ...)                                                    \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            TRACE_LEVEL_REL, "<R>%s " fmt,                                   \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_L(uid, fmt, ...)                                               \
    do {                                                                     \
        zemb::Tracer::getInstance().print(                                   \
            uid, "<L%d>%s@" fmt, uid,                                        \
            zemb::DateTime::getDateTime().toString().data(), ##__VA_ARGS__); \
    } while (0)

#define TRACE_DBG(fmt, ...) \
    { TRACE_D("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define TRACE_WARN(fmt, ...) \
    { TRACE_W("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define TRACE_ERR(fmt, ...) \
    { TRACE_E("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define TRACE_INFO(fmt, ...) \
    { TRACE_I("%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

#define TRACE_REL(fmt, ...) \
    { TRACE_R("%s:" fmt, __FUNCTION__, ##__VA_ARGS__); }

// 扩展日志信息打印(uid必须大于TRACE_LEVEL_MAX)
#define TRACE_EXT(uid, fmt, ...) \
    { TRACE_L(uid, "%s,L%d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); }

// 带类名的TRACE族打印：仅在C++类中可使用,需要this指针.
#define TRACE_DBG_CLASS(fmt, ...)                                         \
    {                                                                     \
        TRACE_D("%s::%s,L%d:" fmt, CSTR(this->className()), __FUNCTION__, \
                __LINE__, ##__VA_ARGS__);                                 \
    }

#define TRACE_WARN_CLASS(fmt, ...)                                        \
    {                                                                     \
        TRACE_W("%s::%s,L%d:" fmt, CSTR(this->className()), __FUNCTION__, \
                __LINE__, ##__VA_ARGS__);                                 \
    }

#define TRACE_ERR_CLASS(fmt, ...)                                         \
    {                                                                     \
        TRACE_E("%s::%s,L%d:" fmt, CSTR(this->className()), __FUNCTION__, \
                __LINE__, ##__VA_ARGS__);                                 \
    }

#define TRACE_INFO_CLASS(fmt, ...)                                        \
    {                                                                     \
        TRACE_I("%s::%s,L%d:" fmt, CSTR(this->className()), __FUNCTION__, \
                __LINE__, ##__VA_ARGS__);                                 \
    }

#define TRACE_REL_CLASS(fmt, ...)                                     \
    {                                                                 \
        TRACE_R("%s::%s:" fmt, CSTR(this->className()), __FUNCTION__, \
                ##__VA_ARGS__);                                       \
    }

// 扩展日志信息打印(uid必须大于TRACE_LEVEL_MAX)
#define TRACE_EXT_CLASS(uid, fmt, ...)                                         \
    {                                                                          \
        TRACE_L(uid, "%s::%s,L%d:" fmt, CSTR(this->className()), __FUNCTION__, \
                __LINE__, ##__VA_ARGS__);                                      \
    }

// 条件断言打印.
#define TRACE_IF(condition)                                                   \
    do {                                                                      \
        if ((condition)) {                                                    \
            TRACE_RED("if(%s)<@%s,L%d>", #condition, __FUNCTION__, __LINE__); \
        }                                                                     \
    } while (0)

#define TRACE_IF_CLASS(condition)                                       \
    do {                                                                \
        if ((condition)) {                                              \
            TRACE_RED("if(%s)<@%s::%s,L%d>", #condition,                \
                      CSTR(this->className()), __FUNCTION__, __LINE__); \
        }                                                               \
    } while (0)

#define TRACE_ASSERT(condition)            \
    do {                                   \
        if (!(condition)) {                \
            TRACE_ERR("Assert !!!");       \
            for (;;) {                     \
                zemb::Thread::msleep(100); \
            }                              \
        }                                  \
    } while (0)

// 按颜色打印
#define TRACE_RED(fmt, ...)                                                   \
    do {                                                                      \
        zemb::Tracer::getInstance().print(TRACE_LEVEL_REL,                    \
                                          "<C>\033[31m\033[1m" fmt "\033[0m", \
                                          ##__VA_ARGS__);                     \
    } while (0)

#define TRACE_GREEN(fmt, ...)                                                 \
    do {                                                                      \
        zemb::Tracer::getInstance().print(TRACE_LEVEL_REL,                    \
                                          "<C>\033[32m\033[1m" fmt "\033[0m", \
                                          ##__VA_ARGS__);                     \
    } while (0)

#define TRACE_YELLOW(fmt, ...)                                                \
    do {                                                                      \
        zemb::Tracer::getInstance().print(TRACE_LEVEL_REL,                    \
                                          "<C>\033[33m\033[1m" fmt "\033[0m", \
                                          ##__VA_ARGS__);                     \
    } while (0)

#define TRACE_PINK(fmt, ...)                                                  \
    do {                                                                      \
        zemb::Tracer::getInstance().print(TRACE_LEVEL_REL,                    \
                                          "<C>\033[35m\033[1m" fmt "\033[0m", \
                                          ##__VA_ARGS__);                     \
    } while (0)

#define TRACE_CYAN(fmt, ...)                                                  \
    do {                                                                      \
        zemb::Tracer::getInstance().print(TRACE_LEVEL_REL,                    \
                                          "<C>\033[36m\033[1m" fmt "\033[0m", \
                                          ##__VA_ARGS__);                     \
    } while (0)

/* 发布版本要去除Debug级别的打印 */
#if BUILD_REL_VERSION
#include "TracerLess.h"
#endif

namespace zemb {
class TracerSink;
/**
 *  @class  Tracer
 *  @brief  调试跟踪类
 *  @note   用于程序调试
 */
class Tracer : public Singleton<Tracer>, public Runnable {
    DECL_CLASSNAME(Tracer)
    DECL_SINGLETON(Tracer)

public:
    ~Tracer();
    /**
     *  @brief  调试信息打印
     *  @param  level 打印级别
     *  @param  format 格式化字串
     *  @return void
     */
    void print(int level, const char* format, ...);
    /**
     *  @brief  设置打印级别
     *  @param  level 打印级别
     *  @return void
     *  @note   默认打印级别为TRACE_LEVEL_INFO(不打印DBG信息)
     */
    Tracer& setLevel(int level);
    /**
     *  @brief  获取打印级别
     *  @param  void
     *  @return int 当前打印级别
     */
    int getLevel();
    /**
     * @brief 增加日志输出器
     * @param sink
     */
    Tracer& addSink(std::shared_ptr<TracerSink> sink);
    /**
     * @brief 启动日志
     */
    void start();

private:
    void run(const Thread& thread);
    void sinkMsg();

private:
    static const int TRACE_MAXLEN =
        4096; /* 打印长度最大为4096Bytes,超出后显示省略号 */
    int m_level{TRACE_LEVEL_INFO};
    bool m_hasSTDSink{false};
    bool m_isStart{false};
    Thread m_thread;
    std::mutex m_logMutex;
    std::mutex m_sinkMutex;
    std::vector<std::string> m_logVect;
    std::vector<std::shared_ptr<TracerSink>> m_sinkVect;
};

class TracerSink {
    DECL_CLASSNAME(TracerSink)
public:
    TracerSink() = default;
    virtual ~TracerSink() = default;
    /* 设置日志级别 */
    virtual void setLevel(int level) {}
    /**
     * @brief 日志输出
     * @param uid 日志唯一标识(0~4:标准输出日志;
     * 5~n:自定义日志,需使用TRACE_EXT输出)
     * @param msg 日志信息
     * @note
     * 日志输出器可以根据uid来过滤要输出的日志,注意:sink方法中不能使用TRACE,否则会死锁!!!
     */
    virtual void sink(int uid, const std::string& msg) = 0;
};

/**
 * @brief 标准输出日志(stdout)
 * @class STDSink
 */
class STDSink : public TracerSink {
    DECL_CLASSNAME(STDSink)
public:
    STDSink();
    virtual ~STDSink();
    void sink(int uid, const std::string& msg) override;
};

/**
 * @brief 系统日志(使用systemd-journald)
 * @class SyslogSink
 */
class SyslogSink : public TracerSink {
    DECL_CLASSNAME(SyslogSink)
public:
    SyslogSink();
    virtual ~SyslogSink();
    void sink(int uid, const std::string& msg) override;
};

#if USE_ROS_LOG
class ROSSink : public TracerSink {
    DECL_CLASSNAME(ROSSink)
public:
    ROSSink();
    virtual ~ROSSink();
    void setLevel(int level) override;
    void sink(int uid, const std::string& msg) override;
};
#endif
}  // namespace zemb
#endif
