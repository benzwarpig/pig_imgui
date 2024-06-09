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
#include "SerialPort.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Tracer.h"

#define SETRS485DIR 0x10 /* 自定义命令,xrm1172驱动(i2c转485) */
#ifndef TIOCSRS485
#define TIOCSRS485 0x542F
#endif
namespace zemb {
/* struct serial_rs485 is from "include/linux/serial.h" */
#define SER_RS485_ENABLED (1 << 0)
#define SER_RS485_RTS_ON_SEND (1 << 1)
#define SER_RS485_RTS_AFTER_SEND (1 << 2)
#define SER_RS485_RTS_BEFORE_SEND (1 << 3)

/**
 * @struct serial_rs485
 * @brief RS485属性
 */
struct serial_rs485 {
    uint32 flags;
    uint32 delay_rts_before_send;
    uint32 delay_rts_after_send;
    uint32 padding[5];
};

SerialPort::SerialPort() {}

SerialPort::~SerialPort() { close(); }

bool SerialPort::open(const std::string& devName, int ioMode) {
    if (devName.empty() || (ioMode < 0) || (ioMode >= IO_MODE_APPEND_ONLY)) {
        TRACE_ERR_CLASS("Parameter error.");
        return false;
    }

    if (m_fd >= 0) {
        TRACE_ERR_CLASS("Device is already opened!");
        return false;
    }

    switch (ioMode) {
        case IO_MODE_RD_ONLY:
            m_fd = ::open(CSTR(devName), O_RDONLY);
            break;
        case IO_MODE_WR_ONLY:
            m_fd =
                ::open(CSTR(devName), O_WRONLY | O_SYNC); /* O_SYNC:同步写入 */
            break;
        case IO_MODE_RDWR_ONLY:
            m_fd = ::open(CSTR(devName), O_RDWR | O_SYNC);
            break;
        default:
            TRACE_ERR_CLASS("Unsupport IO Mode: %d", ioMode);
            return false;
    }

    if (m_fd < 0) {
        TRACE_ERR_CLASS("Open %s error: %s", CSTR(devName), ERRSTR);
        return false;
    }
    m_devName = devName;
    m_openMode = ioMode;

    /* 设置属性 */
    struct termios options;
    tcgetattr(m_fd, &options);

    /* 设置波特率 */
    uint32 baud;
    switch (m_attr.m_bandrate) {
        case BAUDRATE_1200:
            baud = B1200;
            break;
        case BAUDRATE_2400:
            baud = B2400;
            break;
        case BAUDRATE_4800:
            baud = B4800;
            break;
        case BAUDRATE_9600:
            baud = B9600;
            break;
        case BAUDRATE_19200:
            baud = B19200;
            break;
        case BAUDRATE_38400:
            baud = B38400;
            break;
        case BAUDRATE_57600:
            baud = B57600;
            break;
        case BAUDRATE_115200:
            baud = B115200;
            break;
        case BAUDRATE_230400:
            baud = B230400;
            break;
        case BAUDRATE_460800:
            baud = B460800;
            break;
        case BAUDRATE_921600:
            baud = B921600;
            break;
        default:
            TRACE_ERR_CLASS("unsupport baud rate: %d", m_attr.m_bandrate);
            return false;
    }
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    /* input modes */
    options.c_iflag = 0;

    /* output modes */
    options.c_oflag &= ~OPOST;

    /* control modes */
    options.c_cflag |= CREAD; /* 使能接收器 */

    options.c_cflag &= ~CSIZE;
    switch (m_attr.m_databits) {
        case DATABITS_5:
            options.c_cflag |= CS5;
            break;
        case DATABITS_6:
            options.c_cflag |= CS6;
            break;
        case DATABITS_7:
            options.c_cflag |= CS7;
            break;
        default:
            options.c_cflag |= CS8; /* 默认8位数据位 */
            break;
    }

    switch (m_attr.m_stopbits) {
        case STOPBITS_2:
            options.c_cflag |= CSTOPB; /* 2位停止位*/
            break;
        default:
            options.c_cflag &= ~CSTOPB; /* 默认1位停止位*/
            break;
    }

    switch (m_attr.m_parity) {
        case PARITY_ODD:
            options.c_cflag |= (PARENB | PARODD); /* 奇校验 */
            break;
        case PARITY_EVEN:
            options.c_cflag &= ~PARENB; /* 偶校验 */
            options.c_cflag &= ~PARODD;
            break;
        case PARITY_SPACE:
            options.c_cflag &= ~PARENB; /* 空格校验 */
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            options.c_cflag &= ~PARENB; /* 默认无校验 */
            break;
    }

    switch (m_attr.m_flowctrl) {
        case FLOWCTRL_HW:
            options.c_cflag |= CRTSCTS;
            break;
        case FLOWCTRL_SW:
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
        default:
            options.c_cflag &= ~CRTSCTS; /* 默认无流控 */
            break;
    }

    /* control chars,1S first byte overtime & 1 byte received totally */
    options.c_cc[VTIME] = 10; /* 等待10个100毫秒 */
    options.c_cc[VMIN] = 1;   /* 至少接收一个字节 */

    /* local modes */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* 清空接收缓存 */
    tcflush(m_fd, TCIFLUSH);

    /* 设置属性 */
    if (tcsetattr(m_fd, TCSANOW, &options) != 0) {
        TRACE_ERR_CLASS("Can't set attribute!");
        return false;
    }

    /* 擦除m_fd的flags标志位(但不包括O_RDONLY,O_WRONLY,O_RDWR) */
    fcntl(m_fd, F_SETFL, 0);
    return true;
}

bool SerialPort::close() { return IODevice::close(); }

int SerialPort::recvData(char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len <= 0 || m_fd < 0) {
        TRACE_ERR_CLASS("parameter error.");
        return RC_ERROR;
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);

    struct timeval tv, *ptv;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
        ptv = &tv;
    }
    rc = ::select(m_fd + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("ERROR:read timeout.");
        return RC_TIMEOUT;
    } else if (rc < 0) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }

    if (!FD_ISSET(m_fd, &readfds)) {
        TRACE_ERR_CLASS("fd is not set.");
        return RC_ERROR;
    }
    return ::read(m_fd, buf, len);
}

int SerialPort::sendData(const char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len <= 0 || m_fd < 0) {
        TRACE_ERR_CLASS("parameter error.");
        return RC_ERROR;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(m_fd, &writefds);

    struct timeval tv, *ptv;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
        ptv = &tv;
    }
    rc = ::select(m_fd + 1, nullptr, &writefds, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("ERROR:write timeout.");
        return RC_TIMEOUT;
    } else if (rc < 0) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }

    if (!FD_ISSET(m_fd, &writefds)) {
        TRACE_ERR_CLASS("fd is not set.");
        return RC_ERROR;
    }
    return ::write(m_fd, buf, len);
}

int SerialPort::setAttribute(int attr, int value) {
    switch (attr) {
        case ATTR_DATAMODE:
        case ATTR_RS485DIR:
            if (m_fd <= 0) {
                TRACE_ERR_CLASS("Device is not open,cannot set attibute:%d!",
                                attr);
                return RC_ERROR;
            }
            break;
        default:
            if (m_fd >= 0) {
                TRACE_ERR_CLASS(
                    "Device is allready open, cannot set attribute:%d!", attr);
                return RC_ERROR;
            }
            break;
    }

    switch (attr) {
        case ATTR_BAUDRATE:
            if (value != BAUDRATE_1200 && value != BAUDRATE_2400 &&
                value != BAUDRATE_4800 && value != BAUDRATE_9600 &&
                value != BAUDRATE_19200 && value != BAUDRATE_38400 &&
                value != BAUDRATE_57600 && value != BAUDRATE_115200 &&
                value != BAUDRATE_230400 && value != BAUDRATE_460800 &&
                value != BAUDRATE_921600) {
                TRACE_ERR_CLASS("Unsupport baud:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_bandrate = (BAUDRATE_E)value;
            break;
        case ATTR_DATABITS:
            if (value != DATABITS_5 && value != DATABITS_6 &&
                value != DATABITS_7 && value != DATABITS_8) {
                TRACE_ERR_CLASS("Unsupport databits:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_databits = (DATABITS_E)value;
            break;
        case ATTR_STOPBITS:
            if (value != STOPBITS_1 && value != STOPBITS_2) {
                TRACE_ERR_CLASS("Unsupport stopbits:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_stopbits = (STOPBITS_E)value;
            break;
        case ATTR_PARITY:
            if (value != PARITY_NONE && value != PARITY_ODD &&
                value != PARITY_EVEN && value != PARITY_SPACE) {
                TRACE_ERR_CLASS("Unsupport parity:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_parity = (PARITY_E)value;
            break;
        case ATTR_FLOWCTRL:
            if (value != FLOWCTRL_SW && value != FLOWCTRL_HW &&
                value != FLOWCTRL_NONE) {
                TRACE_ERR_CLASS("Unsupport flowctrl:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_flowctrl = (FLOWCTRL_E)value;
            break;
        case ATTR_DATAMODE:
            if (value != DATAMODE_RS232 && value != DATAMODE_RS422 &&
                value != DATAMODE_RS485) {
                TRACE_ERR_CLASS("Unsupport datamode:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_datamode = (DATAMODE_E)value;
            if (m_attr.m_datamode == DATAMODE_RS485) {
                struct serial_rs485 rs485;
                rs485.flags = SER_RS485_ENABLED;
                rs485.delay_rts_after_send = 0;
                rs485.delay_rts_before_send = 0;
                ioctl(m_fd, TIOCSRS485, &rs485);
            }
            return RC_OK;
        case ATTR_RS485DIR:
            if (value != RS485DIR_RX && value != RS485DIR_TX) {
                TRACE_ERR_CLASS("Unsupport rs485dir:%d.", value);
                return RC_ERROR;
            }
            m_attr.m_rs485dir = (RS485DIR_E)value;
            ioctl(m_fd, SETRS485DIR, m_attr.m_rs485dir);
            return RC_OK;
        default:
            TRACE_ERR_CLASS("Unsupport attribute:%d.", attr);
            break;
    }
    return RC_OK;
}

int SerialPort::getAttribute(int attr) {
    switch (attr) {
        case ATTR_BAUDRATE:
            return m_attr.m_bandrate;
        case ATTR_DATABITS:
            return m_attr.m_databits;
        case ATTR_STOPBITS:
            return m_attr.m_stopbits;
        case ATTR_PARITY:
            return m_attr.m_parity;
        case ATTR_FLOWCTRL:
            return m_attr.m_flowctrl;
        case ATTR_DATAMODE:
            return m_attr.m_datamode;
        case ATTR_RS485DIR:
            return m_attr.m_rs485dir;
        default:
            return RC_ERROR;
    }
}
}  // namespace zemb
