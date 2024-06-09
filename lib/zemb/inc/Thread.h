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
#ifndef __ZEMB_THREAD_H__
#define __ZEMB_THREAD_H__

#include <limits.h>
#include <pthread.h>

#include <atomic>
#include <future>
#include <memory>
#include <utility>
#include <vector>

#include "BaseType.h"
#include "Singleton.h"
#include "ThreadUtil.h"
/**
 *  @file   Thread.h
 *  @brief  线程
 */
namespace zemb {
class Thread;
/**
 *  @class  Runnable
 *  @brief  线程运行体,子类必须重写run方法.
 */
class Runnable {
    DECL_CLASSNAME(Runnable)
public:
    Runnable() {}
    virtual ~Runnable() {}
    /**
     * @brief 线程体主函数
     * @param thread 当前线程体依附的线程
     * @note 所有子类必须实现
     */
    virtual void run(const Thread& thread) = 0;
};

class Thread {
public:
    Thread();
    Thread(const Thread& copy);
    virtual ~Thread();
    /**
     * @brief 克隆函数
     * @return std::unique_ptr<Thread>
     */
    virtual std::unique_ptr<Thread> clone() const;
    /**
     *  @brief  启动线程
     *  @param  runnable Runnable对象
     *  @return 成功返回true,失败返回false
     */
    virtual bool start(const Runnable& runnable);
    /**
     *  @brief  停止线程
     *  @param  msTimeout 超时时间,<0时表示阻塞等待线程退出
     *  @return 成功返回true,失败返回false
     */
    virtual bool stop(int msTimeout = -1);
    /**
     *  @brief  判断线程是否正在运行
     *  @param  void
     *  @return 正在运行返回true,否则返回false
     */
    virtual bool isRunning() const;
    /**
     *  @brief  微秒延时函数
     *  @param  us 要延时的微秒数
     *  @return void
     *  @note   函数休眠时,当前进程可能会让出CPU,引发进程调度
     *          注意使用usleep时,时间不能设置得太短,否则调度时
     *          进程间切换太频繁非常耗资源!!!推荐最小值为100us
     */
    static void usleep(int us);
    /**
     *  @brief  毫秒延时函数
     *  @param  ms 要延时的毫秒数
     *  @return void
     *  @note   函数休眠时,当前进程可能会让出CPU,引发进程调度
     */
    static void msleep(int ms);
    /**
     * @brief 获取当前代码所在线程ID
     * @return int
     */
    static int threadID();

protected:
    Runnable* m_runnable{nullptr};

private:
    std::future<void> m_future;
    std::atomic<bool> m_running{false};
};

/**
 *  @class PThread
 *  @brief 线程类
 */
class PThread : public Thread {
    DECL_CLASSNAME(PThread)

public:
    /**
     * @brief 线程属性说明
     * 调度策略
     * SCHED_OTHER:普通调度策略,优先级只能设置为0
     * SCHED_FIFO:不同优先级抢占,同等优先级先进先出,优先级可以设置为1(低)~99(高)
     * SCHED_RR:不同优先级抢占,同等优先级均分时间片,优先级可以设置为1(低)~99(高)
     * 继承方式
     * PTHREAD_INHERIT_SCHED(继承自父线程,忽略当前设置的属性),PTHREAD_EXPLICIT_SCHED(采用当前设置的线程属性)
     * 栈大小
     * 最小值为PTHREAD_STACK_MIN(16384)
     */
    enum SCHED_POLICY_E {
        SCHED_POLICY_OTHER = SCHED_OTHER,
        SCHED_POLICY_FIFO = SCHED_FIFO,
        SCHED_POLICY_RR = SCHED_RR,
    };
    struct ThreadAttr {
        int m_policy{SCHED_POLICY_OTHER};
        int m_priority{0};
        int m_inherit{PTHREAD_INHERIT_SCHED};
        int m_stackSize{PTHREAD_STACK_MIN};
    };
    enum THREAD_STATE {
        STATE_INIT = 0,
        STATE_START,
        STATE_RUNNING,
        STATE_EXIT
    };

public:
    PThread();
    PThread(int policy, int priority, bool inherit = false, int stackSize = 0);
    virtual ~PThread();
    std::unique_ptr<Thread> clone() const override;
    /**
     *  @brief  启动线程
     *  @param  runnable Runnable对象
     *  @return 成功返回true,失败返回false
     */
    bool start(const Runnable& runnable) override;
    /**
     *  @brief  停止线程
     *  @param  msTimeout 超时时间,<0时表示阻塞等待线程退出
     *  @return 成功返回true,失败返回false
     */
    bool stop(int msTimeout = -1) override;
    /**
     *  @brief  判断线程是否正在运行
     *  @param  void
     *  @return 正在运行返回true,否则返回false
     */
    bool isRunning() const override;
    /**
     *  @brief  微秒延时函数
     *  @param  us 要延时的微秒数
     *  @return void
     *  @note   函数休眠时,当前进程可能会让出CPU,引发进程调度
     *          注意使用usleep时,时间不能设置得太短,否则调度时
     *          进程间切换太频繁非常耗资源!!!推荐最小值为100us
     */
    static void usleep(int us);
    /**
     *  @brief  毫秒延时函数
     *  @param  ms 要延时的毫秒数
     *  @return void
     *  @note   函数休眠时,当前进程可能会让出CPU,引发进程调度
     */
    static void msleep(int ms);
    /**
     *  @brief  初始化实时抢占系统
     *  @param  policy
     *  @param  priority
     *  @return 成功返回true,失败返回false
     *  @note   此函数仅限在linux-rt上使用
     */
    static bool initWithPreemptRT(int policy, int priority);
    /**
     * @brief 设置线程取消点
     */
    static void setCancelPoint();

private:
    bool setAttribute(const pthread_attr_t& pAttr);
    void threadMain();
    static void* startRoutine(void* arg);

private:
    bool m_runFlag{false};
    int m_status{STATE_INIT};
    pthread_t m_threadID{0};
    pthread_attr_t m_attribute;
    ThreadAttr m_threadAttribute;
};

/**
 *  @class Threading
 *  @brief 线程化类,用以线程化类成员函数
 *  @note 用法
    class MultiThread:public Threading{
    public:
    MultiThread()
    {
        char* args = "hello";
        threading(&MultiThread::threadA,this);
        threading(&MultiThread::threadB,this,args);
    }
    private:
    void threadA()
    {
    }
    void threadB(void* args)
    {
    }
    };
 */
class Threading {
    DECL_CLASSNAME(Threading)

public:
    Threading();
    virtual ~Threading();
    /**
     * @brief 创建Thread线程
     * @tparam Function
     * @tparam Args
     * @param f
     * @param args
     * @return bool
     * @note 该接口有内存泄露的风险!!!
     */
    template <typename Function, typename... Args>
    bool threading(Function&& f, Args&&... args) {
        auto feature = std::async(std::launch::async, std::bind(f, args...));
        m_futures.push_back(std::move(feature));
        return true;
    }
    /**
     * @brief 创建PThread线程
     * @tparam Function
     * @tparam Args
     * @param f
     * @param args
     * @return true
     * @return false
     */
    template <typename Function, typename... Args>
    bool pthreading(Function&& f, Args&&... args) {
        return startPThreading(std::bind(f, args...));
    }

private:
    bool startPThreading(std::function<void(void*)> func);
    static void* startRoutine(void* arg);

private:
    std::vector<std::future<void>> m_futures;
};

/**
 *  @class  ThreadPool
 *  @brief  线程池
 */
class ThreadPool : public Singleton<ThreadPool> {
    DECL_CLASSNAME(ThreadPool)
    DECL_SINGLETON(ThreadPool)

public:
    ~ThreadPool();
    /**
     *  @brief  线程池初始化
     *  @param  protoType 线程原型,线程池中的线程均由此线程复制生成
     *  @param  maxThreadCount 最大线程个数
     *  @return 成功返回true,失败返回false
     */
    bool init(const Thread& protoType, int maxThreadCount);
    /**
     *  @brief  启动线程
     *  @param  runnable 线程运行体
     *  @param  priority 线程运行优先级
     *  @return 成功返回线程ID,失败返回STATUS_ERROR
     */
    int start(const Runnable& runnable);
    /**
     *  @brief  停止线程
     *  @param  threadID 线程id
     *  @return 成功返回true,失败返回false
     */
    bool cancel(int threadID);
    /**
     *  @brief  返回当前线程池最大线程个数
     *  @param  none
     *  @return 线程个数
     */
    int maxThreadCount();
    /**
     *  @brief  返回当前可用线程个数
     *  @param  none
     *  @return 线程个数
     */
    int idleThreadCount();

private:
    Mutex m_vectMutex;
    struct ThreadElement {
        std::unique_ptr<Thread> m_thread{nullptr};
        bool m_used{false};
    };
    std::vector<std::unique_ptr<ThreadElement>> m_threadVect;
    int m_maxThreadCount{0};
    int m_usedThreadCount{0};
};
}  // namespace zemb
#endif
