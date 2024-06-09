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
#ifndef __ZEMB_SERIAL_PORT_H__
#define __ZEMB_SERIAL_PORT_H__

#include <termios.h>
#include <unistd.h>

#include <string>

#include "IODevice.h"

/**
 * @file SerialPort.h
 * @brief 串口
 */
namespace zemb {
/**
 * @enum BAUDRATE_E
 * @brief 串口波特率
 */
enum BAUDRATE_E {
    BAUDRATE_1200 = 1200,
    BAUDRATE_2400 = 2400,
    BAUDRATE_4800 = 4800,
    BAUDRATE_9600 = 9600,
    BAUDRATE_19200 = 19200,
    BAUDRATE_38400 = 38400,
    BAUDRATE_57600 = 57600,
    BAUDRATE_115200 = 115200,
    BAUDRATE_230400 = 230400,
    BAUDRATE_460800 = 460800,
    BAUDRATE_921600 = 921600,
};

/**
 * @enum DATABITS_E
 * @brief 串口数据位
 */
enum DATABITS_E { DATABITS_5 = 5, DATABITS_6, DATABITS_7, DATABITS_8 };

/**
 * @enum STOPBITS_E
 * @brief 串口停止位
 */
enum STOPBITS_E { STOPBITS_1 = 1, STOPBITS_2 };

/**
 * @enum PARITY_E
 * @brief 串口校验位值
 */
enum PARITY_E { PARITY_NONE = 0, PARITY_ODD, PARITY_EVEN, PARITY_SPACE };

/**
 * @enum FLOWCTRL_E
 * @brief 串口流控位值
 */
enum FLOWCTRL_E { FLOWCTRL_NONE = 0, FLOWCTRL_HW, FLOWCTRL_SW };

/**
 * @enum DATAMODE_E
 * @brief 串口模式
 */
enum DATAMODE_E {
    DATAMODE_RS232 = 0, /* 数字全双工 */
    DATAMODE_RS422,     /* 差分全双工 */
    DATAMODE_RS485,     /* 差分半双工 */
};

/**
 * @enum RS485DIR_E
 * @brief RS485方向属性
 * @note 由于RS485是半双工的,读写前需要设置方向
 */
enum RS485DIR_E {
    RS485DIR_RX = 0,
    RS485DIR_TX,
};

/**
 * @struct SerialPortAttr
 * @brief 串口属性
 */
struct SerialPortAttr {
    int m_bandrate;
    int m_databits;
    int m_stopbits;
    int m_parity;
    int m_flowctrl;
    int m_datamode;
    int m_rs485dir;
};

/**
 *  @class  SerialPort
 *  @brief  串口设备类.
 */
class SerialPort : public IODevice {
    DECL_CLASSNAME(SerialPort)

public:
    /**
     * @enum SERIAL_ATTR_E
     * @brief 串口属性类型
     */
    enum SERIAL_ATTR_E {
        ATTR_BAUDRATE = 0,
        ATTR_DATABITS,
        ATTR_STOPBITS,
        ATTR_PARITY,
        ATTR_FLOWCTRL,
        ATTR_DATAMODE,
        ATTR_RS485DIR,
    };

public:
    SerialPort();
    virtual ~SerialPort();
    /**
     *  @brief  打开串口
     *  @param  devName 设备名称,如/dev/ttySAC0
     *  @param  mode 设备属性,如IO_MODE_RD_ONLY
     *  @return 成功返回true,失败返回false
     *  @note   如果要修改串口属性,必须在open前进行设置
     */
    virtual bool open(const std::string& devName, int ioMode);
    /**
     *  @brief  关闭串口
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool close();
    /**
     *  @brief  读串口数据
     *  @param  buf 存放读取的数据的缓存
     *  @param  len buf缓存的大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回读取的数据长度,失败返回STATUS_ERROR
     */
    virtual int recvData(char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  写串口数据
     *  @param  data 要写入的数据
     *  @param  len 写入数据的长度
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回写入的数据长度,失败返回STATUS_ERROR
     */
    virtual int sendData(const char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  设置串口属性
     *  @param  attr 属性类型SERIAL_ATTR_E
     *  @param  value 属性对应的值
     *  @return 成功返回STATUS_OK,失败返回STATUS_ERROR
     */
    virtual int setAttribute(int attr, int value);
    /**
     *  @brief  获取串口属性
     *  @param  attr 属性类型SERIAL_ATTR_E
     *  @return 成功返回属性对应的值,失败返回STATUS_ERROR
     */
    virtual int getAttribute(int attr);

private:
    SerialPortAttr m_attr{BAUDRATE_115200, DATABITS_8, STOPBITS_1, PARITY_NONE,
                          FLOWCTRL_NONE};
};
}  // namespace zemb

#endif
