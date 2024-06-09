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
#include "DataBuffer.h"

#include <string.h>

#include <iostream>

#include "StrUtil.h"

namespace zemb {
RingBuffer::RingBuffer() {}

RingBuffer::~RingBuffer() {}

bool RingBuffer::init(int capacity) {
    if (!m_bufPtr && capacity > 0) {
        m_bufPtr = std::make_unique<char[]>(capacity);
        if (m_bufPtr) {
            m_capacity = capacity;
            return true;
        }
    }
    return false;
}

void RingBuffer::clear() {
    AutoLock lock(m_mutex);
    m_startPos = 0;
    m_endPos = 0;
    m_occupant = 0;
}

int RingBuffer::capacity() { return m_capacity; }

int RingBuffer::space() { return m_capacity - m_occupant; }

int RingBuffer::size() { return m_occupant; }

int RingBuffer::putData(char* data, int bytes) {
    if (bytes == 0) {
        return 0;
    }
    AutoLock lock(m_mutex);
    int capacity = m_capacity;
    int bytesToWrite = MIN(bytes, capacity - m_occupant);
    if (bytesToWrite <= capacity - m_endPos) {
        memcpy(m_bufPtr.get() + m_endPos, data, bytesToWrite);
        m_endPos += bytesToWrite;
        if (m_endPos == capacity) {
            m_endPos = 0;
        }
    } else {
        int sizeP1 = capacity - m_endPos;
        int sizeP2 = bytesToWrite - sizeP1;
        memcpy(m_bufPtr.get() + m_endPos, data, sizeP1);
        memcpy(m_bufPtr.get(), data + sizeP1, sizeP2);
        m_endPos = sizeP2;
    }

    m_occupant += bytesToWrite;
    return bytesToWrite;
}

int RingBuffer::getData(char* data, int bytes) {
    if (bytes == 0) {
        return 0;
    }
    AutoLock lock(m_mutex);
    int capacity = m_capacity;
    int bytesToRead = MIN(bytes, m_occupant);
    if (bytesToRead <= capacity - m_startPos) {
        memcpy(data, m_bufPtr.get() + m_startPos, bytesToRead);
        m_startPos += bytesToRead;
        if (m_startPos == capacity) {
            m_startPos = 0;
        }
    } else {
        int sizeP1 = capacity - m_startPos;
        int sizeP2 = bytesToRead - sizeP1;
        memcpy(data, m_bufPtr.get() + m_startPos, sizeP1);
        memcpy(data + sizeP1, m_bufPtr.get(), sizeP2);
        m_startPos = sizeP2;
    }

    m_occupant -= bytesToRead;
    return bytesToRead;
}

LineBuffer::LineBuffer() {}

LineBuffer::~LineBuffer() {
    if (m_buffer) {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

bool LineBuffer::init(int capacity) {
    AutoLock lock(m_mutex);
    m_buffer = reinterpret_cast<char*>(calloc(1, capacity));
    if (m_buffer) {
        m_curpos = m_buffer;
        m_endpos = m_buffer;
        m_capacity = capacity;
        return true;
    }
    return false;
}

void LineBuffer::clear() {
    AutoLock lock(m_mutex);
    memset(m_buffer, 0, m_capacity);
    m_curpos = m_buffer;
    m_endpos = m_buffer;
}

int LineBuffer::capacity() { return m_capacity; }

int LineBuffer::space() {
    AutoLock lock(m_mutex);
    return m_capacity - (m_endpos - m_curpos);
}

int LineBuffer::size() {
    AutoLock lock(m_mutex);
    return m_endpos - m_curpos;
}

int LineBuffer::putData(char* data, int size) {
    AutoLock lock(m_mutex);
    if (data && size > 0) {
        if ((m_capacity - (m_endpos - m_curpos)) <= size) {
            /* 缓冲区无法容纳新数据 */
            return 0;
        }

        int space = m_buffer + m_capacity - m_endpos;
        if (space > size) { /* 缓冲区剩余空间大于要加载的数据大小 */
            memcpy(m_endpos, data, size);
            m_endpos += size;
        } else {
            /* 把剩余数据移动到缓冲头 */
            int left = m_endpos - m_curpos;
            memmove(m_buffer, m_curpos, left);
            m_curpos = m_buffer;
            m_endpos = m_curpos + left;
            memcpy(m_endpos, data, size);
            m_endpos += size;
        }
        return size;
    }
    return 0;
}

int LineBuffer::getData(char* buf, int size) {
    AutoLock lock(m_mutex);
    if (buf && size > 0) {
        int left = m_endpos - m_curpos;
        if (left <= 0) {
            /* 缓冲区为空 */
            return 0;
        }
        left = MIN(left, size);
        memcpy(buf, m_curpos, left);
        m_curpos += left;
        if (m_curpos == m_endpos) {
            m_curpos = m_buffer;
            m_endpos = m_buffer;
        }
        return left;
    }
    return 0;
}

int LineBuffer::find(char* str, int size) {
    char* findPos =
        StrUtil::memString(m_curpos, m_endpos - m_curpos, str, size);
    if (findPos) {
        return findPos - m_buffer;
    }
    return -1;
}

char LineBuffer::operator[](int idx) {
    char rc = m_buffer[idx];
    return rc;
}
}  // namespace zemb
