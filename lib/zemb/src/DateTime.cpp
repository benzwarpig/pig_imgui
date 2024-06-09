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
#include "DateTime.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "StrUtil.h"
#include "Tracer.h"  // 不允许使用Tracer,因为Tracer中使用了时间戳,会形成死循环

#if defined(OS_CYGWIN) || defined(OS_ANDROID)
#define USE_RTC 0
#else
#include <linux/rtc.h>
#define RTC_DEV_PATH "/dev/rtc0"
#define USE_RTC 1
#endif

namespace zemb {
Time::Time(uint64 us) {
    m_seconds = us / 1000000;
    m_microSeconds = us % 1000000;
}

Time::Time(int sec, int us) {
    m_seconds = sec + us / 1000000;
    m_microSeconds = us % 1000000;
}

Time::~Time() {}

int Time::secPart() const { return m_seconds; }

int Time::usPart() const { return m_microSeconds; }

std::string Time::toString() {
    return StrUtil::format("%ld.%06d s", m_seconds, m_microSeconds);
}

uint64 Time::toMicroSec() {
    uint64 microSeconds = m_seconds * 1000000LL + m_microSeconds;
    return microSeconds;
}

Time Time::fromEpoch() {
    Time currTime;
    struct timespec currentTs;
    clock_gettime(CLOCK_REALTIME, &currentTs);
    currTime.m_seconds = currentTs.tv_sec;
    currTime.m_microSeconds = static_cast<int>(currentTs.tv_nsec / 1000);
    return currTime;
}

Time Time::fromMono() {
    Time currTime;
    struct timespec currentTs;
    clock_gettime(CLOCK_MONOTONIC, &currentTs);
    currTime.m_seconds = currentTs.tv_sec;
    currTime.m_microSeconds = static_cast<int>(currentTs.tv_nsec / 1000);
    return currTime;
}

uint64 Time::usSinceMono(const Time& monoStartTime) {
    Time diffTime = Time::fromMono() - monoStartTime;
    return diffTime.toMicroSec();
}

Time Time::operator+(const Time& time) {
    Time result;
    result.m_seconds = m_seconds + time.m_seconds;
    result.m_microSeconds = m_microSeconds + time.m_microSeconds;
    while (result.m_microSeconds >= 1000000) {
        result.m_seconds++;
        result.m_microSeconds -= 1000000;
    }
    return result;
}

Time Time::operator-(const Time& time) {
    Time result;
    result.m_microSeconds = m_microSeconds - time.m_microSeconds;
    if (result.m_microSeconds < 0) {
        result.m_microSeconds += 1000000;
        result.m_seconds = m_seconds - time.m_seconds - 1;
    } else {
        result.m_seconds = m_seconds - time.m_seconds;
    }
    return result;
}

bool Time::operator==(const Time& t) const {
    return ((t.m_seconds == this->m_seconds) &&
            (t.m_microSeconds == this->m_microSeconds));
}

DateTime::DateTime() {}

DateTime::DateTime(const Time& epoch) {
    time_t t = epoch.secPart();
    struct tm tmTime;
    localtime_r(&t, &tmTime);
    setMSecond(epoch.usPart() / 1000);
    setSecond(tmTime.tm_sec);
    setMinute(tmTime.tm_min);
    setHour(tmTime.tm_hour);
    setDay(tmTime.tm_mday);
    setMonth(tmTime.tm_mon + 1);
    setYear(tmTime.tm_year + 1900);
}

DateTime::DateTime(unsigned int year, unsigned int month, unsigned int day,
                   unsigned int hour, unsigned int minute,
                   unsigned int second) {
    setYear(year);
    setMonth(month);
    setDay(day);
    setHour(hour);
    setMinute(minute);
    setSecond(second);
}

DateTime::DateTime(const DateTime& copy) {
    m_year = copy.m_year;
    m_month = copy.m_month;
    m_day = copy.m_day;
    m_hour = copy.m_hour;
    m_minute = copy.m_minute;
    m_second = copy.m_second;
}
DateTime::~DateTime() {}
unsigned int DateTime::year() const { return m_year; }
unsigned int DateTime::month() const { return m_month; }
unsigned int DateTime::day() const { return m_day; }
unsigned int DateTime::hour() const { return m_hour; }
unsigned int DateTime::minute() const { return m_minute; }
unsigned int DateTime::second() const { return m_second; }

unsigned int DateTime::msecond() const { return m_msecond; }

unsigned int DateTime::weekday() const {
    /* 蔡勒(Zeller)公式:
     * 1582年10月4日或之前: w=y+[y/4]+[c/4]-2c+[13(m+1)/5]+d+2
     * 1582年10月4日之后: w=y+[y/4]+[c/4]-2c+[13(m+1)/5]+d-1
     * y为年份后两位,c为年份前两位,m取值为3~14(1/2月份看作是前一年的13/14月份),d为日期
     * 2020-02-14: 19+4+5-40+39+14-1=40  40%7=5
     * 如果结果是负数,需转换为正数: Week = (w%7+7)%7
     */
    int tmp, y, c, m, d;
    if (m_month <= 2) {
        tmp = m_year - 1;
        m = 12 + m_month;
    } else {
        tmp = m_year;
        m = m_month;
    }
    y = tmp % 100;
    c = tmp / 100;
    d = m_day;
    tmp = m_year * 10000 + m_month * 100 + m_day;
    if (tmp <= 15821004) {
        tmp = y + y / 4 + c / 4 - (c << 1) + (13 * (m + 1) / 5) + d + 2;
    } else {
        tmp = y + y / 4 + c / 4 - (c << 1) + (13 * (m + 1) / 5) + d - 1;
    }
    return (tmp < 0) ? (((tmp % 7) + 7) % 7) : (tmp % 7);
}

DateTime& DateTime::setYear(unsigned int year) {
    if (year >= 1970 && year < 2100) {
        m_year = year;
    }
    return *this;
}
DateTime& DateTime::setMonth(unsigned int month) {
    if (month <= 12) {
        m_month = month;
    }
    return *this;
}
DateTime& DateTime::setDay(unsigned int date) {
    if (date <= 31) {
        m_day = date;
    }
    return *this;
}
DateTime& DateTime::setHour(unsigned int hour) {
    if (hour < 24) {
        m_hour = hour;
    }
    return *this;
}
DateTime& DateTime::setMinute(unsigned int minute) {
    if (minute < 60) {
        m_minute = minute;
    }
    return *this;
}
DateTime& DateTime::setSecond(unsigned int second) {
    if (second < 60) {
        m_second = second;
    }
    return *this;
}

DateTime& DateTime::setMSecond(unsigned int msecond) {
    if (msecond < 1000) {
        m_msecond = msecond;
    }
    return *this;
}

std::string DateTime::toString() {
    return StrUtil::format("%d-%02d-%02d %02d:%02d:%02d.%03d", m_year, m_month,
                           m_day, m_hour, m_minute, m_second, m_msecond);
}

bool DateTime::isLeapYear() {
    if ((m_year % 4 == 0) && (m_year % 100 != 0)) {
        return true;
    }
    return false;
}

DateTime DateTime::getDateTime() {
    DateTime dateTime;
    time_t nowTime = ::time(nullptr);
    struct tm tmTime;
    localtime_r(&nowTime, &tmTime);
    Time time = Time::fromEpoch();
    dateTime.setMSecond(time.usPart() / 1000);
    dateTime.setSecond(tmTime.tm_sec);
    dateTime.setMinute(tmTime.tm_min);
    dateTime.setHour(tmTime.tm_hour);
    dateTime.setDay(tmTime.tm_mday);
    dateTime.setMonth(tmTime.tm_mon + 1);
    dateTime.setYear(tmTime.tm_year + 1900);
    return dateTime;
}

bool DateTime::setDateTime(const DateTime& dateTime) {
    struct tm currentTime;
    currentTime.tm_year = dateTime.year() - 1900;
    currentTime.tm_mon = dateTime.month() - 1;
    currentTime.tm_mday = dateTime.day();
    currentTime.tm_hour = dateTime.hour();
    currentTime.tm_min = dateTime.minute();
    currentTime.tm_sec = dateTime.second();
    currentTime.tm_isdst = 0;
    time_t seconds = mktime(&currentTime);
    if (seconds < 0) {
        TRACE_ERR("DateTime::setDateTime,make time error.");
        return false;
    }
    struct timeval tv;
    struct timezone tz;
    if (gettimeofday(&tv, &tz) < 0) {
        TRACE_ERR("DateTime::setDateTime,get time error.");
        return false;
    }
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (settimeofday(&tv, &tz) < 0) {
        TRACE_ERR("DateTime::setDateTime,set time error.");
        return false;
    }
    return true;
}

DateTime DateTime::getRTCDateTime() {
    DateTime rtcTime;
#if USE_RTC
    int rtcFd = ::open(RTC_DEV_PATH, O_RDWR);
    if (rtcFd < 0) {
        TRACE_ERR("DateTime::getRTCDateTime,open rtc error:%s.", ERRSTR);
        return getDateTime();
    }
    struct rtc_time rtc_tm;
    int rc = ::ioctl(rtcFd, RTC_RD_TIME, &rtc_tm);
    if (rc < 0) {
        TRACE_ERR("DateTime::getRTCDateTime,read rtc error:%s.", ERRSTR);
        close(rtcFd);
        return getDateTime();
    }
    rtcTime.setYear(rtc_tm.tm_year + 1900);
    rtcTime.setMonth(rtc_tm.tm_mon + 1);
    rtcTime.setDay(rtc_tm.tm_mday);
    rtcTime.setHour(rtc_tm.tm_hour);
    rtcTime.setMinute(rtc_tm.tm_min);
    rtcTime.setSecond(rtc_tm.tm_sec);
    close(rtcFd);
    return rtcTime;
#else
    return getDateTime();
#endif
}

bool DateTime::setRTCDateTime(const DateTime& dateTime) {
#if USE_RTC
    int rtcFd = ::open(RTC_DEV_PATH, O_RDWR);
    if (rtcFd < 0) {
        TRACE_ERR("DateTime::setRTCDateTime,open rtc error:%s.", ERRSTR);
        return false;
    }
    struct rtc_time rtc_tm;
    rtc_tm.tm_year = dateTime.year() - 1900;
    rtc_tm.tm_mon = dateTime.month() - 1;
    rtc_tm.tm_mday = dateTime.day();
    rtc_tm.tm_hour = dateTime.hour();
    rtc_tm.tm_min = dateTime.minute();
    rtc_tm.tm_sec = dateTime.second();
    int rc = ::ioctl(rtcFd, RTC_SET_TIME, &rtc_tm);
    if (rc < 0) {
        TRACE_ERR("DateTime::setRTCDateTime,write rtc error:%s.", ERRSTR);
        close(rtcFd);
        return false;
    }
    close(rtcFd);
    return false;
#else
    return setDateTime(dateTime);
#endif
}

bool DateTime::operator==(const DateTime& dt) const {
    return ((dt.m_year == this->m_year) && (dt.m_month == this->m_month) &&
            (dt.m_day == this->m_day) && (dt.m_hour == this->m_hour) &&
            (dt.m_minute == this->m_minute) &&
            (dt.m_second == this->m_second) &&
            (dt.m_msecond == this->m_msecond));
}
}  // namespace zemb
