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
#ifndef __ZEMB_DATE_TIME_H__
#define __ZEMB_DATE_TIME_H__

#include <iostream>
#include <string>

#include "BaseType.h"

/**
 * @file DateTime.h
 * @brief 日期时间
 */
namespace zemb {

/**
 * @class Time
 * @brief 时间类
 */
class Time {
public:
    explicit Time(uint64 us = 0);
    Time(int sec, int us);
    virtual ~Time();
    /**
     * @brief 获取秒数
     * @return sint64
     */
    int secPart() const;
    /**
     * @brief 获取微秒数
     * @return int
     */
    int usPart() const;
    /**
     * @brief 时间转换成字符串
     * @return std::string
     */
    std::string toString();
    /**
     * @brief 转换成微秒数
     * @return uint64
     */
    uint64 toMicroSec();
    /**
     * @brief 当前时间(从1970-01-01 00:00:00 UTC开始)
     * @return Time
     */
    static Time fromEpoch();
    /**
     * @brief 当前时间(从系统开机的时刻算起)
     * @return Time
     */
    static Time fromMono();
    /**
     * @brief 获取时间间隔
     * @param monoStartTime 起始时间
     * @return uint64
     */
    static uint64 usSinceMono(const Time& monoStartTime);
    Time operator+(const Time& t);
    Time operator-(const Time& t);
    bool operator==(const Time& t) const;

private:
    int m_seconds{0};
    int m_microSeconds{0};
};

/**
 *  @class  DateTime
 *  @brief  日期时间结构体
 */
class DateTime {
public:
    DateTime();
    explicit DateTime(const Time& epoch);
    DateTime(unsigned int year, unsigned int month, unsigned int day,
             unsigned int hour, unsigned int minute, unsigned int second);
    DateTime(const DateTime& copy);
    ~DateTime();
    /**
     * @brief 年份(0~9999)
     * @return unsigned int
     */
    unsigned int year() const;
    /**
     * @brief 月份(1~12)
     * @return unsigned int
     */
    unsigned int month() const;
    /**
     * @brief 日期(1~31)
     * @return unsigned int
     */
    unsigned int day() const;
    /**
     * @brief 小时(0~23)
     * @return unsigned int
     */
    unsigned int hour() const;
    /**
     * @brief 分钟(0~59)
     * @return unsigned int
     */
    unsigned int minute() const;
    /**
     * @brief 秒(0~59)
     * @return unsigned int
     */
    unsigned int second() const;
    /**
     * @brief 毫秒(0~999)
     * @return unsigned int
     */
    unsigned int msecond() const;
    /**
     * @brief 星期(0~6:星期日~星期六)
     * @return unsigned int
     */
    unsigned int weekday() const;
    /**
     * @brief 设置年份
     * @param year
     * @return DateTime&
     */
    DateTime& setYear(unsigned int year);
    /**
     * @brief 设置月份
     * @param month
     * @return DateTime&
     */
    DateTime& setMonth(unsigned int month);
    /**
     * @brief 设置日期
     * @param date
     * @return DateTime&
     */
    DateTime& setDay(unsigned int date);
    /**
     * @brief 设置小时
     * @param hour
     * @return DateTime&
     */
    DateTime& setHour(unsigned int hour);
    /**
     * @brief 设置分钟
     * @param minute
     * @return DateTime&
     */
    DateTime& setMinute(unsigned int minute);
    /**
     * @brief 设置秒数
     * @param second
     * @return DateTime&
     */
    DateTime& setSecond(unsigned int second);
    /**
     * @brief 设置毫秒数
     * @param microSec
     * @return DateTime&
     */
    DateTime& setMSecond(unsigned int microSec);
    /**
     * @brief 转换为字符串格式:"yyyy-mm-dd hh:MM:ss"
     * @return std::string
     */
    std::string toString();
    /**
     * @brief 判断是否是润年
     * @return true
     * @return false
     */
    bool isLeapYear();
    /**
     * @brief 获取当前日期时间
     * @return DateTime
     */
    static DateTime getDateTime();
    /**
     * @brief 设置当前时间
     * @param dateTime
     * @return true
     * @return false
     */
    static bool setDateTime(const DateTime& dateTime);
    /**
     * @brief 获取RTC时间
     * @return DateTime
     */
    static DateTime getRTCDateTime();
    /**
     * @brief 设置RTC时间
     * @param dateTime
     * @return true
     * @return false
     */
    static bool setRTCDateTime(const DateTime& dateTime);
    /**
     * @brief 重载等号
     * @param dt
     */
    bool operator==(const DateTime& dt) const;

private:
    unsigned int m_msecond{0};
    unsigned int m_second{0};
    unsigned int m_minute{0};
    unsigned int m_hour{0};
    unsigned int m_day{1};
    unsigned int m_month{1};
    unsigned int m_year{2000};
};
}  // namespace zemb
#endif
