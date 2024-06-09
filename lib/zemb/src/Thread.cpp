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
#include "Thread.h"

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>

#include "Tracer.h"

namespace zemb {

// Tracer类中使用了Thread,因此Thread类中不能使用TRACE_XXX宏,否则会死锁,只能使用PRINT_XXX宏!!!
#define USE_DEBUG 0
#if (!USE_DEBUG)
#undef PRINT_DBG
#define PRINT_DBG(fmt, args...)
#endif

Thread::Thread() {}

Thread::Thread(const Thread& copy) {}

Thread::~Thread() { stop(); }

std::unique_ptr<Thread> Thread::clone() const {
    return std::make_unique<Thread>(*this);
}

bool Thread::start(const Runnable& runnable) {
    m_runnable = const_cast<Runnable*>(&runnable);
    try {
        m_future = std::async(std::launch::async, [&]() {
            m_running.store(true);
            m_runnable->run(*this);
            m_running.store(false);
        });
    } catch (const std::system_error& e) { /* 当系统资源不够时,抛出系统异常 */
        PRINT_ERR("Thread start error: %s", e.what());
        return false;
    }
    return true;
}

bool Thread::stop(int msTimeout) {
    m_running.store(false);
    if (m_future.valid()) {
        m_future.wait();
    }
    return true;
}

bool Thread::isRunning() const { return m_running.load(); }

void Thread::usleep(int us) {
    us = (us <= 0) ? 1 : us;
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void Thread::msleep(int ms) {
    ms = (ms <= 0) ? 1 : ms;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int Thread::threadID() {
    std::thread::id tid = std::this_thread::get_id();
    return *(reinterpret_cast<int*>(&tid));
}

PThread::PThread() {}

PThread::PThread(int policy, int priority, bool inherit, int stackSize) {
    m_threadAttribute.m_policy = policy;
    if (policy == SCHED_POLICY_OTHER) {
        priority = 0;
    }
    m_threadAttribute.m_priority = priority;
    m_threadAttribute.m_stackSize = stackSize;
    m_threadAttribute.m_inherit =
        inherit ? PTHREAD_INHERIT_SCHED : PTHREAD_EXPLICIT_SCHED;
    if (stackSize < PTHREAD_STACK_MIN) {
        m_threadAttribute.m_stackSize = PTHREAD_STACK_MIN;
    } else {
        m_threadAttribute.m_stackSize = stackSize;
    }
}

PThread::~PThread() {
    if (m_status == STATE_EXIT) {
        pthread_join(m_threadID, nullptr);
        m_threadID = 0;
    }
}

std::unique_ptr<Thread> PThread::clone() const {
    return std::make_unique<PThread>(*this);
}

bool PThread::start(const Runnable& runnable) {
    pthread_attr_t* pAttr = nullptr;
    if (m_status != STATE_INIT) {
        PRINT_ERR("Thread state error, state: %d", m_status);
        return false;
    }

    m_runnable = const_cast<Runnable*>(&runnable);
    /* 设置线程属性 */
    pthread_attr_init(&m_attribute);
    if (setAttribute(m_attribute)) {
        pAttr = &m_attribute;
    }
    m_status = STATE_START;
    if (0 != pthread_create(&m_threadID, pAttr, startRoutine, this)) {
        PRINT_ERR("pthread create error: %s!", ERRSTR);
        return false;
    }
    pthread_attr_destroy(&m_attribute);
    return true;
}

bool PThread::stop(int msTimeout) {
    if (!m_runFlag) {
        return true;
    }
    m_runFlag = false;
    if (msTimeout < 0) {
        msTimeout = MAX_SINT32;
    }
    msTimeout = MAX(10, msTimeout);
    while (msTimeout > 0) {
        if (m_status == STATE_EXIT) {
            return true;
        }
        msleep(10);
        msTimeout -= 10;
    }
    return false;
}

bool PThread::isRunning() const {
    if (m_runnable == nullptr || !m_runFlag) {
        return false;
    }
    return (m_status == STATE_RUNNING) ? true : false;
}

void PThread::usleep(int us) {
    struct timespec ts;
    if (us <= 0) {
        us = 1;
    }
    ts.tv_sec = us / 1000000;
    if (ts.tv_sec == 0) {
        ts.tv_nsec = us * 1000;
    } else {
        ts.tv_nsec = (us % 1000000) * 1000;
    }
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts,
                    nullptr); /* 使用MONITONIC时钟,不受更改系统时钟的影响 */
}

void PThread::msleep(int ms) { PThread::usleep(ms * 1000); }

bool PThread::initWithPreemptRT(int policy, int priority) {
    /* 锁定当前进程内存,防止被系统SWAP到硬盘中去,影响实时性能 */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        return false;
    }

#if 0
    {
        /* 栈空间预留:Linux进程栈空间最大为8M
         * 故意让栈空间往下生长8K,其后的函数调用和局部变量将不再导致
         * 栈增长(依赖于page fault和内存申请)
         */
        unsigned char dummy[8096];
        memset(dummy, 0, 8096);
    }
#endif
    /* 设置当前进程调度策略和优先级 */
    pid_t pid = getpid();
    struct sched_param param;
    param.sched_priority = priority;
    if (sched_setscheduler(pid, policy, &param) < 0) {
        PRINT_ERR("main process(%d) cannot set policy:%d, priority:%d.", pid,
                  policy, priority);
        return false;
    }
    return true;
}

bool PThread::setAttribute(const pthread_attr_t& threadAttr) {
    struct sched_param param;
    pthread_attr_t attribute = threadAttr;
    if (pthread_attr_setstacksize(&attribute, m_threadAttribute.m_stackSize) !=
        0) {
        PRINT_ERR("Set thread attribute stack size:%d failed!",
                  m_threadAttribute.m_stackSize);
        return false;
    }

    /* 设置调度策略 */
    if (pthread_attr_setschedpolicy(&attribute, m_threadAttribute.m_policy) !=
        0) {
        PRINT_ERR("Set thread attribute policy:%d failed",
                  m_threadAttribute.m_policy);
        return false;
    }

    /* 设置调度优先级 */
    param.sched_priority = m_threadAttribute.m_priority;
    if (pthread_attr_setschedparam(&attribute, &param) != 0) {
        PRINT_ERR("Set thread attribute priority:%d failed",
                  m_threadAttribute.m_priority);
        return false;
    }

    /* 设置线程调度策略继承属性 */
    if (pthread_attr_setinheritsched(&attribute, m_threadAttribute.m_inherit) !=
        0) {
        PRINT_ERR("Set thread sched inherit:%d failed",
                  m_threadAttribute.m_inherit);
        return false;
    }
    return true;
}

#if 0
bool PThread::cancel() {
#ifdef OS_ANDROID
    return false;
#else
    if (m_threadID > 0) {
        /* 执行pthread_cancel后,并不会直接取消线程,必须等到下一次系统调用或者pthread_testcancel才会真正取消线程
         */
        if (0 != pthread_cancel(m_threadID)) {
            PRINT_ERR("pthread cancel error:%s.", ERRSTR);
            return false;
        }
        pthread_join(m_threadID, nullptr);
    }
    m_status = STATE_EXIT;
    m_threadID = 0;
    return true;
#endif
}

void PThread::setCancelPoint() {
#ifdef OS_ANDROID
    PRINT_ERR("Not support thread cancel!");
#else
    pthread_testcancel();
#endif
}
#endif

/* 线程运行函数 */
void PThread::threadMain() {
    m_status = STATE_RUNNING;
    m_threadID = pthread_self();
    if (m_runnable != nullptr) {
        m_runFlag = true;
        m_runnable->run(*this);
        m_runFlag = false;
    }
    m_status = STATE_EXIT;
    m_threadID = 0;
    pthread_exit(nullptr);
}

/* 线程入口点 */
void* PThread::startRoutine(void* arg) {
    // 设置detach,在线程结束后自动回收资源,不再需要pthread_join
    pthread_detach(pthread_self());
    PThread* pThread = reinterpret_cast<PThread*>(arg);
    if (pThread != nullptr) {
        pThread->threadMain();
    }
    return nullptr;
}

/**
 * @struct ThreadArgs
 * @brief 线程参数
 */
struct ThreadArgs {
    Threading* m_owner;
    std::function<void(void*)> m_entry;
    void* m_args;
};

Threading::Threading() {}

Threading::~Threading() {
    for (auto& future : m_futures) {
        if (future.valid()) {
            future.wait();
        }
    }
}

bool Threading::startPThreading(std::function<void(void*)> func) {
    pthread_t threadID;
    auto threadArgs = std::make_unique<ThreadArgs>();
    threadArgs->m_owner = this;
    threadArgs->m_entry = func;
    threadArgs->m_args = nullptr;
    if (threadArgs->m_entry == nullptr) {
        PRINT_ERR("thread function cannot be nullptr!");
        return false;
    }
    if (0 != pthread_create(&threadID, nullptr, startRoutine,
                            threadArgs.release())) {
        PRINT_ERR("threading failed:%s!", ERRSTR);
        return false;
    }
    return true;
}

/* 任务入口点 */
void* Threading::startRoutine(void* arg) {
    /* detach后不再需要pthread_join(),会自动回收线程资源 */
    pthread_detach(pthread_self());
    std::unique_ptr<ThreadArgs> threadArgs(reinterpret_cast<ThreadArgs*>(arg));
    if (threadArgs->m_owner != nullptr && threadArgs->m_entry != nullptr) {
        threadArgs->m_entry(threadArgs->m_args);
    }
    return nullptr;
}

ThreadPool::ThreadPool() {}
ThreadPool::~ThreadPool() {}

bool ThreadPool::init(const Thread& protoType, int maxThreadCount) {
    if (maxThreadCount <= 0) {
        PRINT_ERR("Thread number must be greater than 0!");
        return false;
    }

    if (m_maxThreadCount > 0) { /* 已经初始化过了 */
        PRINT_ERR("Thread Pool is allready inited!");
        return false;
    }

    m_maxThreadCount = maxThreadCount;
    for (auto i = 0; i < m_maxThreadCount; i++) {
        auto thread = protoType.clone();
        auto element = std::make_unique<ThreadElement>();
        element->m_thread = std::move(thread);
        element->m_used = false;
        m_threadVect.push_back(std::move(element));
    }
    return true;
}

int ThreadPool::start(const Runnable& runnable) {
    AutoLock lock(m_vectMutex);
    if (m_usedThreadCount >= m_maxThreadCount) {
        return RC_ERROR;
    }
    for (auto i = 0; i < m_maxThreadCount; i++) {
        if (m_threadVect[i]->m_used == false) {
            // if (!m_threadVect[i]->m_thread->isRunning())
            {
                if (!m_threadVect[i]->m_thread->start(runnable)) {
                    return RC_ERROR;
                }
                m_usedThreadCount++;
                m_threadVect[i]->m_used = true;
                return i;
            }
        }
    }
    return RC_ERROR;
}

bool ThreadPool::cancel(int threadID) {
    if (threadID < 0 || threadID >= m_maxThreadCount) {
        return false;
    }
    AutoLock lock(m_vectMutex);
    if (m_threadVect[threadID]->m_used) {
        // if (m_threadVect[threadID]->m_thread->isRunning())
        {
            if (!m_threadVect[threadID]->m_thread->stop()) {
                return false;
            }
        }
        m_usedThreadCount--;
        m_threadVect[threadID]->m_used = false;
    }
    return true;
}

int ThreadPool::maxThreadCount() {
    AutoLock lock(m_vectMutex);
    return m_maxThreadCount;
}

int ThreadPool::idleThreadCount() {
    AutoLock lock(m_vectMutex);
    return m_maxThreadCount - m_usedThreadCount;
}

}  // namespace zemb
