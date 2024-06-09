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
#ifndef __ZEMB_SOCKET_H__
#define __ZEMB_SOCKET_H__

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "BaseType.h"
#include "Thread.h"
#include "ThreadUtil.h"

/**
 * @file Socket.h
 * @brief Socket
 */
namespace zemb {

#define UDP_PACKET_LEN_MAX 1472 /* UDP包最大长度 */

/**
 * @enum SOCKET_OPTION_E
 * @brief 定义socket的可选项
 */
enum SOCKET_OPTION_E {
    SOCKET_OPTION_CASTTYPE, /* 发送方式 */
    SOCKET_OPTION_RCVBUF,   /* 接收窗口大小 */
    SOCKET_OPTION_SNDBUF,   /* 发送窗口大小 */
    SOCKET_OPTION_CANBTR,   /* CAN 波特率 */
    SOCKET_OPTION_UNKNOWN = 0xFF,
};

/**
 * @enum  SOCKET_TYPE_E
 * @brief 定义socket的类型
 */
enum SOCKET_TYPE_E {
    SOCKET_TYPE_BROADCAST, /**< 广播 */
    SOCKET_TYPE_GROUPCAST, /**< 组播 */
    SOCKET_TYPE_MULTICAST, /**< 多播 */
};

/**
 * @enum TCP_LINK_E
 * @brief 定义tcp连接状态
 */
enum TCP_LINK_E {
    TCP_LINK_DISCONNECT, /**< 断开状态 */
    TCP_LINK_CONNECTED,  /**< 连接状态 */
};

/**
 *  @class  Socket
 *  @brief  Socket抽象基类
 */
class Socket {
    DECL_CLASSNAME(Socket)

public:
    Socket();
    virtual ~Socket();
    /**
     * @brief 打开socket
     * @param localAddr
     * @param localPort
     * @return true
     * @return false
     */
    virtual bool open(const std::string& localAddr = "",
                      uint16 localPort = 0) = 0;
    /**
     *  @brief  关闭socket
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool close();
    /**
     * @brief 读取数据
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
     *  @brief  socket查看缓冲区数据
     *  @param  buf 接收缓存
     *  @param  len 缓存大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回查看的数据长度,失败返回STATUS_ERROR
     */
    virtual int peekData(char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  socket接收数据
     *  @param  buf 接收缓存
     *  @param  len 缓存大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回接收的数据长度,失败返回STATUS_ERROR
     */
    virtual int recvData(char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  socket发送数据
     *  @param  data 发送数据缓存
     *  @param  len 缓存大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回发送的数据长度,失败返回STATUS_ERROR
     */
    virtual int sendData(const char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  设置socket选项
     *  @param  socketType SOCKET_TYPE_E
     *  @return 成功返回STATUS_OK,失败返回STATUS_ERROR
     */
    virtual int setOption(int socketOpt, int value);
    /**
     *  @brief  获取socket选项
     *  @param  socketType SOCKET_TYPE_E
     *  @return 成功返回STATUS_OK,失败返回STATUS_ERROR
     */
    virtual int getOption(int socketOpt);
    /**
     *  @brief  设置连接(设置对端地址和端口)
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool setConnection(const std::string& peerAddr, uint16 peerPort);
    /**
     *  @brief  获取连接(获取对端地址和端口)
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool getConnection(std::string* peerAddr, uint16* peerPort);
    /**
     * @brief 判断端口是否活动
     * @return true
     * @return false
     */
    bool isAlive();
    /**
     * @brief 获取端口fd
     * @return int
     */
    int fd();
    /**
     * @brief 通过fd打开端口
     * @param fd
     * @return true
     * @return false
     */
    bool fdopen(int fd);

protected:
    /**
     *  @brief  绑定socket到指定IP地址的端口上
     *  @param  localPort 要绑定的端口
     *  @param  localAddr 绑定的IP地址,不指定则为默认本机任意地址
     *  @return 成功返回true,失败返回false
     */
    bool bindPort(uint16 localPort, std::string localAddr = "");

protected:
    std::string m_peerAddr{""};
    uint16 m_peerPort{0};
    int m_sockfd{-1};
    struct sockaddr_in m_localSockAddr;
};

/**
 *  @class  UdpSocket
 *  @brief  UdpSocket类
 */
class UdpSocket : public Socket {
    DECL_CLASSNAME(UdpSocket)

public:
    UdpSocket();
    virtual ~UdpSocket();
    /**
     *  @brief  打开UDP socket
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    bool open(const std::string& localAddr = "", uint16 localPort = 0);
    /**
     *  @brief  关闭UDP socket
     *  @param  void
     *  @return 成功返回true,失败返回false
     *  @note   none
     */
    bool close();
    /**
     * @brief 读数据
     * @param buf
     * @param len
     * @return int
     */
    int readData(char* buf, int len);
    /**
     * @brief 写数据
     * @param buf
     * @param len
     * @return int
     */
    int writeData(const char* buf, int len);
    /**
     *  @brief  UDP socket接收数据
     *  @param  buf 接收缓存
     *  @param  len 缓存大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回接收的数据长度,失败返回STATUS_ERROR
     */
    int recvData(char* buf, int len, int usTimeout = -1);
    /**
     *  @brief  UDP socket发送数据
     *  @param  data 发送数据缓存
     *  @param  len 缓存大小
     *  @param  usTimeout 超时时间,-1:阻塞,0:立即返回,>0:超时时间us
     *  @return 成功返回发送的数据长度,失败返回STATUS_ERROR
     */
    int sendData(const char* buf, int len, int usTimeout = -1);
};

/**
 *  @class  TcpSocket
 *  @brief  TcpSocket类
 */
class TcpSocket : public Socket {
    DECL_CLASSNAME(TcpSocket)

public:
    TcpSocket();
    virtual ~TcpSocket();
    /**
     *  @brief  打开TCP socket
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool open(const std::string& localAddr = "", uint16 localPort = 0);
    /**
     *  @brief  关闭TCP socket
     *  @param  void
     *  @return 成功返回true,失败返回false
     */
    virtual bool close();
    /**
     *  @brief  TCP socket监听连接
     *  @param  maxpend 最大挂起连接数
     *  @return 成功返回true,失败返回false
     */
    virtual bool listenConnection(int maxpend);
    /**
     *  @brief  TCP socket接受连接
     *  @param  newsocket 接受的新连接
     *  @return 成功返回true,失败返回false
     */
    virtual bool acceptConnection(TcpSocket* newSocket);
    /**
     *  @brief  TCP socket连接远端
     *  @param  port 对端端口
     *  @param  ipaddr 对端IP地址
     *  @return 成功返回接收的数据长度,失败返回STATUS_ERROR
     */
    virtual bool setConnection(const std::string& peerAddr, uint16 peerPort);
};

/**
 *  @class  LocalTcpSocket
 *  @brief  LocalTcpSocket类
 *  @note 	LocalSocket在Android中Native端使用时需分配权限,这样
 *          App才可以使用该socket.
 *          init.rc中增加：
 *          service socket_test /system/native/socket_test
 *          class main
 *          socket_name steam 600 system system
 */
class LocalTcpSocket : public TcpSocket {
    DECL_CLASSNAME(LocalTcpSocket)

public:
    LocalTcpSocket();
    virtual ~LocalTcpSocket();
    /**
     * @brief 打开本地socket
     * @param localSocketName
     * @param localPort
     * @return true
     * @return false
     */
    bool open(const std::string& localSocketName = "", uint16 localPort = 0);
    /**
     * @brief 接受连接
     * @param newSocket
     * @return true
     * @return false
     */
    bool acceptConnection(TcpSocket* newSocket);
    /**
     * @brief 连接本地socket
     * @param localName 本地socket名称
     * @param peerPort  设置为0时,代表使用无名socket,其他值将连接有名socket
     * @return 成功返回true,失败返回false
     */
    bool setConnection(const std::string& peerAddr, uint16 peerPort);

private:
    std::string m_socketName{""};
    std::string m_peerName{""};
};

/**
 *  @class  UdpServer
 *  @brief  UDP服务器
 */
class UdpServer : public Runnable {
    DECL_CLASSNAME(UdpServer)

public:
    UdpServer();
    virtual ~UdpServer();
    /**
     * @brief 配置服务
     * @param serverIP
     * @param serverPort
     * @return true
     * @return false
     */
    bool setup(std::string serverIP, uint16 serverPort);
    /**
     * @brief 接收数据报回调方法(子类必须实现)
     * @param udpSocket
     * @param data
     * @param len
     * @return true 继续处理数据报
     * @return false 退出服务
     */
    virtual bool onNewDatagram(const UdpSocket& udpSocket, char* data,
                               int len) = 0;

private:
    void run(const Thread& thread);

private:
    UdpSocket m_serverSocket;
    bool m_stopFlag{false};
};

/**
 *  @class  TcpServer
 *  @brief  Tcp服务器
 */
class TcpServer : public Runnable {
    DECL_CLASSNAME(TcpServer)

public:
    TcpServer();
    virtual ~TcpServer();
    /**
     * @brief 服务配置
     * @param serverIP
     * @param serverPort
     * @param maxPendings
     * @return true
     * @return false
     */
    virtual bool setup(std::string serverIP, uint16 serverPort,
                       int maxPendings = 1);
    /**
     * @brief TCP连接回调方法
     * @param client
     * @return true 继续处理流
     * @return false 退出服务
     */
    virtual bool onNewConnection(std::unique_ptr<TcpSocket> client) = 0;

private:
    void run(const Thread& thread);

protected:
    bool m_stopFlag{false};
    std::unique_ptr<TcpSocket> m_serverSocket{nullptr};
};

/**
 *  @class  LocalTcpServer
 *  @brief  本地TCP服务器
 */
class LocalTcpServer : public TcpServer {
    DECL_CLASSNAME(LocalTcpServer)
public:
    LocalTcpServer();
    virtual ~LocalTcpServer();
    /**
     * @brief 本地TCP服务配置
     * @param serverName
     * @param serverPort
     * @param maxPendings
     * @return true
     * @return false
     */
    bool setup(std::string serverName, uint16 serverPort, int maxPendings = 1);

private:
    void run(const Thread& thread);
};

/**
 * @class SocketPair
 * @brief 套接字对类
 */
class SocketPair {
    DECL_CLASSNAME(SocketPair)

public:
    /**
     * @enum PAIRFD_TYPE_E
     * @brief SocketPair描述符类型
     */
    enum {
        PAIRFD_READ = 0,
        PAIRFD_WRITE = 1,
    } PAIRFD_TYPE_E;

public:
    SocketPair();
    ~SocketPair();
    /**
     * @brief 创建套接字对
     * @return true
     * @return false
     */
    bool createPair();
    /**
     * @brief 获取套接字描述符
     * @param pairType
     * @return int
     */
    int fd(int pairType);
    /**
     * @brief 接收数据
     * @param buf
     * @param len
     * @param usTimeout
     * @return int
     */
    int recvData(char* buf, int len, int usTimeout = -1);
    /**
     * @brief 发送数据
     * @param buf
     * @param len
     * @param usTimeout
     * @return int
     */
    int sendData(const char* buf, int len, int usTimeout = -1);

private:
    int m_fd[2]{-1, -1};
};

/**
 * @enum  CAN_FF_E
 * @brief CAN帧格式
 */
enum CAN_FF_E {
    CAN_FF_STD = 0, /**< 标准帧 */
    CAN_FF_EXT      /**< 扩展帧 */
};

/**
 * @enum  CAN_FF_E
 * @brief CAN帧类型
 */
enum CAN_FT_E {
    CAN_FT_DATA = 0, /**< 数据帧 */
    CAN_FT_REMOTE    /**< 远程帧 */
};

/**
 * @enum  CAN_FE_E
 * @brief CAN帧错误
 */
enum CAN_FE_E {
    CAN_FE_NONE = 0, /**< 无错误 */
    CAN_FE_ERR       /**< 错误消息 */
};

class SocketCAN;

/**
 * @class CANFrame
 * @brief CAN帧
 */
class CANFrame {
public:
    CANFrame();
    /**
     * @brief CAN帧构造函数
     * @param canid 帧id(包括标志位)
     * @param data 数据
     * @param len 数据长度
     */
    CANFrame(uint32 canid, char* data, uint8 len);
    /**
     * @brief CAN帧构造函数
     * @param id 帧id(不包括标志位)
     * @param data 数据
     * @param len 数据长度
     * @param format 帧格式
     */
    CANFrame(uint32 id, char* data, uint8 len, uint8 format);
    ~CANFrame();
    /**
     * @brief 获取帧格式
     * @return uint8
     */
    uint8 format();
    /**
     * @brief 获取帧类型
     * @return uint8
     */
    uint8 type();
    /**
     * @brief 获取帧错误
     * @return uint8
     */
    uint8 error();
    /**
     * @brief 获取帧id(不包括标志位)
     * @return uint32
     */
    uint32 id();
    /**
     * @brief 获取帧id(包括标志位)
     * @return uint32
     */
    uint32 canid();
    /**
     * @brief 获取帧数据长度
     * @return uint8
     */
    uint8 dataLen();
    /**
     * @brief 获取帧数据
     * @return char*
     */
    char* data();

private:
    friend class SocketCAN;
    struct can_frame m_frame;
};

/**
 * @class SocketCAN
 * @brief CAN套接字
 * @note 仅实现数据收发功能,对于CAN接口的配置请使用libsocketcan开源库
 */
class SocketCAN : public Socket {
    DECL_CLASSNAME(SocketCAN)

public:
    SocketCAN();
    virtual ~SocketCAN();
    /**
     * @brief 获取接口名称
     * @return std::string
     */
    std::string interface();
    /**
     * @brief 打开套接字
     * @param canName
     * @param localPort
     * @return true
     * @return false
     */
    bool open(const std::string& canName = "", uint16 localPort = 0);
    /**
     * @brief 关闭套接字
     * @return true
     * @return false
     */
    bool close();
    /**
     * @brief 接收帧
     * @param canFrame
     * @param usTimeout
     * @return int
     */
    int recvFrame(CANFrame* canFrame, int usTimeout = -1);
    /**
     * @brief 发送帧
     * @param canFrame
     * @param usTimeout
     * @return int
     */
    int sendFrame(const CANFrame& canFrame, int usTimeout = -1);
    /**
     * @brief 读取帧
     * @param canFrame
     * @return int
     */
    int readFrame(CANFrame* canFrame);
    /**
     * @brief 写入帧
     * @param canFrame
     * @return int
     */
    int writeFrame(const CANFrame& canFrame);

private:
    int recvData(char* buf, int len, int usTimeout = -1);
    int sendData(const char* data, int len, int usTimeout = -1);
    int peekData(char* buf, int len, int usTimeout) { return RC_ERROR; }
    bool setConnection(const std::string& peerAddr, uint16 peerPort) {
        return false;
    }
    int setOption(int socketOpt, int value) { return RC_ERROR; }
    int getOption(int socketOpt) { return RC_ERROR; }

private:
    std::string m_interface{""};
};

}  // namespace zemb
#endif
