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
 * Copyright 2014-2020 @ ShenZhen ,China
 *******************************************************************************/
#ifndef __ZEMB_POLLER_H__
#define __ZEMB_POLLER_H__

#include <sys/epoll.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "BaseType.h"

/**
 * @file Poller.h
 * @brief IO复用/轮询
 */
namespace zemb {

class PollEvent {
public:
    enum EVENT_E {
        POLLIN = 0,  // 读
        POLLOUT,     // 写
    };

public:
    PollEvent() {}
    PollEvent(int dev, int event) : m_dev(dev), m_event(event) {}
    int dev() const { return m_dev; }
    int event() const { return m_event; }

private:
    int m_dev{-1};
    int m_event{-1};
};

class PollDev {
public:
    PollDev() {}
    virtual ~PollDev() {}
    virtual bool reopen() { return false; }
    virtual PollEvent pollEvent() = 0;
    virtual bool onPoll() = 0;
};

/**
 * @class Poller
 * @brief IO复用/轮询集
 */
class Poller {
    DECL_CLASSNAME(Poller)

public:
    Poller();
    virtual ~Poller();
    /**
     * @brief 打开复用集
     * @param onceNotify 只通知一次
     * @return true
     * @return false
     */
    bool open(int maxEvents, bool onceNotify = false);
    /**
     * @brief 关闭复用集
     * @param void
     */
    void close();
    /**
     * @brief 增加轮询事件
     * @param event
     * @return true
     * @return false
     */
    bool addEvent(const PollEvent& event);
    /**
     * @brief 移除轮询事件
     * @param event
     * @return true
     * @return false
     */
    bool removeEvent(const PollEvent& event);
    /**
     * @brief 等待事件
     * @param usTimeout
     * @return 事件集
     */
    std::vector<std::shared_ptr<PollEvent>> waitEvent(int usTimeout);

private:
    int m_epfd{-1};
    int m_size{0};
    bool m_onceNotify{false};
    int m_eventNum{0};
    struct epoll_event* m_events{nullptr};
    std::mutex m_mutex;
};

class EventPoller {
    DECL_CLASSNAME(EventPoller)
public:
    EventPoller();
    virtual ~EventPoller();
    bool setup(int evMask = 0xFFFFFFFF);
    bool meetEvent(int event);
    int waitEvent(const int& event, uint32 msTimeout,
                  const std::vector<int>& evWait = std::vector<int>());
    void clearEvent();

private:
    int m_epfd{-1};
    int m_rwfd[2]{-1, -1};
    int m_evMask{-1};
    std::atomic<bool> m_isWaitting{false};
    struct epoll_event m_event {
        0
    };
};
}  // namespace zemb
#endif
