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
#include "IODevice.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Tracer.h"

namespace zemb {
IODevice::IODevice() {}

IODevice::~IODevice() { close(); }

bool IODevice::open(const std::string& devName, int mode) {
    if (devName.empty() || (mode < 0) || (mode > IO_MODE_APPEND_ONLY)) {
        TRACE_ERR_CLASS("Parameter error.");
        return false;
    }

    if (m_fd >= 0) {
        TRACE_ERR_CLASS("Device is already opened!");
        return false;
    }

    switch (mode) {
        case IO_MODE_RD_ONLY:
            m_fd = ::open(CSTR(devName), O_RDONLY);
            break;
        case IO_MODE_WR_ONLY:
            m_fd = ::open(CSTR(devName), O_WRONLY);
            break;
        case IO_MODE_RDWR_ONLY:
            m_fd = ::open(CSTR(devName), O_RDWR);
            break;
        case IO_MODE_APPEND_ONLY:
            m_fd = ::open(CSTR(devName), O_RDWR, O_CREAT | O_APPEND);
            break;
        default:
            TRACE_ERR_CLASS("Unsupport IO Mode: %d", mode);
            return false;
    }

    if (m_fd < 0) {
        TRACE_ERR_CLASS("Open %s error: %s", CSTR(devName), ERRSTR);
        return false;
    }
    m_devName = devName;
    m_openMode = mode;
    return true;
}

bool IODevice::close() {
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
    return true;
}

int IODevice::readData(char* buf, int len) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_fd < 0) {
        TRACE_ERR_CLASS("device not open.");
        return RC_ERROR;
    }

    rc = ::read(m_fd, buf, len);
    if (rc < 0) {
        if (rc == -1 && errno != EAGAIN) {
            TRACE_ERR_CLASS("read error:%s", ERRSTR);
        }
        return RC_ERROR;
    }
    return rc;
}
int IODevice::writeData(const char* buf, int len) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_fd < 0) {
        TRACE_ERR_CLASS("device not open.");
        return RC_ERROR;
    }

    rc = ::write(m_fd, buf, len);
    if (rc < 0) {
        if (rc == -1 && errno != EAGAIN) {
            TRACE_ERR_CLASS("write error:%s", ERRSTR);
        }
        return RC_ERROR;
    }
    return rc;
}

int IODevice::recvData(char* buf, int count, int usTimeout) { return RC_ERROR; }

int IODevice::sendData(const char* buf, int count, int usTimeout) {
    return RC_ERROR;
}

int IODevice::setAttribute(int attr, int value) { return RC_ERROR; }

int IODevice::getAttribute(int attr) { return RC_ERROR; }

int IODevice::fd() { return m_fd; }

bool IODevice::isOpen() { return (m_fd > 0) ? true : false; }

bool IODevice::reopen() {
    close();
    return open(m_devName, m_openMode);
}

Fifo::Fifo() {}
Fifo::~Fifo() {}
bool Fifo::open(const char* devName, int ioMode) {
    if (devName == nullptr) {
        TRACE_ERR_CLASS("fifo name is nullptr.");
        return false;
    }
    bool exist = false;
    if (0 == access(devName, F_OK)) {
        struct stat fileStat;
        if (0 == lstat(devName, &fileStat)) {
            if (!S_ISDIR(fileStat.st_mode)) {
                exist = true;
            }
        }
    }
    if (!exist) {
        int rc = mkfifo(devName, S_IFIFO | 0666);
        if (rc < 0) {
            TRACE_ERR_CLASS("mkfifo error:%s.", ERRSTR);
            return false;
        }
    }

    if (m_fd >= 0) {
        TRACE_ERR_CLASS("Device is already opened!");
        return false;
    }

    switch (ioMode) {
        case IO_MODE_RD_ONLY:
            m_fd = ::open(devName, O_RDONLY | O_NONBLOCK);
            break;
        case IO_MODE_WR_ONLY:
            m_fd = ::open(devName, O_WRONLY | O_NONBLOCK);
            break;
        case IO_MODE_RDWR_ONLY:
            m_fd = ::open(devName, O_RDWR | O_NONBLOCK);
            break;
        default:
            TRACE_ERR_CLASS("Unsupport IO Mode: %d", ioMode);
            return false;
    }

    if (m_fd < 0) {
        TRACE_ERR_CLASS("Open %s error: %s", devName, ERRSTR);
        return false;
    }
    m_devName = devName;
    m_openMode = ioMode;
    return true;
}
}  // namespace zemb
