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
#include "MsgQueue.h"

#include <stdlib.h>
#include <sys/mount.h>

#include "FileUtil.h"
#include "Tracer.h"
#if USE_MQ_SYSV
#else
#include <time.h>
#include <unistd.h>
#endif

#define MAX_MESSAGES 200 /* 最大允许的消息条数 */

namespace zemb {
MsgQueue::MsgQueue(key_t key) {}

MsgQueue::~MsgQueue() {
#if USE_MQ_SYSV
#else
    mq_unlink(CSTR(getMqName(m_key)));
#endif
}

std::string MsgQueue::getMqName(int key) {
#if USE_MQ_SYSV
    return "";
#else
    char mqName[32] = {0};
    snprintf(mqName, sizeof(mqName), "/mq_%x", key);
    return std::string(mqName);
#endif
}

bool MsgQueue::initialize() {
#if USE_MQ_SYSV
    m_msgID = msgget(m_key, 0666 | IPC_CREAT);
#else
    if (!Directory::exists(POSIX_MQFS)) {
        if (!Directory::createDir(POSIX_MQFS, 0666)) {
            return false;
        }
        // SysUtil::execute("mount -t mqueue none "POSIX_MQFS);
        if (mount("none", POSIX_MQFS, "mqueue", 0, nullptr) != 0) {
            TRACE_ERR_CLASS("mount mqueue error!");
            return false;
        }
    }
    mq_unlink(CSTR(getMqName(m_key)));
    int oflags = O_RDWR | O_CREAT | O_EXCL;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE_MAX + 4;
    m_msgID =
        static_cast<int>(mq_open(CSTR(getMqName(m_key)), oflags, mode, &attr));
#endif
    if (m_msgID < 0) {
        TRACE_ERR_CLASS("MsgQueue(key:0x%x) initialize failed:%s.", m_key,
                        ERRSTR);
        return false;
    }
    return true;
}

int MsgQueue::sendMsg(const QueueMsgBody& msg) {
    int rc = 0;
    if (msg.m_msgType <= 0) {
        TRACE_ERR_CLASS("msg type must be greater than zero!");
        return RC_ERROR;
    }
    if (msg.m_dataLen >= MSG_SIZE_MAX) {
        TRACE_ERR_CLASS("msg data len must be less than %d!", MSG_SIZE_MAX);
        return RC_ERROR;
    }
#if USE_MQ_SYSV
    rc = msgsnd(m_msgID,
                reinterpret_cast<void*>(const_cast<QueueMsgBody*>(&msg)),
                msg.m_dataLen + sizeof(msg.m_dataLen), 0);
#else
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;
    rc = mq_timedsend((mqd_t)m_msgID, (const char*)(&msg), MSG_SIZE_MAX + 4, 0,
                      &timeout);
#endif
    if (rc < 0) {
        // TRACE_ERR_CLASS("MsgQueue(key:0x%x) send msg(type:%d,datalen:%d)
        // failed.",m_key,msg.m_msgType,msg.m_dataLen);
        return RC_ERROR;
    }
    return RC_OK;
}

int MsgQueue::recvMsg(const QueueMsgBody& msg, int msgType) {
    int rc = 0;
#if USE_MQ_SYSV
    rc = msgrcv(m_msgID,
                reinterpret_cast<void*>(const_cast<QueueMsgBody*>(&msg)),
                MSG_SIZE_MAX, (long)msgType, /* NOLINT */
                IPC_NOWAIT | MSG_NOERROR);
#else
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;
    rc = mq_timedreceive(
        (mqd_t)m_msgID,
        reinterpret_cast<char*>(const_cast<QueueMsgBody*>(&msg)),
        MSG_SIZE_MAX + 4, 0, &timeout);
#endif
    if (rc < 0) {
        // TRACE_ERR_CLASS("MsgQueue(key:0x%x) recieve message(%d)
        // failed(%s).",m_key, msgType,ERRSTR);
        return RC_ERROR;
    }
    return RC_OK;
}

int MsgQueue::clearMsg(int msgType) {
    QueueMsgBody msg;
    while (recvMsg(msg, msgType) == RC_OK) {
        continue;
    }
    return RC_OK;
}

MsgQueueFactory::MsgQueueFactory() {}

MsgQueueFactory::~MsgQueueFactory() {}

std::shared_ptr<MsgQueue> MsgQueueFactory::getMsgQueue(key_t key) {
    AutoLock lock(m_mutex);
    auto iter = m_msgQueueMap.find(key);
    if (iter != m_msgQueueMap.end()) {
        return iter->second;
    } else {
        auto pMsg = std::make_shared<MsgQueue>(key);
        if (!pMsg) {
            return nullptr;
        }

        if (pMsg->initialize()) {
            m_msgQueueMap.insert(std::make_pair(key, pMsg));
            return pMsg;
        } else {
            TRACE_ERR_CLASS("create MsgQueue[key=0x%x] failed!", key);
            return nullptr;
        }
    }
}
}  // namespace zemb
