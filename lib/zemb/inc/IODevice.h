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
#ifndef __ZEMB_IODEVICE_H__
#define __ZEMB_IODEVICE_H__

#include <iostream>
#include <string>

#include "BaseType.h"

/**
 * @file IODevice.h
 * @brief 输入输出设备
 */

namespace zemb {
/** 定义输入输出模式 */
enum IO_MODE_E {
    IO_MODE_RD_ONLY = 0,    /**< 只可读(r) */
    IO_MODE_WR_ONLY,        /**< 只可写 */
    IO_MODE_RDWR_ONLY,      /**< 只可读写(r+) */
    IO_MODE_APPEND_ONLY,    /**< 只增加 */
    IO_MODE_REWR_ORNEW,     /**< 文件重写,没有则创建(w) */
    IO_MODE_RDWR_ORNEW,     /**< 文件可读写,没有则创建(w+) */
    IO_MODE_APPEND_ORNEW,   /**< 文件可增加,没有则创建(a) */
    IO_MODE_RDAPPEND_ORNEW, /**< 文件可读或可增加,没有则创建(a+) */
    IO_MODE_INVALID = 0xFF, /**< 非法模式 */
};

/**
 *  @class  IODevice
 *  @brief  IO设备抽象类
 */
class IODevice {
    DECL_CLASSNAME(IODevice)

public:
    IODevice();
    virtual ~IODevice();
    /**
     *  @brief  打开设备
     *  @param  devName 设备全名(包含路径,如:/dev/ttyS0)
     *  @param  mode 打开模式IO_MODE_E
     *  @return 设备打开成功返回true,失败返回false
     *  @note   none
     */
    virtual bool open(const std::string& devName, int ioMode = IO_MODE_INVALID);
    /**
     *  @brief  关闭设备
     *  @param  void
     *  @return 设备关闭成功返回true,失败返回false
     *  @note   none
     */
    virtual bool close();
    /**
     * @brief 读数据
     * @param buf
     * @param len
     * @return int
     */
    virtual int readData(char* buf, int len);
    /**
     * @brief 写入数据
     * @param buf
     * @param len
     * @return int
     */
    virtual int writeData(const char* buf, int len);
    /**
     * @brief 接收数据
     * @param buf
     * @param len
     * @param usTimeout
     * @return int
     */
    virtual int recvData(char* buf, int len, int usTimeout = -1);
    /**
     * @brief 发送数据
     * @param buf
     * @param len
     * @param usTimeout
     * @return int
     */
    virtual int sendData(const char* buf, int len, int usTimeout = -1);
    /**
     * @brief 设置属性
     * @param attr
     * @param value
     * @return int
     */
    virtual int setAttribute(int attr, int value);
    /**
     * @brief 获取属性值
     * @param attr
     * @return int
     */
    virtual int getAttribute(int attr);
    /**
     * @brief 获取设备描述符
     * @return int
     */
    virtual int fd();
    /**
     * @brief 判断设备是否打开
     * @return true
     * @return false
     */
    virtual bool isOpen();

    /**
     * @brief 重新打开设备
     * @return true
     * @return false
     */
    virtual bool reopen();

protected:
    int m_fd{-1};
    int m_openMode{-1};
    std::string m_devName{""};
};

/**
 * @class Fifo
 * @brief 进程间管道(有名管道)
 */
class Fifo : public IODevice {
    DECL_CLASSNAME(Fifo)
public:
    Fifo();
    virtual ~Fifo();
    /**
     * @brief 打开有名管道
     * @param devName 管道名称
     * @param ioMode 设备模式(IO_MODE_E)
     * @return true 打开成功
     * @return false 打开失败
     */
    virtual bool open(const char* devName, int ioMode = IO_MODE_RDWR_ONLY);
};

}  // namespace zemb
#endif
