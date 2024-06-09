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
#ifndef __ZEMB_MSG_QUEUE_H__
#define __ZEMB_MSG_QUEUE_H__

#include <string>

#include "BaseType.h"
#include "Singleton.h"
#include "ThreadUtil.h"
/**
 * @file MsgQueue.h
 * @brief 消息队列
 */

#if defined OS_CYGWIN || defined OS_UNIX
#define USE_MQ_SYSV (1) /* cygwin和linux下只能使用sysv的消息队列 */
#else
#define USE_MQ_SYSV (0) /* Android下使用posix消息队列 */
#endif
#include <sys/types.h>
#if USE_MQ_SYSV
#include <sys/ipc.h>
#include <sys/msg.h>
#else
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#endif

#include <iostream>
#include <map>
#include <memory>

#define MSG_SIZE_MAX 128         /**< 最大消息长度 */
#define POSIX_MQFS "/dev/mqueue" /**< Posix消息队列挂载点 */

namespace zemb {
/**
 *  @struct QueueMsgBody
 *  @brief  msgbuf消息结构体
 */
struct QueueMsgBody {
    int m_msgType;                 /**< 消息类型 */
    unsigned char m_dataLen;       /**< 数据长度 */
    char m_data[MSG_SIZE_MAX - 1]; /**< 消息数据 */
};

/**
 *  @class  MsgQueue
 *  @brief  消息队列类
 */
class MsgQueue {
    DECL_CLASSNAME(MsgQueue)

public:
    explicit MsgQueue(key_t key);
    virtual ~MsgQueue();
    /**
     *  @brief  初始化消息队列
     *  @param  void
     *  @return true
     *  @return false
     */
    bool initialize();
    /**
     *  @brief  发送消息
     *  @param  msg 消息体
     *  @return 成功返回RC_OK,失败返回RC_ERROR
     */
    int sendMsg(const QueueMsgBody& msg);
    /**
     *  @brief  接收消息
     *  @param  msg 消息体
     *  @param  msgType 消息类型
     *  @return 成功返回RC_OK,失败返回RC_ERROR
     */
    int recvMsg(const QueueMsgBody& msg, int msgType = 0);
    /**
     *  @brief  清空消息
     *  @param  msgType 消息体
     *  @return 成功返回STATUS_OK,失败返回STATUS_ERROR
     *  @note   msgType=0 清空队列中所有的消息
     *          msgType>0 清空队列中类型值为msgType的消息
     *          msgType<0 清空队列中类型值小于或等于|msgType|的消息
     */
    int clearMsg(int msgType = 0);

private:
    std::string getMqName(int key);

private:
    int m_msgID{-1};
    key_t m_key{-1};
};

/**
 *  @class  MsgQueueFactory
 *  @brief  消息队列工厂类
 */
class MsgQueueFactory : public Singleton<MsgQueueFactory> {
    DECL_CLASSNAME(MsgQueueFactory)
    DECL_SINGLETON(MsgQueueFactory)
public:
    ~MsgQueueFactory();
    /**
     * @brief 从消息队列工厂取得消息队列
     * @param key
     * @return std::shared_ptr<MsgQueue>
     */
    std::shared_ptr<MsgQueue> getMsgQueue(key_t key);

private:
    Mutex m_mutex;
    std::map<key_t, std::shared_ptr<MsgQueue>> m_msgQueueMap;
};
}  // namespace zemb

#endif
