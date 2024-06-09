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
 * Copyright 2014-2020 @ ShenZhen ,China
 *******************************************************************************/
#ifndef __ZEMB_SYS_UTIL_H__
#define __ZEMB_SYS_UTIL_H__

/**
 *  @file   SysUtil.h
 *  @brief  进程工具类.
 */
#include <string>
#include <vector>

#include "BaseType.h"
#include "IODevice.h"
#include "StrUtil.h"
#include "Thread.h"
namespace zemb {
/**
 * @class SysUtil
 * @brief 进程工具类
 */
class SysUtil {
public:
    SysUtil() {}
    ~SysUtil() {}
    /**
     * @brief 执行命令
     * @param cmd
     * @return int int 成功返回RC_OK,失败返回RC_ERROR
     * @note 内部使用system实现,函数会阻塞直到cmd执行完
     */
    static int runCommand(const std::string& cmd);
    /**
     * @brief 执行命令
     * @param cmd 命令
     * @param resultStr 命令输出结果
     * @param timeoutSec 超时时间(-1:等待进程退出,其他值:最长等待时间)
     * @return int 成功返回RC_OK,错误返回RC_ERROR,超时返回RC_TIMEOUT
     */
    static int runCommand(const std::string& cmd, std::string* resultStr,
                          int timeoutSec = -1);
    /**
     * @brief 启动进程
     * @param procName 进程名
     * @param args 参数
     * @param envs 环境变量
     * @param delays 延时时间
     * @return int 成功返回RC_OK,否则返回RC_ERROR
     */
    static int spawnOne(const std::string& procName, char** args,
                        int delays = 0);
    /**
     * @brief 停止除自身外的进程(杀死进程)
     * @param procName 进程名
     */
    static void killOther(std::string procName);
    /**
     * @brief 获取进程PID
     * @param processName 进程名称
     * @return std::vector<int> 进程PID数组
     */
    static std::vector<int> getPidsByName(const std::string& processName);
    /**
     * @brief 获取环境变量
     * @param name 环境变量名称
     * @return std::string
     */
    static std::string getEnv(const std::string& name);
    /**
     * @brief 设置环境变量
     * @param name 环境变量名称
     * @param value 环境变量值
     */
    static void setEnv(const std::string& name, const std::string& value);
    /**
     * @brief 设置进程亲和性(将当前进程/线程绑定到指定的cpu)
     * @param cpuMask 类型:bitmap,bit0代表cpu0,以此类推
     * @return true
     * @return false
     */
    static bool setAffinity(int cpuMask);
    /**
     * @brief 获取进程占用的内存
     * @param name 进程名称
     * @return int
     */
    static int getProcMemory(const std::string& name);

    /**
     * @brief 重启系统
     * @return true
     * @return false
     */
    static bool rebootSystem();
};

/**
 * @class SysStat
 * @brief 系统统计
 */
class SysStat : public Runnable {
    DECL_CLASSNAME(SysStat)
    struct CpuStat {
        char name[20];
        uint32 user{0};
        uint32 nice{0};
        uint32 system{0};
        uint32 idle{0};
        uint32 iowait{0};
        uint32 irq{0};
        uint32 softirq{0};
    };

public:
    SysStat();
    ~SysStat();
    /**
     * @brief 开始统计
     * @param msInterval 统计间隔时间(ms)
     * @param cupId (-1:所有CPU,0~n:单CPU)
     * @param statCallback (统计回调)
     * @return true
     * @return false
     */
    bool start(int cpuId = -1, uint32 msInterval = 1000,
               std::function<void(void)> statCallback = nullptr);
    /**
     * @brief 停止统计
     */
    void stop();
    /**
     * @brief 获取CPU使用率
     */
    float getCpuUsage();
    /**
     * @brief 获取系统内存大小(kbytes)
     * @return uint32
     */
    uint32 getMemTotal();
    /**
     * @brief 获取系统内存空闲量(kbytes)
     * @return uint32
     */
    uint32 getMemFree();

private:
    void run(const Thread& thread) override;
    void updateCpuStat();
    void updateMemStat();

private:
    Thread m_thread;
    uint32 m_interval{1000};
    int m_cpuId{-1};
    std::function<void(void)> m_statCallback{nullptr};
    CpuStat m_lastCpuStat;
    CpuStat m_currCpuStat;
    uint32 m_memTotalKB{0};
    uint32 m_memFreeKB{0};
};

}  // namespace zemb
#endif
