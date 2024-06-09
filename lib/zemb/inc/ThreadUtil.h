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
#ifndef __ZEMB_THREAD_UTIL_H__
#define __ZEMB_THREAD_UTIL_H__

#include <fcntl.h> /* For O_* constants */
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h> /* For mode constants */

#include <queue>
#include <string>

#include "BaseType.h"

/**
 *  @file  ThreadUtil.h
 *  @brief 线程工具
 */
namespace zemb {
/**
 * @class Mutex
 * @brief 互斥锁类
 * @note
 * 互斥锁在lock后可以睡眠,但需注意unLock后再次lock之间必须给其他线程留有一定的抢占时间,否则当前线程会一直占着锁.
 */
class Mutex {
public:
    Mutex();
    ~Mutex();
    /**
     * @brief  互斥锁上锁
     * @return int 成功返回0,错误返回-1
     */
    int lock();
    /**
     * @brief  互斥锁解锁
     * @return int 成功返回0,错误返回-1
     */
    int unLock();
    /**
     * @brief  互斥锁尝试上锁
     * @return int 成功返回0,错误返回-1
     */
    int tryLock();

private:
    pthread_mutex_t m_mutex;
    friend class MutexCond;
};

/**
 * @class RWMutex
 * @brief 读写锁
 */
class RWMutex {
public:
    void lockR();
    void unLockR();
    void lockW();
    void unLockW();

private:
    Mutex m_rMutex;
    Mutex m_wMutex;
    int m_readCnts{0};  // 已加读锁个数
};

/**
 *  @class AutoLock
 *  @brief 自动释放的互斥锁类
 */
class AutoLock {
public:
    explicit AutoLock(const Mutex& mutex);
    virtual ~AutoLock();

private:
    Mutex* m_mutex;
};

/**
 *  @class MutexCond
 *  @brief 条件变量
 */
class MutexCond : public Mutex {
public:
    MutexCond();
    ~MutexCond();
    /**
     *  @brief  等待条件变量
     *  @param  usec,超时时间,<0时阻塞等待,>=0时等待usec
     *  @return
     * 成功等到条件变量返回STATUS_OK,超时返回STATUS_TIMEOUT,失败返回STATUS_ERROR
     *  @code   多个线程不可以同时调用wait()方法,必需使用lock进行互斥,例程:
     *          线程A                              线程B
     *          ...                              ...
     *          cond.lock();                     cond.lock();
     *          while(pass==0)                   pass=1;
     *          {                                cond.meet();
     *             cond.wait(100);               con.unlock();
     *          }                                ...
     *          cond.unlock();
     *          ...
     * @endcode
     */
    int wait(int usec = -1);
    /**
     * @brief 通知等待者满足条件变量
     * @return int
     */
    int meet();

private:
    pthread_cond_t m_cond;
    pthread_condattr_t m_condAttr;
};

/**
 *  @class Semaphore
 *  @brief 信号量类(基于POSIX接口)
 *  @note 无名信号量用于线程间通信,有名信号量用于进程间通信
 *        !!!有名信号量目前测试有问题,请使用SemaphoreV替代!!!
 */
class Semaphore {
    DECL_CLASSNAME(Semaphore)

public:
    Semaphore();
    ~Semaphore();
    /**
     *  @brief  打开信号量
     *  @param name 名称中不能包含'/',name为空时表示创建无名信号量
     *  @param  value 打开有名信号量时,如果信号量已经存在,该值会被忽略
     *  @return 成功返回true,失败返回false
     *  @note   信号量存在,则返回,不存在则创建
     */
    virtual bool open(const std::string& name, int value);
    /**
     *  @brief  关闭信号量
     *  @param  void
     *  @return 成功返回true,失败返回false
     *  @note   对于有名信号量,关闭信号量不会真正删除信号量,还需要调用unlink()
     */
    virtual bool close();
    /**
     *  @brief  删除信号量
     *  @param  void
     *  @return 成功返回true,失败返回false
     *  @note   该方法仅对有名信号量有效
     */
    virtual bool unlink();
    /**
     *  @brief  等待信号量
     *  @param  msTimeout 超时时间
     *  @return 成功返回true,失败返回false
     *  @note   使信号量值减1,如果无资源可申请,则阻塞.
     */
    virtual bool wait(int msTimeout = -1);
    /**
     *  @brief  尝试等待信号量
     *  @param  void
     *  @return 成功返回true,失败返回false
     *  @note   使信号量值减1,如果无资源可申请,则返回.
     */
    virtual bool tryWait();
    /**
     *  @brief  释放信号量
     *  @param  void
     *  @return 成功返回true,失败返回false
     *  @note   使信号量值增加1,释放资源
     */
    virtual bool post();
    /**
     *  @brief  获取信号量值
     *  @param  value 当前值
     *  @return 成功返回true,失败返回false
     *  @note   当value>0说明有资源,=0无资源,<0表示有|value|个线程在等待资源
     */
    virtual bool getValue(int* value);

protected:
    sem_t m_sem;
    std::string m_name;
};

/**
 *  @class SemaphoreV
 *  @brief 信号量类(基于systemV接口)
 *  @note  使用systemV实现的进程间信号量
 */
class SemaphoreV : public Semaphore {
    DECL_CLASSNAME(SemaphoreV)

public:
    SemaphoreV();
    ~SemaphoreV();
    /**
     * @brief 打开信号量
     * @param name 信号量名称(一般为一个文件名)
     * @param value 初始值(创建时初始化),当信号量已经存在时忽略该值
     * @return true
     * @return false
     */
    bool open(const std::string& name, int value) final;
    /**
     * @brief 关闭信号量
     */
    bool close() final;
    /**
     * @brief 等待信号量
     * @param msTimeout 超时时间
     * @return true
     * @return false
     */
    bool wait(int msTimeout = -1) final;
    /**
     * @brief 发送信号量
     * @return true
     * @return false
     */
    bool post() final;
    /**
     * @brief 获取信号量值
     * @param value
     * @return true
     * @return false
     */
    bool getValue(int* value) final;

private:
    int m_semid{-1};
};
}  // namespace zemb
#endif
