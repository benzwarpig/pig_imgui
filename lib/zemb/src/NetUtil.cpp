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
#ifdef OS_CYGWIN
#else
#include "NetUtil.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "StrUtil.h"
#include "SysUtil.h"
#include "Tracer.h"

namespace zemb {
NetUtil::NetUtil() {}

NetUtil::~NetUtil() {}

bool NetUtil::isValidIP(const std::string ip) {
    auto splits = StrUtil::splitString(ip, ":");
    if (splits.empty()) {
        return false;
    }
    struct in_addr addr;
    int rc = inet_pton(AF_INET, CSTR(splits[0]), &addr);
    if (rc > 0) {
        return true;
    } else {
        return false;
    }
}

bool NetUtil::checkSubnetIP(const std::string& gatewayIP,
                            const std::string& subnetMask,
                            std::string* subnetIP) {
    struct in_addr ip, mask, sub, result;
    if (inet_pton(AF_INET, CSTR(gatewayIP), &ip) <= 0) {
        return false;
    }

    if (inet_pton(AF_INET, CSTR(subnetMask), &mask) <= 0) {
        return false;
    }

    if (inet_pton(AF_INET, CSTR(*subnetIP), &sub) <= 0) {
        return false;
    }

    if ((sub.s_addr & mask.s_addr) == (ip.s_addr & mask.s_addr)) {
        return true;
    }

    result.s_addr = (ip.s_addr & mask.s_addr);
    result.s_addr = result.s_addr | ((~mask.s_addr) & sub.s_addr);
    char tmpStr[16] = {0};
    if (nullptr == inet_ntop(AF_INET, &result, tmpStr, 16)) {
        return false;
    }
    *subnetIP = std::string(tmpStr);
    return true;
}

std::string NetUtil::getSubnetIP(const std::string& gatewayIP,
                                 const std::string& subnetMask, int subIndex) {
    struct in_addr ip, mask, result;
    if (inet_pton(AF_INET, CSTR(gatewayIP), &ip) <= 0) {
        return "";
    }
    if (inet_pton(AF_INET, CSTR(subnetMask), &mask) <= 0) {
        return "";
    }

    result.s_addr = ip.s_addr & mask.s_addr;
    result.s_addr = result.s_addr & (subIndex << 24);
    char tmp[16] = {0};
    if (nullptr == inet_ntop(AF_INET, &result, tmp, 16)) {
        return "";
    }
    return std::string(tmp);
}

std::string NetUtil::getInetAddr(const std::string intf) {
    struct sockaddr_in* addr;
    struct ifreq ifr;
    int sockfd;
    int rc;
    if (intf.size() >= IFNAMSIZ) {
        return "";
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return "";
    }

    rc = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::getInetAddr,ioctl error:SIOCGIFADDR,%s.", ERRSTR);
        close(sockfd);
        return "";
    }
    addr = (struct sockaddr_in*)&(ifr.ifr_addr);
    char* ipstr = inet_ntoa(addr->sin_addr);
    if (ipstr == nullptr) {
        TRACE_ERR("NetUtil::getInetAddr,inet_ntoa error.");
        close(sockfd);
        return "";
    }
    std::string ipaddr = ipstr;
    close(sockfd);
    return ipaddr;
}

std::string NetUtil::getMaskAddr(const std::string intf) {
    struct sockaddr_in* addr;
    struct ifreq ifr;
    int sockfd;
    int rc;
    if (intf.size() >= IFNAMSIZ) {
        return "";
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return "";
    }
    rc = ioctl(sockfd, SIOCGIFNETMASK, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::getMaskAddr,ioctl error:SIOCGIFNETMASK,%s.",
                  ERRSTR);
        close(sockfd);
        return "";
    }
    addr = (struct sockaddr_in*)&(ifr.ifr_addr);
    char* ipstr = inet_ntoa(addr->sin_addr);
    if (ipstr == nullptr) {
        TRACE_ERR("NetUtil::getMaskAddr,inet_ntoa error.");
        close(sockfd);
        return "";
    }
    std::string netmask = ipstr;
    close(sockfd);
    return netmask;
}

std::string NetUtil::getMacAddr(const std::string intf) {
    int sockfd;
    int rc;
    char mac[6];
    struct ifreq ifr;
    if (intf.size() >= IFNAMSIZ) {
        return "";
    }

    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return "";
    }

    rc = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::getMacAddr,ioctl error:SIOCGIFHWADDR,%s.", ERRSTR);
        close(sockfd);
        return "";
    }
    memcpy(mac, ifr.ifr_hwaddr.sa_data, sizeof(mac));
    char buf[20] = {0};
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);
    std::string macaddr = buf;
    close(sockfd);
    return macaddr;
}

NetHwState NetUtil::getHwState(const std::string intf) {
    int rc;
    struct ifreq ifr;
    int sockfd;
    if (intf.size() >= IFNAMSIZ) {
        return NetHwState::Error;
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        TRACE_ERR("NetUtil::getHwState, open socket error.");
        return NetHwState::Error;
    }
    rc = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::getHwState,ioctl error:SIOCGIFFLAGS,%s.", ERRSTR);
        close(sockfd);
        return NetHwState::Error;
    }

    close(sockfd);
    if (ifr.ifr_flags & IFF_UP) {
        return NetHwState::Up;
    } else {
        return NetHwState::Down;
    }
}

NetLinkState NetUtil::getLinkState(const std::string intf) {
    char cmd[64] = {0};
    if (intf.size() >= IFNAMSIZ) {
        return NetLinkState::Error;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "cat /sys/class/net/%s/operstate", CSTR(intf));
    std::string result;
    if (SysUtil::runCommand(cmd, &result) == RC_OK) {
        if (!result.empty()) {
            if (result.find("up") != std::string::npos) {
                return NetLinkState::Connected;
            }
        }
    }
    return NetLinkState::Disconnected;
}

bool NetUtil::setInetAddr(const std::string intf, const std::string ipaddr) {
    int rc;
    struct ifreq ifr;
    if (intf.size() >= IFNAMSIZ) {
        return false;
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return false;
    }

    rc = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setInetAddr,ioctl error:SIOCGIFADDR,%s.", ERRSTR);
        close(sockfd);
        return false;
    }
    struct sockaddr_in* addr = (struct sockaddr_in*)&(ifr.ifr_addr);
    addr->sin_family = AF_INET;
    rc = inet_aton(CSTR(ipaddr), &(addr->sin_addr));
    if (rc == 0) {
        TRACE_ERR("NetUtil::setInetAddr,inet_aton error.");
        close(sockfd);
        return false;
    }
    rc = ioctl(sockfd, SIOCSIFADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setInetAddr,ioctl error:SIOCSIFADDR,%s.", ERRSTR);
        close(sockfd);
        return false;
    }
    close(sockfd);
    return true;
}

bool NetUtil::setMaskAddr(const std::string intf, const std::string netmask) {
    int rc;
    struct ifreq ifr;
    if (intf.size() >= IFNAMSIZ) {
        return false;
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return false;
    }
    rc = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setMaskAddr,ioctl error:SIOCGIFADDR,%s.", ERRSTR);
        close(sockfd);
        return false;
    }
    struct sockaddr_in* addr = (struct sockaddr_in*)&(ifr.ifr_addr);
    addr->sin_family = AF_INET;
    rc = inet_aton(CSTR(netmask), &(addr->sin_addr));
    if (rc == 0) {
        TRACE_ERR("NetUtil::setMaskAddr,inet_aton error.");
        close(sockfd);
        return false;
    }

    rc = ioctl(sockfd, SIOCSIFNETMASK, &ifr);
    if (rc < 0) {
        close(sockfd);
        return false;
    }
    close(sockfd);
    return true;
}

bool NetUtil::setMacAddr(const std::string intf, const std::string mac) {
    int sockfd;
    int rc;
    char macaddr[6] = {0};
    struct ifreq ifr;
    if (intf.size() >= IFNAMSIZ || mac.size() != 17) {
        return false;
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return false;
    }

    const char* pMacStr = CSTR(mac);
    for (auto i = 0; i < 6; i++) {
        *(macaddr + i) = StrUtil::ctoi(*(pMacStr + 3 * i)) * 16 +
                         StrUtil::ctoi(*(pMacStr + 3 * i + 1));
    }

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy((unsigned char*)ifr.ifr_hwaddr.sa_data, macaddr, 6);
    rc = ioctl(sockfd, SIOCSIFHWADDR, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setMacAddr[%s],ioctl error:SIOCSIFHWADDR,%s.",
                  CSTR(mac), ERRSTR);
        close(sockfd);
        return false;
    }
    close(sockfd);
    return true;
}

bool NetUtil::setHwState(const std::string intf, NetHwState state) {
    int rc;
    struct ifreq ifr;
    int sockfd;

    if (intf.size() >= IFNAMSIZ) {
        return false;
    }
    snprintf(ifr.ifr_name, intf.size(), "%s", CSTR(intf));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return false;
    }
    /* 先读取ifflags */
    rc = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setHwState,ioctl error:SIOCGIFFLAGS,%s.", ERRSTR);
        close(sockfd);
        return false;
    }

    /* 再设置ifflags */
    if (state == NetHwState::Down) {
        ifr.ifr_flags &= (~IFF_UP);
    } else {
        ifr.ifr_flags |= IFF_UP;
    }
    rc = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    if (rc < 0) {
        TRACE_ERR("NetUtil::setHwState,ioctl error:SIOCSIFFLAGS,%s.", ERRSTR);
        close(sockfd);
        return false;
    }

    close(sockfd);
    return true;
}

std::string NetUtil::getHostIP(const std::string& hostName) {
    auto splits = StrUtil::splitString(hostName, ":");
    std::string port = "";
    if (splits.size() >= 2) {
        port = std::string(":") + splits[1];
    }
    struct hostent* host = gethostbyname(CSTR(splits[0]));
    if (host == nullptr) {
        return "";
    }
    return std::string(inet_ntoa(*((struct in_addr*)host->h_addr)))
        .append(port);
}
}  // namespace zemb
#endif
