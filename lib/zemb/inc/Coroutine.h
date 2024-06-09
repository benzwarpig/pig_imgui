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

#ifndef __ZEMB_COROUTINE_H__
#define __ZEMB_COROUTINE_H__

#include <ucontext.h>

#include <map>
#include <memory>

#include "BaseType.h"

/**
 * @file Coroutine.h
 * @brief 协程
 */

/* Linux进程最大堆栈空间为8M-8388608Bytes(可通过"ulimit -s"查看,值单位为K) */
const int kCoroutineStackSize = 1048576; /* 1M bytes:1024*1024=1048576 */

namespace zemb {
class CoScheduler;
/**
 * @enum CoroutineState
 * @brief 协程状态
 */
enum class CoroutineState { Stop = 0, Start, Suspend };

/**
 * @class Coroutine
 * @brief 协程类
 */
class Coroutine {
    DECL_CLASSNAME(Coroutine)

public:
    Coroutine();
    virtual ~Coroutine();
    /**
     * @brief 协程运行体
     * @note 子类需实现该方法
     */
    virtual void routine() = 0;
    /**
     * @brief 获取协程ID
     * @return int
     */
    int coroutineID();
    /**
     * @brief 获取协程状态
     * @return CoroutineState
     */
    CoroutineState state();
    /**
     * @brief 微妙级休眠
     * @param us
     */
    void usleep(int us);
    /**
     * @brief 毫秒级休眠
     * @param ms
     */
    void msleep(int ms);
    /**
     * @brief 判断协程是否正在运行
     * @return true
     * @return false
     */
    bool isRunning();

protected:
    void resume();
    void yield();

private:
    void saveStack();
    void restoreStack();

private:
    friend class CoScheduler;
    CoScheduler* m_scheduler;
    int m_id;
    ucontext_t m_context;
    CoroutineState m_state{CoroutineState::Stop};
    int m_stackSize{0};
    std::unique_ptr<char[]> m_stackBuffer;
};

/**
 * @class CoScheduler
 * @brief 协程调度器
 */
class CoScheduler {
    DECL_CLASSNAME(CoScheduler)

public:
    CoScheduler();
    virtual ~CoScheduler();
    /**
     * @brief 启动协程
     * @param coroutine
     * @return true
     * @return false
     */
    bool start(std::shared_ptr<Coroutine> coroutine);
    /**
     * @brief 调度协程
     * @param coroutineID
     */
    void schedule(int coroutineID);
    /**
     * @brief 停止协程
     * @param coroutineID
     */
    void stop(int coroutineID);
    /**
     * @brief 查询当前协程个数
     * @return int
     */
    int routines();
    /**
     * @brief 返回协程上下文
     * @return void*
     */
    void* context();
    /**
     * @brief 返回协程堆栈空间
     * @return char*
     */
    char* stack();

private:
    static void coroutineEntry(void* coroutine);

private:
    std::map<int, std::shared_ptr<Coroutine>> m_coroutineMap;
    ucontext_t m_context;
    char m_stack[kCoroutineStackSize];
};

}  // namespace zemb
#endif
