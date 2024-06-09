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
#ifndef __ZEMB_LOGGER_H__
#define __ZEMB_LOGGER_H__

#include <map>
#include <memory>
#include <string>

#include "BaseType.h"
#include "DateTime.h"
#include "FileUtil.h"
#include "Singleton.h"
#include "ThreadUtil.h"

/**
 * @file Logger.h
 * @brief 一个简单的日志记录器
 */
namespace zemb {
#define LOG_INFO(logName, fmt, arg...)                                     \
    do {                                                                   \
        LoggerManager::getInstance().getLogger(logName)->log(              \
            "[%s]<I>" fmt, CSTR(zemb::DateTime::getDateTime().toString()), \
            ##arg);                                                        \
    } while (0);
#define LOG_WARN(logName, fmt, arg...)                                     \
    do {                                                                   \
        LoggerManager::getInstance().getLogger(logName)->log(              \
            "[%s]<W>" fmt, CSTR(zemb::DateTime::getDateTime().toString()), \
            ##arg);                                                        \
    } while (0);
#define LOG_ERR(logName, fmt, arg...)                                      \
    do {                                                                   \
        LoggerManager::getInstance().getLogger(logName)->log(              \
            "[%s]<E>" fmt, CSTR(zemb::DateTime::getDateTime().toString()), \
            ##arg);                                                        \
    } while (0);

/**
 * @class Logger
 * @brief 日志器
 */
class Logger {
    DECL_CLASSNAME(Logger)

public:
    Logger();
    ~Logger();
    /**
     * @brief 打开日志
     * @param logFileName 日志文件名
     * @param maxSize 日志文件最大大小
     * @param rotations 循环日志最大文件个数
     * @return true
     * @return false
     */
    bool open(const std::string& logFileName, uint32 maxSize,
              uint32 rotations = 0);
    /**
     * @brief 关闭日志
     */
    void close();
    /**
     * @brief 记录日志
     * @param format 格式化字符串
     * @param ... 可变参数列表
     */
    void log(const char* format, ...);

private:
    uint32 m_writens{0};
    uint32 m_maxSize{0};
    uint32 m_rotations{0};
    uint32 m_findex{0};
    std::string m_fileName{""};
    std::shared_ptr<File> m_file{nullptr};
    std::unique_ptr<char[]> m_logBufPtr;
    Mutex m_mutex;
};

/**
 * @class LoggerManager
 * @brief 日志管理器
 */
class LoggerManager : public Singleton<LoggerManager> {
    DECL_CLASSNAME(LoggerManager)
    DECL_SINGLETON(LoggerManager)

public:
    virtual ~LoggerManager();
    /**
     * @brief 设置日志根目录
     * @param logDir
     */
    void setRoot(const std::string& logDir);
    /**
     * @brief 获取日志器
     * @param logName
     * 日志名称(日志管理器会在日志根目录下自动创建logName.log的日志文件)
     * @param maxSize 单个文件最大大小
     * @param rotations
     * 循环记录文件个数(0:不循环记录,>=1:至多创建rotations个记录文件)
     * @return true
     * @return false
     * @note
     * 循环记录文件命名为logNameDD,DD是整数(例:rotations=1时,当logName达到maxSize后,
     *       将创建一个logName01文件用于循环记录)
     */
    bool createLogger(const std::string& logName, int maxSize,
                      int rotations = 0);
    /**
     * @brief 获取日志器
     * @param logName 日志名称
     * @return std::shared_ptr<Logger> 日志器
     * @note 日志管理器根据日志名称来区别日志
     */
    std::shared_ptr<Logger> getLogger(const std::string logName);

private:
    std::map<std::string, std::shared_ptr<Logger>> m_loggerMap;
    std::string m_logDir{""};
};

}  // namespace zemb

#endif
