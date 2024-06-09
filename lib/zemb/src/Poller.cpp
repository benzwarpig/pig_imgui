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
#include "Poller.h"

#include <stdlib.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "DateTime.h"
#include "Tracer.h"
/******************************************************************************
 * EPOLL事件标志:
 * EPOLLIN : 输入事件,有数据待读取
 * EPOLLOUT: 输出事件,有数据要写入
 * EPOLLET : 边沿触发(事件就绪时,假设对事件没做处理,内核不会反复通知事件就绪)
 *           默认为水平触发(事件就绪时,假设对事件没做处理,内核会反复通知事件就绪)
 *****************************************************************************/
namespace zemb {
Poller::Poller() {}

Poller::~Poller() { close(); }

bool Poller::open(int maxEvents, bool onceNotify) {
    if (m_epfd > 0) {
        return true;
    }
    m_epfd = epoll_create1(0);
    if (m_epfd < 0) {
        return false;
    }
    m_events =
        (struct epoll_event*)calloc(maxEvents, sizeof(struct epoll_event));
    if (m_events == nullptr) {
        return false;
    }
    m_size = maxEvents;
    m_onceNotify = onceNotify;
    return true;
}
void Poller::close() {
    if (m_epfd > 0) {
        ::close(m_epfd);
        m_epfd = -1;
    }
    if (m_events != nullptr) {
        free(m_events);
        m_events = nullptr;
    }
}
bool Poller::addEvent(const PollEvent& pe) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_epfd < 0 || m_eventNum >= m_size || pe.dev() < 0) {
        return false;
    }
    struct epoll_event evt;
    evt.data.fd = pe.dev();
    switch (pe.event()) {
        case PollEvent::POLLIN:
            evt.events = EPOLLIN;
            break;
        case PollEvent::POLLOUT:
            evt.events = EPOLLOUT;
            break;
        default:
            return false;
    }
    if (m_onceNotify) {
        evt.events |= EPOLLET;
    }
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, evt.data.fd, &evt) < 0) {
        return false;
    }
    m_eventNum++;
    return true;
}

bool Poller::removeEvent(const PollEvent& pe) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_epfd < 0 || m_eventNum == 0) {
        return false;
    }
    struct epoll_event evt;
    evt.data.fd = pe.dev();
    switch (pe.event()) {
        case PollEvent::POLLIN:
            evt.events = EPOLLIN;
            break;
        case PollEvent::POLLOUT:
            evt.events = EPOLLOUT;
            break;
        default:
            return false;
    }
    if (m_onceNotify) {
        evt.events |= EPOLLET;
    }
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, evt.data.fd, &evt) < 0) {
        return false;
    }
    m_eventNum--;
    return true;
}

std::vector<std::shared_ptr<PollEvent>> Poller::waitEvent(int usTimeout) {
    int msTimeout = usTimeout / 1000;
    std::vector<std::shared_ptr<PollEvent>> peVect;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_epfd < 0) {
        return peVect;
    }

    int fds = epoll_wait(m_epfd, m_events, m_size, msTimeout);
    if (fds <= 0) {
        return peVect;
    }

    for (auto i = 0; i < fds; i++) {
        int fd = m_events[i].data.fd;
        std::shared_ptr<PollEvent> pe = nullptr;
        if (m_events[i].events & EPOLLIN) {
            pe = std::make_shared<PollEvent>(fd, PollEvent::POLLIN);
        } else if (m_events[i].events & EPOLLOUT) {
            pe = std::make_shared<PollEvent>(fd, PollEvent::POLLOUT);
        }
        if (pe != nullptr) {
            peVect.push_back(std::move(pe));
        }
    }
    return peVect;
}

EventPoller::EventPoller() {}
EventPoller::~EventPoller() {
    if (m_rwfd[1] > 0) {
        ::close(m_rwfd[1]);
    }
    if (m_rwfd[0] > 0) {
        ::close(m_rwfd[0]);
    }
    if (m_epfd > 0) {
        ::close(m_epfd);
    }
}

bool EventPoller::setup(int evMask) {
    struct epoll_event evt;
    if (m_epfd > 0) {
        return true;
    }
    m_epfd = epoll_create1(0);
    if (m_epfd < 0) {
        return false;
    }

    if (pipe(m_rwfd) != 0) {
        return false;
    }
    evt.data.fd = m_rwfd[0];
    evt.events = EPOLLIN;
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, evt.data.fd, &evt) < 0) {
        return false;
    }
    m_evMask = evMask;
    return true;
}

bool EventPoller::meetEvent(int event) {
    /* 在没有事件等待者时不要写入,因为管道写满后会发生阻塞 */
    if (m_isWaitting.load()) {
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(m_rwfd[1], &writefds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        int rc = ::select(m_rwfd[1] + 1, nullptr, &writefds, nullptr, &tv);
        if (rc <= 0) {
            return false;
        }
        if (FD_ISSET(m_rwfd[1], &writefds)) {
            int ret = write(m_rwfd[1], &event, sizeof(event));
            if (ret != sizeof(event)) {
                return false;
            }
            return true;
        }
    }
    return false;
}

int EventPoller::waitEvent(const int& event, uint32 msTimeout,
                           const std::vector<int>& evWait) {
    int eventVal = event;
    uint64 usTimeout = msTimeout * 1000;
    Time startTime = Time::fromMono();
    for (;;) {
        auto interval = Time::usSinceMono(startTime);
        if (interval >= usTimeout) {
            return RC_TIMEOUT;
        }
        m_isWaitting.store(true);
        int fds = epoll_wait(m_epfd, &m_event, 1, msTimeout);
        if (fds > 0) {
            if ((m_event.data.fd == m_rwfd[0]) && (m_event.events & EPOLLIN)) {
                int val = 0;
                int ret = read(m_rwfd[0], &val, sizeof(val));
                if (ret == sizeof(val)) {
                    eventVal = val;
                    eventVal &= m_evMask;
                    if (evWait.empty()) {
                        m_isWaitting.store(false);
                        return RC_OK;
                    }
                    for (auto& ev : evWait) {
                        if (ev == eventVal) {
                            m_isWaitting.store(false);
                            return RC_OK;
                        }
                    }
                }
            }
        }
    }
    m_isWaitting.store(false);
    return RC_ERROR;
}

void EventPoller::clearEvent() {
    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_rwfd[0], &readfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        int rc = ::select(m_rwfd[0] + 1, &readfds, nullptr, nullptr, &tv);
        if (rc <= 0) {
            break;
        } else if (FD_ISSET(m_rwfd[0], &readfds)) {
            char buf[256];
            read(m_rwfd[0], buf, sizeof(buf));
        }
    }
}
}  // namespace zemb
