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
#include "Socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "StrUtil.h"
#include "Tracer.h"

namespace zemb {
Socket::Socket() {}
Socket::~Socket() { this->close(); }

bool Socket::bindPort(uint16 localPort, std::string localAddr) {
    int rc = -1;
    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket is not open");
        return false;
    }

    /*为TCP链接设置IP和端口等信息*/
    bzero(&m_localSockAddr, sizeof(m_localSockAddr));
    m_localSockAddr.sin_family = AF_INET;
    m_localSockAddr.sin_port = htons(localPort);

    /* 如果本机ip地址没指定,则设置本机任意地址 */
    if (localAddr.empty()) {
        m_localSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        m_localSockAddr.sin_addr.s_addr = inet_addr(CSTR(localAddr));
    }
    /*地址与socket绑定bind*/
    rc = ::bind(m_sockfd, (struct sockaddr*)&m_localSockAddr,
                sizeof(m_localSockAddr));
    if (rc < 0) {
        TRACE_ERR_CLASS("bind(%s:%d) error:%s.",
                        inet_ntoa(m_localSockAddr.sin_addr), localPort, ERRSTR);
        return false;
    }
    if (localPort == 0) {
        struct sockaddr_in localaddr;
        socklen_t addrLen = sizeof(struct sockaddr_in);
        getsockname(m_sockfd, (struct sockaddr*)&localaddr, &addrLen);
        m_localSockAddr.sin_port = localaddr.sin_port;
        localPort = ntohs(localaddr.sin_port);
    }
    TRACE_REL_CLASS("bind ok %s:%d", inet_ntoa(m_localSockAddr.sin_addr),
                    localPort);
    return true;
}

bool Socket::close() {
    if (m_sockfd >= 0) {
        ::close(m_sockfd);
    }
    // TRACE_YELLOW("socket close(%d)",m_sockfd);
    m_sockfd = -1;
    return true;
}

int Socket::readData(char* buf, int len) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    rc = ::read(m_sockfd, buf, len);
    if (rc < 0) {
        if (rc == -1 && errno != EAGAIN) {
            TRACE_ERR_CLASS("read error:%s", ERRSTR);
        }
        return RC_ERROR;
    }
    return rc;
}

int Socket::writeData(const char* buf, int len) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }
    int wlen = 0;
    do {
        rc = ::write(m_sockfd, buf + wlen, len - wlen);
        if (rc > 0) {
            wlen += rc;
        } else {
            if (rc == -1 && errno != EAGAIN) {
                TRACE_ERR_CLASS("write error:%s", ERRSTR);
            }
            return RC_ERROR;
        }
    } while (wlen < len);
    return wlen;
}

int Socket::peekData(char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    struct timeval tv, *ptv;
    fd_set readfds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&readfds);
    FD_SET(m_sockfd, &readfds);
    rc = select(m_sockfd + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        // TRACE_INFO_CLASS("recv timeout.");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }
    /* 检查fd是否可读 */
    if (!FD_ISSET(m_sockfd, &readfds)) {
        TRACE_ERR_CLASS("readfds error");
        return RC_ERROR;
    }

    int recvLen = recv(m_sockfd, buf, len, MSG_PEEK);
    if (recvLen < 0) {
        if (errno == ECONNABORTED) {
            TRACE_ERR_CLASS("socket peek error:%s", ERRSTR);
            return 0;
        } else if (errno != EAGAIN) {
            TRACE_ERR_CLASS("sockfd(%d) peek error:%s", m_sockfd, ERRSTR);
            return RC_ERROR;
        }
    }
    return recvLen;
}

int Socket::recvData(char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    struct timeval tv, *ptv;
    fd_set readfds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&readfds);
    FD_SET(m_sockfd, &readfds);
    rc = select(m_sockfd + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        // TRACE_INFO_CLASS("recv timeout.");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {  // EINTR表示被信号打断
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }
    /* 检查fd是否可读 */
    if (!FD_ISSET(m_sockfd, &readfds)) {
        TRACE_ERR_CLASS("readfds error");
        return RC_ERROR;
    }

    // int recvLen = read(m_sockfd, buf, len);
    int recvLen = recv(m_sockfd, buf, len, 0);
    if (recvLen < 0) {
        if (errno == ECONNABORTED) {
            TRACE_ERR_CLASS("socket read error:%s", ERRSTR);
            return 0;
        } else if (errno != EAGAIN) {
            TRACE_ERR_CLASS("sockfd(%d) read error:%s", m_sockfd, ERRSTR);
            return RC_ERROR;
        }
    }
    return recvLen;
}

int Socket::sendData(const char* data, int len, int usTimeout) {
    int rc;
    if ((nullptr == data) || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    struct timeval tv, *ptv;
    fd_set writefds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&writefds);
    FD_SET(m_sockfd, &writefds);
    rc = select(m_sockfd + 1, nullptr, &writefds, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("send timeout.");
        return RC_TIMEOUT;
    } else if (rc < 0) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }

    /* 检查fd是否可写 */
    if (!FD_ISSET(m_sockfd, &writefds)) {
        TRACE_ERR_CLASS("writefds error");
        return RC_ERROR;
    }

    /* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
    signal(SIGPIPE, SIG_IGN);
    // int sndLen = write(m_sockfd, data, len);
    int sndLen = send(m_sockfd, data, len, 0);
    if (sndLen < 0) {
        TRACE_ERR_CLASS("sockfd(%d) write error:%s", m_sockfd, ERRSTR);
        return RC_ERROR;
    }
    return sndLen;
}

int Socket::setOption(int socketOpt, int value) {
    int level; /* SOL_SOCKET,IPPROTO_TCP,IPPROTO_IP,IPPROTO_IPV6 */
    int option;
    int optionVal;
    int rc;
    switch (socketOpt) {
        case SOCKET_OPTION_CASTTYPE:
            if (value == SOCKET_TYPE_BROADCAST) {
                level = SOL_SOCKET;
                option = SO_BROADCAST;
                optionVal = 1;
            } else if (value == SOCKET_TYPE_MULTICAST) {
            } else if (value == SOCKET_TYPE_GROUPCAST) {
            } else {
                TRACE_ERR_CLASS("Unsupport socket type:%d", value);
                return RC_ERROR;
            }
            break;
        case SOCKET_OPTION_RCVBUF: {
            level = SOL_SOCKET;
            option = SO_RCVBUF;
            optionVal = value;
            break;
        }
        case SOCKET_OPTION_SNDBUF: {
            level = SOL_SOCKET;
            option = SO_SNDBUF;
            optionVal = value;
            break;
        }
        default:
            TRACE_ERR_CLASS("Unsupport socket option:%d", socketOpt);
            return RC_ERROR;
    }
    rc = setsockopt(m_sockfd, level, option, &optionVal, sizeof(optionVal));
    if (rc < 0) {
        return RC_ERROR;
    }
    return RC_OK;
}

int Socket::getOption(int socketOpt) {
    int level; /* SOL_SOCKET,IPPROTO_TCP,IPPROTO_IP,IPPROTO_IPV6 */
    int option;
    int optionVal;
    int optionLen;
    int rc;
    switch (socketOpt) {
        case SOCKET_OPTION_RCVBUF: {
            level = SOL_SOCKET;
            option = SO_RCVBUF;
            break;
        }
        case SOCKET_OPTION_SNDBUF: {
            level = SOL_SOCKET;
            option = SO_SNDBUF;
            break;
        }
        default:
            TRACE_ERR_CLASS("Unsupport socket option:%d", socketOpt);
            return RC_ERROR;
    }
    rc = getsockopt(m_sockfd, level, option, &optionVal,
                    reinterpret_cast<socklen_t*>(&optionLen));
    if (rc < 0) {
        return RC_ERROR;
    }
    return optionVal;
}

bool Socket::setConnection(const std::string& peerAddr, uint16 peerPort) {
    if (!peerAddr.empty()) {
        m_peerAddr = peerAddr;
    }
    m_peerPort = peerPort;
    return true;
}

bool Socket::getConnection(std::string* peerAddr, uint16* peerPort) {
    *peerAddr = m_peerAddr;
    *peerPort = m_peerPort;
    return true;
}

bool Socket::isAlive() { return (m_sockfd < 0) ? false : true; }

int Socket::fd() { return m_sockfd; }

bool Socket::fdopen(int fd) {
    if (m_sockfd > 0) {
        return false;
    } else if (fd > 0) {
        m_sockfd = fd;
        return true;
    }
    return false;
}

UdpSocket::UdpSocket() {}
UdpSocket::~UdpSocket() { this->close(); }

bool UdpSocket::open(const std::string& localAddr, uint16 localPort) {
    if (m_sockfd < 0) {
        m_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0) {
            TRACE_ERR_CLASS("open socket error:%s", ERRSTR);
            return false;
        }
        if (!bindPort(localPort, localAddr)) {
            close();
            return false;
        }
    }
    // int flag = fcntl(m_sockfd, F_GETFL, 0);
    // fcntl(m_sockfd, F_SETFL, flag | O_NONBLOCK);
    return true;
}

bool UdpSocket::close() { return Socket::close(); }

int UdpSocket::readData(char* buf, int len) {
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }
    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }
    struct sockaddr_in cliSockAddr;
    socklen_t addLen = sizeof(cliSockAddr);
    int recvLen = recvfrom(m_sockfd, buf, len, 0,
                           (struct sockaddr*)&cliSockAddr, &addLen);
    if (recvLen < 0) {
        if (errno != EAGAIN) {
            TRACE_ERR_CLASS("sockfd(%d) read error:%s", m_sockfd, ERRSTR);
            return RC_ERROR;
        }
    }
    if (recvLen > 0) {
        m_peerAddr = inet_ntoa(cliSockAddr.sin_addr);
        m_peerPort = ntohs(cliSockAddr.sin_port);
    }
    return recvLen;
}
int UdpSocket::writeData(const char* data, int len) {
    if (m_peerAddr == "" || m_peerPort == 0) {
        TRACE_ERR_CLASS("peer not set, peerAddr:%s,peerPort:%d.",
                        CSTR(m_peerAddr), m_peerPort);
        return RC_ERROR;
    }
    if (nullptr == data || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    /* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
    signal(SIGPIPE, SIG_IGN);

    struct sockaddr_in peerSockAddr;
    peerSockAddr.sin_family = AF_INET;
    peerSockAddr.sin_port = htons(m_peerPort);
    peerSockAddr.sin_addr.s_addr = inet_addr(CSTR(m_peerAddr));
    int sendLen = sendto(m_sockfd, data, len, 0,
                         (struct sockaddr*)&peerSockAddr, sizeof(peerSockAddr));
    if (sendLen < 0) {
        TRACE_ERR_CLASS("sockfd(%d) write error:%s", m_sockfd, ERRSTR);
        return RC_ERROR;
    }
    return sendLen;
}

int UdpSocket::recvData(char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }
    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }
#if 1
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_sockfd, &readfds);

    struct timeval tv, *ptv;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
        ptv = &tv;
    }

    rc = select(m_sockfd + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("recv timeout");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {  // EINTR表示被信号打断
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }
    /* 检查fd是否可读 */
    if (!FD_ISSET(m_sockfd, &readfds)) {
        TRACE_ERR_CLASS("readfds error.");
        return RC_ERROR;
    }
#endif

    struct sockaddr_in cliSockAddr;
    socklen_t addLen = sizeof(cliSockAddr);
    int recvLen = recvfrom(m_sockfd, buf, len, 0,
                           (struct sockaddr*)&cliSockAddr, &addLen);
    if (recvLen < 0) {
        if (errno != EAGAIN) {
            TRACE_ERR_CLASS("sockfd(%d) read error:%s", m_sockfd, ERRSTR);
            return RC_ERROR;
        }
    }
    if (recvLen > 0) {
        m_peerAddr = inet_ntoa(cliSockAddr.sin_addr);
        m_peerPort = ntohs(cliSockAddr.sin_port);
        // TRACE_DBG_CLASS("UDP
        // recvfrom(%s:%d),len:%d",CSTR(m_peerAddr),m_peerPort,recvLen);
    }
    return recvLen;
}

int UdpSocket::sendData(const char* data, int len, int usTimeout) {
    int rc;
    if (m_peerAddr == "" || m_peerPort == 0) {
        TRACE_ERR_CLASS("peer not set, peerAddr:%s,peerPort:%d.",
                        CSTR(m_peerAddr), m_peerPort);
        return RC_ERROR;
    }
    if (nullptr == data || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(m_sockfd, &writefds);

    struct timeval tv, *ptv;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
        ptv = &tv;
    }

    rc = select(m_sockfd + 1, nullptr, &writefds, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("send timeout");
        return RC_TIMEOUT;
    } else if (rc < 0) {
        TRACE_ERR_CLASS("select error:%s.", ERRSTR);
        return RC_ERROR;
    }

    /* 检查fd是否可写 */
    if (!FD_ISSET(m_sockfd, &writefds)) {
        TRACE_ERR_CLASS("writefds error.");
        return RC_ERROR;
    }

    /* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
    signal(SIGPIPE, SIG_IGN);

    struct sockaddr_in peerSockAddr;
    peerSockAddr.sin_family = AF_INET;
    peerSockAddr.sin_port = htons(m_peerPort);
    peerSockAddr.sin_addr.s_addr = inet_addr(CSTR(m_peerAddr));

    // TRACE_INFO_CLASS("UDP sendto(%s:%d),data
    // len:%d",CSTR(m_peerAddr),m_peerPort,len);
    int sendLen = sendto(m_sockfd, data, len, 0,
                         (struct sockaddr*)&peerSockAddr, sizeof(peerSockAddr));
    if (sendLen < 0) {
        TRACE_ERR_CLASS("sockfd(%d) write error:%s", m_sockfd, ERRSTR);
        return RC_ERROR;
    }

    return sendLen;
}

TcpSocket::TcpSocket() {}
TcpSocket::~TcpSocket() { this->close(); }

bool TcpSocket::open(const std::string& localAddr, uint16 localPort) {
    if (m_sockfd < 0) {
        m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd < 0) {
            TRACE_ERR_CLASS("open socket error:%s", ERRSTR);
            return false;
        }
        int on = 1;
        if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
            0) {
            TRACE_ERR_CLASS("setsockopt error:%s", ERRSTR);
            close();
            return false;
        }

        if (!bindPort(localPort, localAddr)) {
            close();
            return false;
        }
    }
    return true;
}

bool TcpSocket::close() { return Socket::close(); }

bool TcpSocket::listenConnection(int maxpending) {
    int rc;
    rc = ::listen(m_sockfd, maxpending);
    if (rc < 0) {
        TRACE_ERR_CLASS("listen error:%s.", ERRSTR);
        return false;
    }
    return true;
}

bool TcpSocket::acceptConnection(TcpSocket* newsocket) {
    int rc;
    struct sockaddr_in client_sockaddr;
    int addr_size = sizeof(struct sockaddr_in);
    rc = ::accept(m_sockfd, (struct sockaddr*)&client_sockaddr,
                  reinterpret_cast<socklen_t*>(&addr_size));
    if (rc < 0) {
        TRACE_ERR_CLASS("accept error:%s", ERRSTR);
        Thread::msleep(3000);
        return false;
    }
    newsocket->fdopen(rc);
    TRACE_REL_CLASS("tcp connection from %s:%d",
                    inet_ntoa(client_sockaddr.sin_addr),
                    ntohs(client_sockaddr.sin_port));
    return true;
}

bool TcpSocket::setConnection(const std::string& peerAddr, uint16 peerPort) {
    int rc;
    struct sockaddr_in peerSockAddr;
    bzero(&peerSockAddr, sizeof(peerSockAddr));
    peerSockAddr.sin_family = AF_INET;
    peerSockAddr.sin_port = htons(peerPort);
    if (peerAddr.empty()) {
        TRACE_ERR_CLASS("peer ip addr not set");
        return false;
    } else {
        peerSockAddr.sin_addr.s_addr = inet_addr(CSTR(peerAddr));
    }

    /* 开始连接服务器connect */
    TRACE_REL_CLASS("connect to %s:%d", inet_ntoa(peerSockAddr.sin_addr),
                    ntohs(peerSockAddr.sin_port));
    int constatus = ::connect(m_sockfd, (struct sockaddr*)&peerSockAddr,
                              sizeof(peerSockAddr));
    if (0 == constatus) {  // 连接正常
        struct timeval tv;
        fd_set writefds;

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_ZERO(&writefds);
        FD_SET(m_sockfd, &writefds);

        rc = select(m_sockfd + 1, nullptr, &writefds, nullptr, &tv);
        if (0 == rc) {
            // TRACE_ERR_CLASS("select timeout.");
            return false;
        } else if (rc < 0) {
            TRACE_ERR_CLASS("select error:%s.", ERRSTR);
            return false;
        }

        if (!FD_ISSET(m_sockfd, &writefds)) {
            TRACE_ERR_CLASS("socket fd error.");
            return false;
        }
        // TRACE_REL_CLASS("=========TcpSocket connect success.");
        return true;
    } else {
        TRACE_ERR_CLASS("connect error:%s.", ERRSTR);
        if (errno != EINPROGRESS) {
            // TRACE_ERR_CLASS("connect error:%s",ERRSTR);
        }
    }
    return false;
}

LocalTcpSocket::LocalTcpSocket() {}

LocalTcpSocket::~LocalTcpSocket() { this->close(); }

bool LocalTcpSocket::open(const std::string& localSocketName,
                          uint16 localPort) {
    struct sockaddr_un sockAddr;
    if (m_sockfd < 0) {
        m_sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
        if (m_sockfd < 0) {
            TRACE_ERR_CLASS("open socket error:%s", ERRSTR);
            return false;
        }

        bzero(&sockAddr, sizeof(sockAddr));
        sockAddr.sun_family = AF_LOCAL;
        if (localPort == 0) {
            /* 无名socket,使用系统全局名称,无名的sun_path[0]必须为0 */
            sockAddr.sun_path[0] = 0;
            m_socketName = localSocketName;
            snprintf(sockAddr.sun_path + 1, m_socketName.size(), "%s",
                     CSTR(m_socketName));

        } else {
            /* 有名socket,绑定固定的路径 */
            m_socketName = localSocketName + std::string("/") +
                           StrUtil::format("%d", localPort);
            snprintf(sockAddr.sun_path, m_socketName.size(), "%s",
                     CSTR(m_socketName));
        }

        unlink(CSTR(m_socketName));

        /* 地址与socket绑定bind */
        if (::bind(m_sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) <
            0) {
            TRACE_ERR_CLASS("bind error:%s.", ERRSTR);
            close();
            return false;
        }
    }
    return true;
}

bool LocalTcpSocket::setConnection(const std::string& peerAddr,
                                   uint16 peerPort) {
    int rc;
    struct sockaddr_un sockAddr;
    bzero(&sockAddr, sizeof(sockAddr));
    sockAddr.sun_family = AF_LOCAL;
    if (peerPort == 0) {
        /* 无名socket,使用系统全局名称,无名的sun_path[0]必须为0 */
        sockAddr.sun_path[0] = 0;
        m_peerName = peerAddr;
        snprintf(sockAddr.sun_path + 1, m_peerName.size(), "%s",
                 CSTR(m_peerName));

    } else {
        /* 有名socket,绑定固定的路径 */
        m_peerName =
            peerAddr + std::string("/") + StrUtil::format("%d", peerPort);
        snprintf(sockAddr.sun_path, m_peerName.size(), "%s", CSTR(m_peerName));
    }

    unlink(CSTR(m_peerName));

    /*开始连接服务器connect*/
    TRACE_REL_CLASS("connect to %s", CSTR(m_peerName));
    int constatus =
        ::connect(m_sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
    if (0 == constatus) {  // 连接正常
        struct timeval tv;
        fd_set writefds;

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_ZERO(&writefds);
        FD_SET(m_sockfd, &writefds);

        rc = select(m_sockfd + 1, nullptr, &writefds, nullptr, &tv);
        if (0 == rc) {
            // TRACE_ERR_CLASS("select timeout.");
            return false;
        } else if (rc < 0) {
            TRACE_ERR_CLASS("select error:%s.", ERRSTR);
            return false;
        }

        if (!FD_ISSET(m_sockfd, &writefds)) {
            TRACE_ERR_CLASS("socket fd error.");
            return false;
        }
        TRACE_REL_CLASS("Local Socket connect success.");
        return true;
    } else {
        TRACE_ERR_CLASS("connect error:%s.", ERRSTR);
        if (errno != EINPROGRESS) {
            TRACE_ERR_CLASS("connect error.");
        }
    }
    return false;
}

bool LocalTcpSocket::acceptConnection(TcpSocket* newsocket) {
    int rc;
    struct sockaddr_un client_sockaddr;
    int addr_size = sizeof(client_sockaddr);
    rc = ::accept(m_sockfd, (struct sockaddr*)&client_sockaddr,
                  reinterpret_cast<socklen_t*>(&addr_size));
    if (rc < 0) {
        TRACE_ERR_CLASS("accept error:%s", ERRSTR);
        Thread::msleep(3000);
        return false;
    }
    newsocket->fdopen(rc);
    return true;
}

UdpServer::UdpServer() {}

UdpServer::~UdpServer() { m_serverSocket.close(); }

bool UdpServer::setup(std::string serverIP, uint16 serverPort) {
    if (!m_serverSocket.open(CSTR(serverIP), serverPort)) {
        return false;
    }
    return true;
}

void UdpServer::run(const Thread& thread) {
    int len = UDP_PACKET_LEN_MAX;
    auto bufPtr = std::make_unique<char[]>(len);
    char* buf = bufPtr.get();
    bool run = true;
    while (run) {
        int rc = m_serverSocket.recvData(buf, len, 10000);
        if (rc > 0) {
            run = onNewDatagram(m_serverSocket, buf, rc);
        }
    }
}

TcpServer::TcpServer() {}

TcpServer::~TcpServer() { m_serverSocket->close(); }

bool TcpServer::setup(std::string serverIP, uint16 serverPort,
                      int maxPendings) {
    m_serverSocket = std::make_unique<TcpSocket>();
    if (!m_serverSocket) {
        return false;
    }
    if (!m_serverSocket->open(CSTR(serverIP), serverPort)) {
        return false;
    }

    if (!m_serverSocket->listenConnection(maxPendings)) {
        return false;
    }
    return true;
}

void TcpServer::run(const Thread& thread) {
    bool run = true;
    while (run) {
        auto client = std::make_unique<TcpSocket>();
        if (!m_serverSocket->acceptConnection(client.get())) {
            continue;
        } else {
            TRACE_REL_CLASS("new tcp client connected.");
            run = onNewConnection(std::move(client));
        }
    }
}

LocalTcpServer::LocalTcpServer() {}
LocalTcpServer::~LocalTcpServer() {}

bool LocalTcpServer::setup(std::string serverName, uint16 serverPort,
                           int maxPendings) {
    m_serverSocket = std::make_unique<LocalTcpSocket>();
    if (!m_serverSocket) {
        return false;
    }
    if (!m_serverSocket->open(CSTR(serverName), serverPort)) {
        return false;
    }

    if (!m_serverSocket->listenConnection(maxPendings)) {
        return false;
    }

    return true;
}

void LocalTcpServer::run(const Thread& thread) {
    m_stopFlag = false;
    while (!m_stopFlag) {
        auto client = std::make_unique<LocalTcpSocket>();
        if (!m_serverSocket->acceptConnection(client.get())) {
            continue;
        } else {
            TRACE_REL_CLASS("new local tcp client connected.");
            onNewConnection(std::move(client));
        }
    }
}

SocketPair::SocketPair() {}
SocketPair::~SocketPair() {}

bool SocketPair::createPair() {
    /* fd[0]负责读,fd[1]负责写 */
    int rc = socketpair(AF_UNIX, SOCK_DGRAM, 0, m_fd);
    fcntl(m_fd[1], F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        TRACE_ERR_CLASS("create socket pair failed.");
        return false;
    }
    return true;
}

int SocketPair::fd(int pairType) {
    switch (pairType) {
        case PAIRFD_READ:
            return m_fd[0];
        case PAIRFD_WRITE:
            return m_fd[1];
        default:
            break;
    }
    return RC_ERROR;
}

int SocketPair::recvData(char* buf, int len, int usTimeout) {
    if (m_fd[0] < 0) {
        TRACE_ERR_CLASS("read fd: %d.", m_fd[0]);
        return RC_ERROR;
    }
    struct timeval tv, *ptv;
    fd_set readfds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&readfds);
    FD_SET(m_fd[0], &readfds);
    int rc = select(m_fd[0] + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        TRACE_ERR_CLASS("select read timeout.");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {
        TRACE_ERR_CLASS("select error: %s", ERRSTR);
        return RC_ERROR;
    }
    if (!FD_ISSET(m_fd[0], &readfds)) {
        TRACE_ERR_CLASS("readfds error");
        return RC_ERROR;
    }

    rc = read(m_fd[0], buf, len);
    if (rc < 0) {
        TRACE_ERR_CLASS("read msg error");
        return RC_ERROR;
    }
    return rc;
}

int SocketPair::sendData(const char* buf, int len, int usTimeout) {
    int rc;
#if 0
    if (m_fd[1] < 0) {
        TRACE_ERR_CLASS("Invalid write fd.");
        return RC_ERROR;
    }
    struct timeval tv, *ptv;
    fd_set writefds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&writefds);
    FD_SET(m_fd[1], &writefds);
    rc = select(m_fd[PAIRFD_WRITE] + 1, &writefds, nullptr, nullptr, ptv);
    if (0 == rc) {
        TRACE_ERR_CLASS("select write timeout.");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {
        TRACE_ERR_CLASS("select error: %s", ERRSTR);
        return RC_ERROR;
    }
    if (!FD_ISSET(m_fd[1], &writefds)) {
        TRACE_ERR_CLASS("writefds error");
        return RC_ERROR;
    }
#endif
    /* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
    // signal(SIGPIPE, SIG_IGN);
    rc = write(m_fd[1], buf, len);
    if (rc < 0) {
        TRACE_ERR_CLASS("socket pair write failed.");
        return RC_ERROR;
    }
    return rc;
}

/******************************************************************************
 * CAN收发数据帧定义
 * struct can_frame {
 * canid_t can_id;  	// 32 bit CAN_ID + EFF/RTR/ERR flags
 * __u8    can_dlc; 	// frame payload length in byte (0 .. CAN_MAX_DLEN)
 * __u8    data[CAN_MAX_DLEN] __attribute__((aligned(8)));
 * };
 * can_id是一个32位整数,位定义如下:
 * bit 0-28 : CAN identifier (11/29 bit)
 * bit 29   : error message frame flag (0 = data frame, 1 = error message)
 * bit 30   : remote transmission request flag (1 = rtr frame)
 * bit 31   : frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 * 帧处理时要用到的掩码:
 * // special address description flags for the CAN_ID
 * #define CAN_EFF_FLAG 0x80000000U // EFF/SFF is set in the MSB
 * #define CAN_RTR_FLAG 0x40000000U // remote transmission request
 * #define CAN_ERR_FLAG 0x20000000U // error message frame
 * // valid bits in CAN ID for frame formats
 * #define CAN_SFF_MASK 0x000007FFU // standard frame format (SFF)
 * #define CAN_EFF_MASK 0x1FFFFFFFU // extended frame format (EFF)
 * #define CAN_ERR_MASK 0x1FFFFFFFU // omit EFF, RTR, ERR flags
 *****************************************************************************/
CANFrame::CANFrame() {}

CANFrame::CANFrame(uint32 canid, char* data, uint8 len) {
    m_frame.can_id = canid;
    if (len == 0 || len > CAN_MAX_DLEN) {
        m_frame.can_dlc = 0;
    } else {
        m_frame.can_dlc = len;
        memcpy(m_frame.data, data, len);
    }
}

CANFrame::CANFrame(uint32 id, char* data, uint8 len, uint8 format) {
    m_frame.can_id = id;
    if (format == CAN_FF_EXT) {
        m_frame.can_id |= CAN_EFF_FLAG;
    }
    if (len == 0 || len > CAN_MAX_DLEN) {
        m_frame.can_dlc = 0;
    } else {
        m_frame.can_dlc = len;
        memcpy(m_frame.data, data, len);
    }
}

CANFrame::~CANFrame() {}
uint8 CANFrame::format() {
    uint8 format = m_frame.can_id & CAN_EFF_FLAG;
    return format;
}
uint8 CANFrame::type() {
    uint8 type = m_frame.can_id & CAN_RTR_FLAG;
    return type;
}
uint8 CANFrame::error() {
    uint8 error = m_frame.can_id & CAN_ERR_FLAG;
    return error;
}
uint32 CANFrame::id() {
    if (format() == CAN_FF_EXT) {
        return (m_frame.can_id & CAN_EFF_MASK);
    } else {
        return (m_frame.can_id & CAN_SFF_MASK);
    }
}

uint32 CANFrame::canid() { return m_frame.can_id; }

uint8 CANFrame::dataLen() { return m_frame.can_dlc; }
char* CANFrame::data() { return reinterpret_cast<char*>(m_frame.data); }

SocketCAN::SocketCAN() {}
SocketCAN::~SocketCAN() {}

std::string SocketCAN::interface() { return m_interface; }

bool SocketCAN::open(const std::string& canName, uint16 localPort) {
    UNUSED_PARAM(localPort);
    if (m_sockfd < 0) {
        int family = PF_CAN;
        int type = SOCK_RAW;
        int proto = CAN_RAW;
        struct ifreq ifr;
        struct sockaddr_can addr;

        m_sockfd = socket(family, type, proto);
        if (m_sockfd < 0) {
            TRACE_ERR_CLASS("open socket error:%s", ERRSTR);
            return false;
        }

        addr.can_family = family;
        snprintf(ifr.ifr_name, canName.size(), "%s", CSTR(canName));
        if (ioctl(m_sockfd, SIOCGIFINDEX, &ifr) != 0) {
            close();
            return false;
        }
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close();
            return false;
        }
    }
    m_interface = canName;
    return true;
}
bool SocketCAN::close() { return Socket::close(); }

int SocketCAN::recvFrame(CANFrame* canFrame, int usTimeout) {
    int rc = recvData(reinterpret_cast<char*>(&(canFrame->m_frame)),
                      sizeof(struct can_frame), usTimeout);
    if (rc != sizeof(struct can_frame)) {
        switch (rc) {
            case RC_ERROR:
            case RC_TIMEOUT:
                return rc;
            default:
                return RC_ERROR;
        }
    }
    return RC_OK;
}

int SocketCAN::sendFrame(const CANFrame& canFrame, int usTimeout) {
    int rc = sendData(
        reinterpret_cast<char*>(const_cast<can_frame*>(&canFrame.m_frame)),
        sizeof(struct can_frame), usTimeout);
    // uint8 errcode = canFrame.error();
    // TRACE_INFO_CLASS("CAN Frame erorr: %02x!",errcode);
    if (rc != sizeof(struct can_frame)) {
        switch (rc) {
            case RC_ERROR:
            case RC_TIMEOUT:
                return rc;
            default:
                return RC_ERROR;
        }
    }
    return RC_OK;
}

int SocketCAN::recvData(char* buf, int len, int usTimeout) {
    int rc;
    if (nullptr == buf || len < sizeof(struct can_frame)) {
        TRACE_ERR_CLASS("param error,len:%d,min:%d.", len,
                        sizeof(struct can_frame));
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    struct timeval tv, *ptv;
    fd_set readfds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&readfds);
    FD_SET(m_sockfd, &readfds);
    rc = select(m_sockfd + 1, &readfds, nullptr, nullptr, ptv);
    if (0 == rc) {
        // TRACE_INFO_CLASS("recv timeout.");
        return RC_TIMEOUT;
    } else if ((rc < 0) && (errno != EINTR)) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }
    /* 检查fd是否可读 */
    if (!FD_ISSET(m_sockfd, &readfds)) {
        TRACE_ERR_CLASS("readfds error");
        return RC_ERROR;
    }

    /* 每次只能读一帧数据 */
    int recvLen = read(m_sockfd, buf, sizeof(struct can_frame));
    if (recvLen < 0) {
        if (errno == ECONNABORTED) {
            TRACE_ERR_CLASS("socket read error:%s", ERRSTR);
            return 0;
        } else if (errno != EAGAIN) {
            TRACE_ERR_CLASS("sockfd(%d) read error:%s", m_sockfd, ERRSTR);
            return RC_ERROR;
        }
    }
    return recvLen;
}

int SocketCAN::sendData(const char* data, int len, int usTimeout) {
    int rc;
    if ((nullptr == data) || len <= 0) {
        TRACE_ERR_CLASS("param error.");
        return RC_ERROR;
    }

    if (m_sockfd < 0) {
        TRACE_ERR_CLASS("socket not open.");
        return RC_ERROR;
    }

    struct timeval tv, *ptv;
    fd_set writefds;
    if (usTimeout < 0) {
        ptv = nullptr;
    } else {
        ptv = &tv;
        tv.tv_sec = usTimeout / 1000000;
        tv.tv_usec = usTimeout % 1000000;
    }
    FD_ZERO(&writefds);
    FD_SET(m_sockfd, &writefds);
    rc = select(m_sockfd + 1, nullptr, &writefds, nullptr, ptv);
    if (0 == rc) {
        // TRACE_ERR_CLASS("send timeout.");
        return RC_TIMEOUT;
    } else if (rc < 0) {
        TRACE_ERR_CLASS("select error:%s", ERRSTR);
        return RC_ERROR;
    }

    /* 检查fd是否可写 */
    if (!FD_ISSET(m_sockfd, &writefds)) {
        TRACE_ERR_CLASS("writefds error");
        return RC_ERROR;
    }

    /* 防止在socket关闭后,write会产生SIGPIPE信号导致进程退出 */
    signal(SIGPIPE, SIG_IGN);
    int sndLen = write(m_sockfd, data, len);
    if (sndLen < 0) {
        TRACE_ERR_CLASS("sockfd(%d) write error:%s", m_sockfd, ERRSTR);
        return RC_ERROR;
    }
    // fsync(m_sockfd);
    return sndLen;
}

}  // namespace zemb
