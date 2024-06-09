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

#include "Coroutine.h"

#include "DateTime.h"
#include "Tracer.h"

namespace zemb {
static int s_coroutineStartID = 0;
Coroutine::Coroutine() {
    m_id = s_coroutineStartID;
    s_coroutineStartID++;
}

Coroutine::~Coroutine() {}

int Coroutine::coroutineID() { return m_id; }

CoroutineState Coroutine::state() { return m_state; }

void Coroutine::usleep(int us) {
    Time startTime = Time::fromMono();
    if (us <= 0) {
        us = 1;
    }
    for (;;) {
        yield();
        Time duration = Time::fromMono() - startTime;
        if (duration.toMicroSec() >= us) {
            break;
        }
    }
}

void Coroutine::msleep(int ms) { usleep(1000 * ms); }

bool Coroutine::isRunning() {
    return (m_state == CoroutineState::Stop) ? false : true;
}

void Coroutine::resume() {
    if (m_state == CoroutineState::Suspend) {
        restoreStack();
        m_state = CoroutineState::Start;
        /* 切换到当前协程 */
        if (swapcontext(reinterpret_cast<ucontext_t*>(m_scheduler->context()),
                        &m_context) < 0) {
            TRACE_ERR_CLASS("swap context error:%s", ERRSTR);
        }
    }
}

void Coroutine::yield() {
    if (m_state == CoroutineState::Start) {
        saveStack();
        m_state = CoroutineState::Suspend;
        /* 切换回调度线程 */
        if (swapcontext(&m_context, reinterpret_cast<ucontext_t*>(
                                        m_scheduler->context())) < 0) {
            TRACE_ERR_CLASS("swap context error:%s", ERRSTR);
        }
    }
}

void Coroutine::saveStack() {
    /* 计算当前堆栈位置,堆栈是往下生长的,栈顶在低地址,栈底是高地址 */
    char* stackBottom = m_scheduler->stack() + kCoroutineStackSize;
    char dumy = 0;
    int stackSize = stackBottom - &dumy; /* stackSize:当前所占用的栈空间 */
    // TRACE_REL_CLASS("Coroutine:%d, stack size:%d",m_id,stackSize);
    if (stackSize > kCoroutineStackSize) {
        TRACE_ERR_CLASS("stack overflow at coroutine:%d", m_id);
        return;
    }
    /* 第一次就分配足够的空间 */
    if (!m_stackBuffer) {
        m_stackBuffer = std::make_unique<char[]>(kCoroutineStackSize);
    }
    m_stackSize = stackSize;
    memcpy(m_stackBuffer.get(), &dumy, m_stackSize);
}

void Coroutine::restoreStack() {
    char* stackTop = m_scheduler->stack() + kCoroutineStackSize - m_stackSize;
    memcpy(stackTop, m_stackBuffer.get(), m_stackSize);
}

CoScheduler::CoScheduler() {}

CoScheduler::~CoScheduler() {}

bool CoScheduler::start(std::shared_ptr<Coroutine> coroutine) {
    int id = coroutine->m_id;
    auto state = coroutine->m_state;
    if (id < 0 || state != CoroutineState::Stop) {
        return false;
    }

    auto iter = m_coroutineMap.find(id);
    if (iter != m_coroutineMap.end()) {
        m_coroutineMap.erase(iter);
    }
    m_coroutineMap.insert(std::make_pair(id, coroutine));

    /* 创建上下文 */
    if (getcontext(&(coroutine->m_context)) < 0) {
        return false;
    }
    coroutine->m_scheduler = this;
    coroutine->m_context.uc_stack.ss_sp = m_stack;
    coroutine->m_context.uc_stack.ss_size = kCoroutineStackSize;
    coroutine->m_context.uc_stack.ss_flags = 0;
    coroutine->m_context.uc_link = &m_context;

#if 0
    uint32 p1 =
        static_cast<uint32>((0x00000000FFFFFFFF) &
                            (reinterpret_cast<uint64>(coroutine.get())) >> 32);
    uint32 p2 = static_cast<uint32>(0x00000000FFFFFFFF &
                                    reinterpret_cast<uint64>(coroutine.get()));
    makecontext(&m_context, (void (*)())coroutineEntry, 2, p1, p2);
#else
    makecontext(&(coroutine->m_context), (void (*)())coroutineEntry, 1,
                reinterpret_cast<void*>(coroutine.get()));
#endif

    /* 调用swapcontext成功后会跳转到coroutine,不会立即返回,当coroutine挂起后才返回
     */
    if (swapcontext(&m_context, &(coroutine->m_context)) < 0) {
        TRACE_ERR_CLASS("swap context error:%s", ERRSTR);
        return false;
    }
    return true;
}

void CoScheduler::schedule(int coroutineID) {
    auto iter = m_coroutineMap.find(coroutineID);
    if (iter != m_coroutineMap.end()) {
        iter->second->resume();
    }
}
void CoScheduler::stop(int coroutineID) {
    auto iter = m_coroutineMap.find(coroutineID);
    if (iter != m_coroutineMap.end()) {
        iter->second->m_state =
            CoroutineState::Stop; /* 设置为STATE_STOP,这样协程能正常退出 */
        m_coroutineMap.erase(iter);
    }
}
int CoScheduler::routines() {
    int count = 0;
    for (auto kv : m_coroutineMap) {
        if (kv.second->state() != CoroutineState::Stop) {
            count++;
        }
    }
    return count;
}

void* CoScheduler::context() { return (reinterpret_cast<void*>(&m_context)); }

char* CoScheduler::stack() { return m_stack; }

void CoScheduler::coroutineEntry(void* coroutine) {
    Coroutine* crtn = reinterpret_cast<Coroutine*>(coroutine);
    crtn->m_state = CoroutineState::Start;
    crtn->routine();
    while (crtn->m_state != CoroutineState::Stop) {
        crtn->usleep(100);
    }
    crtn->m_state = CoroutineState::Stop;
}

}  // namespace zemb
