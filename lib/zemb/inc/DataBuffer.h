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
#ifndef __ZEMB_DATA_BUFFER_H__
#define __ZEMB_DATA_BUFFER_H__

#include <memory>

#include "BaseType.h"
#include "ThreadUtil.h"
/**
 * @file DataBuffer.h
 * @brief 数据缓冲
 */
namespace zemb {

class DataBuffer {
    DECL_CLASSNAME(DataBuffer)

public:
    DataBuffer() {}
    virtual ~DataBuffer() {}
    /**
     * @brief 初始化
     * @param capacity 容量
     * @return true
     * @return false
     */
    virtual bool init(int capacity) = 0;
    /**
     * @brief 清空缓冲区
     */
    virtual void clear() = 0;
    /**
     * @brief 获取总容量
     * @return int
     */
    virtual int capacity() = 0;
    /**
     * @brief 获取剩余空间
     * @return int
     */
    virtual int space() = 0;
    /**
     * @brief 获取实际存储的数据长度
     * @return int
     */
    virtual int size() = 0;
    /**
     * @brief 放入数据
     * @param data
     * @param bytes
     * @return int
     */
    virtual int putData(char* data, int bytes) = 0;
    /**
     * @brief 取出数据
     * @param data
     * @param bytes
     * @return int
     */
    virtual int getData(char* data, int bytes) = 0;
};

/**
 * @class RingBuffer
 * @brief 循环缓冲
 */
class RingBuffer : public DataBuffer {
    DECL_CLASSNAME(RingBuffer)
public:
    RingBuffer();
    ~RingBuffer();
    bool init(int capacity) override;
    void clear() override;
    int capacity() override;
    int space() override;
    int size() override;
    int putData(char* data, int bytes) override;
    int getData(char* data, int bytes) override;

private:
    int m_startPos{0};
    int m_endPos{0};
    int m_occupant{0};
    int m_capacity{0};
    std::unique_ptr<char[]> m_bufPtr{nullptr};
    Mutex m_mutex;
};

/**
 * @class LineBuffer
 * @brief 线性缓冲
 */
class LineBuffer : public DataBuffer {
public:
    LineBuffer();
    ~LineBuffer();
    bool init(int capacity) override;
    void clear() override;
    int capacity() override;
    int space() override;
    int size() override;
    int putData(char* data, int size) override;
    int getData(char* buf, int size) override;
    int find(char* str, int size);
    char operator[](int idx);

private:
    Mutex m_mutex;
    char* m_buffer;
    char* m_endpos;
    char* m_curpos;
    int m_capacity{0};
};
}  // namespace zemb
#endif
