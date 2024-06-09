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
#ifndef __ZEMB_TIMER_H__
#define __ZEMB_TIMER_H__

#include <map>
#include <memory>

#include "BaseType.h"
#include "DateTime.h"
#include "Poller.h"
#include "Thread.h"
#include "ThreadUtil.h"

/**
 * @file  Timer.h
 * @brief 定时器
 */
namespace zemb {

/**
 * @class Timer
 * @brief 定时器类(较低精度,秒级定时器)
 */
class Timer {
    DECL_CLASSNAME(Timer)

public:
    /**
     * @brief 构造定时器
     * @param seconds seconds必须大于0
     * @param repeated
     * @note 在构造函数中会调用reset()进行复位
     */
    explicit Timer(int seconds, bool repeated = false);
    ~Timer();
    /**
     * @brief 检查是否到期
     * @param callback 定时回调函数
     * @note 定时器的起始时间为调用reset()的时间点
     */
    bool checkTimeout(std::function<void()> callback = nullptr);
    /**
     * @brief 重置定时器
     * @param seconds 当seconds<=0时,不复位超时时间和重复方式
     * @param repeated 重复方式
     */
    void reset(int seconds = 0, bool repeated = false);

private:
    Time m_startTime;
    bool m_isRepeated{false};
    int m_usInterval{0};
    bool m_expired{false};
};

/**
 * @class TimerManager
 * @brief 定时器管理器
 */
class TimerManager : public Runnable, public Singleton<TimerManager> {
    DECL_CLASSNAME(TimerManager)
    DECL_SINGLETON(TimerManager)

public:
    ~TimerManager();
    /**
     * @brief 注册定时器
     * @param timer 定时器
     * @param callback 定时回调
     * @return true
     * @return false
     */
    bool registerTimer(const Timer& timer, std::function<void()> callback);
    /**
     * @brief 注销定时器
     * @param timer
     * @return true
     * @return false
     */
    bool unregisterTimer(const Timer& timer);
    /**
     * @brief 启动定时
     * @return true
     * @return false
     */
    bool start();
    /**
     * @brief 停止定时
     */
    void stop();

private:
    void run(const Thread& thread);

private:
    Mutex m_mutex;
    Thread m_thread;
    std::map<Timer*, std::function<void()>> m_timerMap;
};

/**
 * @class TimerListener
 * @brief Timer定时器监听器接口
 */
class TimerListener {
public:
    TimerListener() {}
    virtual ~TimerListener() {}
    /**
     * @brief 定时回调函数
     * @param timerID
     * @note
     * 回调函数中不允许调用任何影响Timer生命周期的方法,比如start(),stop()等
     */
    virtual void onTimer(int timerID) = 0;
};

/**
 *  @class  RTimer
 *  @brief  实时定时器类(较高精度,适用于毫秒级定时)
 */
class RTimer : public Runnable {
    DECL_CLASSNAME(RTimer)

public:
    /**
     * @brief RTimer构造函数
     * @param listener 定时器监听器
     * @param timerID 定时器ID
     */
    RTimer(const TimerListener& listener, int timerID);
    ~RTimer();
    /**
     * @brief 启动定时器
     * @param msTimeout 定时时间(unit: ms)
     * @param repeated 是否重复
     * @return true 启动成功
     * @return false 启动失败
     */
    bool start(int msTimeout, bool repeated = false);
    /**
     * @brief 停止定时器
     */
    void stop();
    /**
     * @brief 获取定时器ID
     * @return int 定时器ID
     */
    int id() { return m_timerID; }

private:
    void run(const Thread& thread);

private:
    std::unique_ptr<Thread> m_thread{nullptr};
    TimerListener* m_listener{nullptr};
    bool m_isRepeated{false};
    int m_timerID{0};
    int m_usInterval{0};
    int m_usTick{1};
};

/**
 *  @class  HRTimer
 *  @brief  高精度定时器(基于事件实现的定时器类,适用于实时系统中微秒级定时)
 */
class HRTimer : public Runnable {
    DECL_CLASSNAME(HRTimer)

public:
    /**
     * @brief PollTimer构造函数
     * @param listener 定时器监听器
     * @param timerID 定时器ID
     */
    HRTimer(const TimerListener& listener, int timerID);
    ~HRTimer();
    /**
     * @brief 启动定时器
     * @param usTimeout 定时时间
     * @param repeated 是否重复
     * @return true 启动成功
     * @return false 启动失败
     */
    bool start(int usTimeout, bool repeated = false);
    /**
     * @brief 停止定时器
     */
    void stop();
    /**
     * @brief 获取定时器ID
     * @return int 定时器ID
     */
    int id() { return m_timerID; }

private:
    void run(const Thread& thread);

private:
    Thread m_thread;
    TimerListener* m_listener{nullptr};
    Poller m_poller;
    int m_tmfd{-1};
    int m_usInterval{0};
    int m_timerID{0};
};
}  // namespace zemb
#endif
