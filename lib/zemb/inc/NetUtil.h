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
#ifndef __ZEMB_NET_UTIL_H__
#define __ZEMB_NET_UTIL_H__

#ifdef OS_CYGWIN
#else
#include <string>

#include "BaseType.h"
/**
 * @file NetUtil.h
 * @brief 网络工具
 */
namespace zemb {

/**
 * @enum  NetHwState
 * @brief 网络硬件状态
 */
enum class NetHwState {
    Down, /**< 网口已被ifconfig down */
    Up,   /**< 网口已被ifconfig up */
    Error
};

/**
 * @enum  NetLinkState
 * @brief 网络连接状态
 */
enum class NetLinkState {
    Disconnected, /**< 网口未插入网线,link down */
    Connected,    /**< 网口已插入网线,link up */
    Error
};

/**
 *  @class  NetUtil
 *  @brief  网络工具类,提供对网络设别的常用操作.
 */
class NetUtil {
public:
    NetUtil();
    ~NetUtil();
    /**
     * @brief 判断是否是合法IP
     * @param ip
     * @return true
     * @return false
     */
    static bool isValidIP(const std::string ip);
    /**
     * @brief 校验子网IP
     * @param gatewayIP
     * @param subnetMask
     * @param subnetIP
     * @return true
     * @return false
     */
    static bool checkSubnetIP(const std::string& gatewayIP,
                              const std::string& subnetMask,
                              std::string* subnetIP);
    /**
     * @brief 获取子网IP
     * @param gatewayIP
     * @param subnetMask
     * @param subIndex
     * @return std::string
     */
    static std::string getSubnetIP(const std::string& gatewayIP,
                                   const std::string& subnetMask, int subIndex);
    /**
     *  @brief  获取网口ip地址
     *  @param  intf 网口名称,如eth0
     *  @return std::string IP地址字符串,获取失败返回空字符串
     */
    static std::string getInetAddr(const std::string intf);
    /**
     *  @brief  获取网口子网掩码地址
     *  @param  intf 网口名称,如eth0
     *  @return std::string IP地址字符串,获取失败返回空字符串
     */
    static std::string getMaskAddr(const std::string intf);
    /**
     *  @brief  获取网口mac地址
     *  @param  intf 网口名称,如eth0
     *  @return std::string mac地址字符串,获取失败返回空字符串
     */
    static std::string getMacAddr(const std::string intf);
    /**
     *  @brief  获取网口状态
     *  @param  intf 网口名称,如eth0
     *  @return NetHwState 成功返回网口状态
     */
    static NetHwState getHwState(const std::string intf);
    /**
     *  @brief  获取网络连接状态
     *  @param  intf 网口名称,如eth0
     *  @return NetLinkState 成功返回网口状态
     */
    static NetLinkState getLinkState(const std::string intf);
    /**
     * @brief 设置网口IP
     * @param intf
     * @param ipaddr
     * @return true
     * @return false
     */
    static bool setInetAddr(const std::string intf, const std::string ipaddr);
    /**
     * @brief 设置子网掩码
     * @param intf
     * @param netmask
     * @return true
     * @return false
     */
    static bool setMaskAddr(const std::string intf, const std::string netmask);
    /**
     * @brief 设置MAC地址
     * @param intf
     * @param mac
     * @return true
     * @return false
     */
    static bool setMacAddr(const std::string intf, const std::string mac);
    /**
     * @brief 设置网口状态
     * @param intf
     * @param state
     * @return true
     * @return false
     */
    static bool setHwState(const std::string intf, NetHwState state);

    /**
     * @brief 获取域名对应的IP
     * @param hostName 主机域名
     * @return std::string IP地址(如果域名带端口号,端口号一并返回)
     */
    static std::string getHostIP(const std::string& hostName);
};
}  // namespace zemb
#endif
#endif
