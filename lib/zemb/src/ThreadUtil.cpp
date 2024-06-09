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
#include "ThreadUtil.h"

#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#include "Tracer.h"

namespace zemb {
Mutex::Mutex() { pthread_mutex_init(&m_mutex, nullptr); }

Mutex::~Mutex() { pthread_mutex_destroy(&m_mutex); }

int Mutex::lock() { return pthread_mutex_lock(&m_mutex); }

int Mutex::unLock() { return pthread_mutex_unlock(&m_mutex); }

int Mutex::tryLock() { return pthread_mutex_trylock(&m_mutex); }

void RWMutex::lockR() {
    m_rMutex.lock();
    if (++m_readCnts == 1) {
        m_wMutex.lock(); /* 存在线程读操作时,写加锁(只加一次) */
    }
    m_rMutex.unLock();
}

void RWMutex::unLockR() {
    m_rMutex.lock();
    if (--m_readCnts == 0) {
        m_wMutex.unLock(); /* 没有线程读操作时,释放写锁 */
    }
    m_rMutex.unLock();
}

void RWMutex::lockW() { m_wMutex.lock(); }

void RWMutex::unLockW() { m_wMutex.unLock(); }

AutoLock::AutoLock(const Mutex& mutex) : m_mutex(const_cast<Mutex*>(&mutex)) {
    m_mutex->lock();
}

AutoLock::~AutoLock() {
    if (m_mutex) {
        m_mutex->unLock();
    }
}

MutexCond::MutexCond() {
#if defined OS_CYGWIN || defined OS_UNIX
    pthread_condattr_init(&m_condAttr);
    pthread_condattr_setclock(&m_condAttr, CLOCK_MONOTONIC);
#endif
    pthread_cond_init(&m_cond, &m_condAttr);
}

MutexCond::~MutexCond() {}

int MutexCond::wait(int usec) {
    int rc;
    if (usec >= 0) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        time_t sec = usec / 1000000;
        if (sec > 0) {
            ts.tv_sec += sec;
        }
        long nsec = (usec % 1000000) * 1000; /* NOLINT */
        ts.tv_nsec += nsec;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec = nsec - 1000000000;
        }
#if defined OS_CYGWIN || defined OS_UNIX
        rc = pthread_cond_timedwait(
            &m_cond, &m_mutex,
            &ts);  // wait函数内部会自动解锁,直到等到条件成立或超时
#else
        /* for Android bionic pthread */
        rc = pthread_cond_timedwait_monotonic_np(&m_cond, &m_mutex, &ts);
#endif
    } else {
        rc = pthread_cond_wait(&m_cond, &m_mutex);
    }
    switch (rc) {
        case 0:
            return RC_OK;
        case ETIMEDOUT:
            return RC_TIMEOUT;
        default:
            return RC_ERROR;
    }
}

int MutexCond::meet() {
    return (pthread_cond_signal(&m_cond) == 0) ? RC_OK : RC_ERROR;
    // return (pthread_cond_broadcast(&m_cond)==0)?RC_OK:RC_ERROR;
}

Semaphore::Semaphore() {}
Semaphore::~Semaphore() {}
bool Semaphore::open(const std::string& name, int value) {
    m_name = name;
    if (m_name.empty()) {
        if (value < 0) {
            return false;
        }
        /* 初始化无名信号量 */
        if (-1 == sem_init(&m_sem, 0, value)) {
            TRACE_ERR_CLASS("sem init error:%s", ERRSTR);
            return false;
        }
    } else {
        /* 有名信号量,名称必须以'/'开头,see"sem_overview" */
        m_name = std::string("/") + m_name;
        sem_t* sem =
            sem_open(CSTR(m_name), O_CREAT | O_RDWR | O_EXCL, 0664, value);
        if (sem == SEM_FAILED) { /* 信号量已存在 */
            if (value < 0) {     /* 仅打开,不初始化值 */
                sem = sem_open(CSTR(m_name), 0);
                if (sem == SEM_FAILED) {
                    TRACE_ERR_CLASS("sem_open error: %s", ERRSTR);
                    return false;
                }
            } else { /* 重新打开并初始化值 */
                unlink();
                sem = sem_open(CSTR(m_name), O_CREAT | O_RDWR | O_EXCL, 0664,
                               value);
                if (sem == SEM_FAILED) {
                    TRACE_ERR_CLASS("sem open with value(%d) error: %s", value,
                                    ERRSTR);
                    return false;
                }
            }
        }
        m_sem = *sem;
    }
    return true;
}

bool Semaphore::close() {
    if (m_name.empty()) {
        if (sem_destroy(&m_sem) != 0) {
            TRACE_ERR_CLASS("sem destroy error:%s", ERRSTR);
            return false;
        }
    } else {
        if (sem_close(&m_sem) != 0) {
            TRACE_ERR_CLASS("sem close error:%s", ERRSTR);
            return false;
        }
    }
    return true;
}

bool Semaphore::unlink() {
    if (!m_name.empty()) {
        if (sem_unlink(CSTR(m_name)) != 0) {
            TRACE_ERR_CLASS("sem unlink(%s) error:%s", CSTR(m_name), ERRSTR);
            return false;
        }
    }
    return true;
}

bool Semaphore::wait(int msTimeout) {
    if (0 == sem_wait(&m_sem)) {
        return true;
    }
    return false;
}

bool Semaphore::tryWait() {
    if (0 == sem_trywait(&m_sem)) {
        return true;
    }
    return false;
}

bool Semaphore::post() {
    if (0 == sem_post(&m_sem)) {
        return true;
    }
    return false;
}

bool Semaphore::getValue(int* value) {
    if (-1 == sem_getvalue(&m_sem, value)) {
        return false;
    }
    return true;
}

/**
 * @union semun
 * @brief systemV信号量属性
 */
union semun {
    int val;              /* Value for SETVAL */
    struct semid_ds* buf; /* Buffer for IPC_STAT, IPC_SET */
    unsigned short* array; /* Array for GETALL, SETALL */ /* NOLINT */
    struct seminfo* __buf; /* Buffer for IPC_INFO (Linux-specific) */
};
SemaphoreV::SemaphoreV() {}
SemaphoreV::~SemaphoreV() {}
bool SemaphoreV::open(const std::string& name, int value) {
    if (m_semid >= 0) {
        TRACE_ERR_CLASS("sem[%s] is allready opened!", CSTR(m_name));
        return false;
    }
    auto key = ftok(CSTR(name), 1);
    if (key < 0) {
        TRACE_ERR_CLASS("sem[%s] get key error: %s!", CSTR(name), ERRSTR);
        return false;
    }
    int semid = semget((key_t)key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid < 0) {
        // IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        semid = semget((key_t)key, 1, IPC_CREAT | 0666);
        if (semid < 0) {
            TRACE_ERR_CLASS("sem[%s] open error: %s", CSTR(name), ERRSTR);
            return false;
        }
        if (value >= 0) { /* 重新打开并初始化值 */
            union semun sem_union;
            if (semctl(semid, 0, IPC_RMID, sem_union) < 0) {
                TRACE_ERR_CLASS("sem[%s] rmid error: %s!", CSTR(name), ERRSTR);
                return false;
            }
            semid = semget((key_t)key, 1, IPC_CREAT | 0666);
        }
    }
    if (semid < 0) {
        TRACE_ERR_CLASS("sem[%s] open error: %s", CSTR(name), ERRSTR);
        return false;
    }
    if (value >= 0) {
        union semun sem_union;
        sem_union.val = value;
        if (semctl(semid, 0, SETVAL, sem_union) < 0) {
            TRACE_ERR_CLASS("sem[%s] setval(%d) error: %s", CSTR(name), value,
                            ERRSTR);
            this->close();
            return false;
        }
    }
    m_name = name;
    m_semid = semid;
    return true;
}

bool SemaphoreV::close() {
    if (m_semid < 0) {
        return true;
    }
    union semun sem_union;
    if (semctl(m_semid, 0, IPC_RMID, sem_union) < 0) {
        TRACE_ERR_CLASS("sem[%s] close error: %s!", CSTR(m_name), ERRSTR);
        return false;
    }
    m_semid = -1;
    return true;
}

bool SemaphoreV::wait(int msTimeout) {
    if (m_semid < 0) {
        return true;
    }
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = -1;
    semb.sem_flg = SEM_UNDO;
    if (semop(m_semid, &semb, 1) < 0) {
        TRACE_ERR_CLASS("sem[%s] wait error: %s!", CSTR(m_name), ERRSTR);
        return false;
    }
    return true;
}

bool SemaphoreV::post() {
    if (m_semid < 0) {
        return true;
    }
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = 1;
    semb.sem_flg = SEM_UNDO;
    if (semop(m_semid, &semb, 1) < 0) {
        TRACE_ERR_CLASS("sem[%s] post error: %s!", CSTR(m_name), ERRSTR);
        return false;
    }
    return true;
}

bool SemaphoreV::getValue(int* value) {
    if (m_semid < 0) {
        TRACE_ERR_CLASS("sem[%s] not open!", CSTR(m_name));
        return false;
    }
    union semun sem_union;
    if (semctl(m_semid, 0, GETVAL, sem_union) < 0) {
        TRACE_ERR_CLASS("sem[%s] getval error: %s!", CSTR(m_name), ERRSTR);
        return false;
    }
    *value = sem_union.val;
    return true;
}
}  // namespace zemb
