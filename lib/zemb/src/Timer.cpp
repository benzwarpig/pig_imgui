#include "Timer.h"

#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "DateTime.h"
#include "Tracer.h"

namespace zemb {
Timer::Timer(int seconds, bool repeated) {
    if (seconds > 0) {
        reset(seconds, repeated);
    } else {
        m_expired = true;
    }
}

Timer::~Timer() {}

bool Timer::checkTimeout(std::function<void()> callback) {
    if (m_expired) {
        return true;
    }
    int diff = Time::usSinceMono(m_startTime);
    if (diff < m_usInterval) {
        return false;
    }
    m_expired = true;
    if (callback) {
        callback();
    }
    if (m_isRepeated) {
        int usOverTime = diff - m_usInterval;
        if (usOverTime < m_usInterval) {
            m_startTime = Time::fromMono() - Time(0, usOverTime);
        } else {
            m_startTime = Time::fromMono();
        }
    }
}

void Timer::reset(int seconds, bool repeated) {
    if (seconds > 0) {
        m_isRepeated = repeated;
        m_usInterval = seconds * 1000000;
    }
    if (m_isRepeated) {
        m_expired = false;
    }
    m_startTime = Time::fromMono();
}

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {}

bool TimerManager::registerTimer(const Timer& timer,
                                 std::function<void()> callback) {
    Timer* pTimer = const_cast<Timer*>(&timer);
    AutoLock lock(m_mutex);
    if (m_timerMap.find(pTimer) != m_timerMap.end()) {
        return false;
    }
    m_timerMap.insert({pTimer, callback});
    return true;
}

bool TimerManager::unregisterTimer(const Timer& timer) {
    Timer* pTimer = const_cast<Timer*>(&timer);
    AutoLock lock(m_mutex);
    auto iter = m_timerMap.find(pTimer);
    if (iter == m_timerMap.end()) {
        return false;
    }
    m_timerMap.erase(iter);
    return false;
}

bool TimerManager::start() { return m_thread.start(*this); }

void TimerManager::stop() { m_thread.stop(); }

void TimerManager::run(const Thread& thread) {
    {
        AutoLock lock(m_mutex);
        for (auto& iter : m_timerMap) {
            iter.first->reset();
        }
    }
    while (thread.isRunning()) {
        Thread::msleep(10);
        AutoLock lock(m_mutex);
        for (auto& iter : m_timerMap) {
            if (iter.first->checkTimeout(iter.second)) {
                iter.first->reset();
            }
        }
    }
}

RTimer::RTimer(const TimerListener& listener, int id) : m_timerID(id) {
    m_listener = const_cast<TimerListener*>(&listener);
}

RTimer::~RTimer() {
    if (m_thread) {
        stop();
        m_thread = nullptr;
    }
}

bool RTimer::start(int msTimeout, bool repeat) {
    if (msTimeout <= 0 || m_timerID < 0 || m_listener == nullptr) {
        TRACE_ERR_CLASS("param error, id:%d, timeout:%d, listener:0x%x",
                        m_timerID, msTimeout, m_listener);
        return false;
    }
    m_isRepeated = repeat;
    m_usInterval = msTimeout * 1000;
    /* 定时器tick,取值为[1000,10000],值越小线程负载越大,值越大精度越低 */
    m_usTick = CLIP(1000, m_usInterval / 100, 10000);
    m_thread = std::make_unique<Thread>();
    return m_thread->start(*this);
}

void RTimer::stop() {
    if (m_thread) {
        m_thread->stop();
    }
}

void RTimer::run(const Thread& thread) {
    bool firstTime = true;
    Time lastTime, currTime;
    while (thread.isRunning()) {
        if (firstTime) {
            lastTime = Time::fromMono();
            firstTime = false;
        } else {
            currTime = Time::fromMono();
            Time timeDiff = currTime - lastTime;
            int usOver = static_cast<int>(timeDiff.toMicroSec()) - m_usInterval;
            if (usOver >= 0) { /* 到了定时时间 */
                // TRACE_DBG("RTimer usOver=%d",usOver);
                if (usOver > m_usInterval) {
                    // TRACE_WARN("RTimer over run interval: %d >
                    // %d",usOver,m_usInterval);
                    lastTime = currTime; /* 已经超过一个周期了,重新计算差值 */
                } else {
                    Time overTime(0, usOver);
                    lastTime = currTime - overTime; /* 减去超过的时间 */
                }

                if (m_listener != nullptr) {
                    m_listener->onTimer(m_timerID);
                    if (!m_isRepeated) {
                        break;
                    }
                }
            }
        }
        /* 每隔m_usTick读一次时间 */
        PThread::usleep(m_usTick);
    }
}

HRTimer::HRTimer(const TimerListener& listener, int id) : m_timerID(id) {
    m_listener = const_cast<TimerListener*>(&listener);
}

HRTimer::~HRTimer() {
    if (m_tmfd > 0) {
        close(m_tmfd);
    }
    stop();
}

bool HRTimer::start(int usTimeout, bool repeat) {
    if (usTimeout <= 0) {
        return false;
    }

    if (m_tmfd < 0) {
        m_tmfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if (m_tmfd < 0) {
            return false;
        }
        if (!m_poller.open(1, false)) {
            TRACE_ERR_CLASS("poller open error!");
            return false;
        }
        PollEvent event(m_tmfd, PollEvent::POLLIN);
        if (!m_poller.addEvent(event)) {
            TRACE_ERR_CLASS("poller add event error!");
            return false;
        }
    }

    itimerspec ts;
    Time value(usTimeout);  // 第一次超时时间
    ts.it_value.tv_sec = value.secPart();
    ts.it_value.tv_nsec = value.usPart() * 1000;

    if (repeat) {
        Time interval(usTimeout);  // 重复时间间隔
        ts.it_interval.tv_sec = interval.secPart();
        ts.it_interval.tv_nsec = interval.usPart() * 1000;
    } else {
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
    }
    if (timerfd_settime(m_tmfd, 0, &ts, nullptr) < 0) {
        return false;
    }

    m_usInterval = usTimeout;
    return m_thread.start(*this);
}

void HRTimer::stop() {
    if (m_tmfd > 0) {
        itimerspec ts;
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 0;
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
        timerfd_settime(m_tmfd, 0, &ts, nullptr);
    }
    m_thread.stop();
}

void HRTimer::run(const Thread& thread) {
    int checkTime = m_usInterval / 10;
    while (thread.isRunning()) {
        auto events = m_poller.waitEvent(checkTime);
        if (events.size() > 0) {
            uint64 timeoutCount = 0;
            int ret = read(m_tmfd, &timeoutCount, sizeof(timeoutCount));
            if (ret == sizeof(timeoutCount) && m_listener != nullptr) {
                if (timeoutCount > 1) {
                    // TRACE_WARN_CLASS("HRTimer::run, timer over run: %llu",
                    // timeoutCount);
                }
                m_listener->onTimer(m_timerID);
            }
        }
    }
}
}  // namespace zemb
