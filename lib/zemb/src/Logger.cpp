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
#include "Logger.h"

#include "StrUtil.h"
#include "Tracer.h"
namespace zemb {
#define LOG_BUFLEN_MAX 1024
#define LOG_ROOT_DIR "/tmp/log/"
Logger::Logger() { m_logBufPtr = std::make_unique<char[]>(LOG_BUFLEN_MAX); }

Logger::~Logger() {}

bool Logger::open(const std::string& logFileName, uint32 maxSize,
                  uint32 rotations) {
    AutoLock lock(m_mutex);
    if (m_file != nullptr) {
        TRACE_ERR_CLASS("logger is allready open!");
        return false;
    }
    m_file = std::make_shared<File>();
    if (!m_file->open(logFileName, IO_MODE_REWR_ORNEW)) {
        return false;
    }
    m_fileName = logFileName;
    m_maxSize = maxSize;
    m_rotations = rotations;
    return true;
}
void Logger::close() {
    AutoLock lock(m_mutex);
    m_file = nullptr;
}

void Logger::log(const char* format, ...) {
    AutoLock lock(m_mutex);
    va_list argp;
    va_start(argp, format);
    int size = LOG_BUFLEN_MAX - 1;
    char* buf = m_logBufPtr.get();
    size = vsnprintf(buf, size, format, argp);
    va_end(argp);
    if (size > 0) {
        if (size >= LOG_BUFLEN_MAX) { /* 超过长度了 */
            buf[LOG_BUFLEN_MAX - 5] = '.';
            buf[LOG_BUFLEN_MAX - 4] = '.';
            buf[LOG_BUFLEN_MAX - 3] = '.';
            buf[LOG_BUFLEN_MAX - 2] = '\n';
            buf[LOG_BUFLEN_MAX - 1] = 0;
            size = LOG_BUFLEN_MAX;
        }
        int len = m_file->writeData(buf, size);
        if (len > 0) {
            m_writens += len;
        }
        /* 到达最大文件大小,重命名文件并创建新日志文件 */
        if (m_maxSize > 0 && m_writens >= m_maxSize) {
            m_file->close();
            FilePath path(m_fileName);
            std::string dirName = path.dirName();
            std::string fileName = path.baseName();
            std::string suffix = StrUtil::suffix(fileName, ".");
            if (!suffix.empty()) {
                std::string newFile =
                    fileName.substr(0, fileName.size() - suffix.size());
                m_findex %= m_rotations;
                // TRACE_DBG_CLASS("fileName:%s, newFile:%s,
                // suffix:%s",CSTR(fileName),CSTR(newFile),CSTR(suffix));
                newFile = StrUtil::format("%s%02d%s", CSTR(newFile), ++m_findex,
                                          CSTR(suffix));
                File::renameFile(m_fileName,
                                 dirName.append("/").append(newFile));
                m_writens = 0;
            }
            if (!m_file->open(m_fileName, IO_MODE_REWR_ORNEW)) {
                TRACE_ERR_CLASS("open log file:%s error!", CSTR(m_fileName));
            }
        }
    }
}

LoggerManager::LoggerManager() {}

LoggerManager::~LoggerManager() {
    for (auto kv : m_loggerMap) {
        kv.second->close();
    }
    m_loggerMap.clear();
}

void LoggerManager::setRoot(const std::string& logDir) { m_logDir = logDir; }

bool LoggerManager::createLogger(const std::string& logName, int maxSize,
                                 int rotations) {
    if (m_logDir.empty()) {
        m_logDir = LOG_ROOT_DIR;
    }
    if (Directory::exists(CSTR(m_logDir)) ||
        Directory::createDir(CSTR(m_logDir), 0666, true)) {
        std::string logFileName =
            m_logDir + std::string("/") + logName + std::string(".log");
        auto iter = m_loggerMap.find(logName);
        if (iter != m_loggerMap.end()) {
            return true;
        }
        auto logger = std::make_shared<Logger>();
        if (!logger->open(logFileName, maxSize, rotations)) {
            TRACE_ERR_CLASS("cannot create log: %s", CSTR(logName));
            return false;
        }
        m_loggerMap.insert(std::make_pair(logName, logger));
        return true;
    }
    TRACE_ERR_CLASS("create log error, no log dir:%s", CSTR(m_logDir));
    return false;
}

std::shared_ptr<Logger> LoggerManager::getLogger(const std::string logName) {
    auto iter = m_loggerMap.find(logName);
    if (iter != m_loggerMap.end()) {
        return iter->second;
    }
    return nullptr;
}
}  // namespace zemb
