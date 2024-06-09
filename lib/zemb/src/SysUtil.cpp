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
#include "SysUtil.h"

#include <linux/reboot.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <iostream>

#include "FileUtil.h"
#include "StrUtil.h"
#include "Tracer.h"

#define $TIMEOUT_EXECUTE (3) /* 默认超时时间为2秒 */
extern char** environ;       /* 当前环境变量(Linux全局变量) */

namespace zemb {
int SysUtil::runCommand(const std::string& cmd) {
    pid_t status;
    status = system(cmd.data());
    if (-1 == status) {  // 创建子进程失败
        TRACE_ERR("system fork error!")
        return RC_ERROR;
    } else {
        if (WIFEXITED(status)) {             // cmd被成功调用
            if (0 == WEXITSTATUS(status)) {  // 判断cmd返回值
                return RC_OK;
            }
        }
    }
    TRACE_ERR("system cmd error, exit code: %d !", WEXITSTATUS(status));
    return RC_ERROR;
}

int SysUtil::runCommand(const std::string& cmd, std::string* result,
                        int timeoutSec) {
    FILE* fp = popen(CSTR(cmd), "r");
    if (nullptr == fp) {
        TRACE_ERR("SysUtil::runCommand(%s), popen failed.", CSTR(cmd));
        return RC_ERROR;
    }
    std::string procName = StrUtil::trimHeadUnvisible(cmd);
    result->clear();
    if (timeoutSec < 0) {
        procName = "";
    }
    timeoutSec = MAX(timeoutSec, $TIMEOUT_EXECUTE);
    int tout = timeoutSec;
    time_t startTime = ::time(nullptr);
    while (1) {
        time_t endTime = ::time(nullptr);
        if ((endTime - startTime) < 0 || (endTime - startTime) >= timeoutSec) {
            break;
        }
        if (tout <= 0) {
            break;
        }
        int fd = fileno(fp);
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int rc = select(fd + 1, &readfds, nullptr, nullptr, &tv);
        if (rc <= 0 || (!FD_ISSET(fd, &readfds))) {
            tout -= 1;
            continue;
        } else {
            char buf[256] = {0};
            rc = fread(buf, 1, sizeof(buf), fp);
            if (rc <= 0) {
                break;
            }
            result->append(std::string(buf, rc));
        }
    }
    if (!procName.empty()) {
        killOther(procName);
    }
    pclose(fp);
    return RC_OK;
}

int SysUtil::spawnOne(const std::string& procName, char** argv, int delays) {
    int pid = fork();
    if (pid < 0) {
        TRACE_ERR("SysUtil::spawnOne, fork error: %s!", ERRSTR);
        return RC_ERROR;
    } else if (pid == 0) {  // 子进程
        if (delays > 0) {
            sleep(delays);
        }
        if (execvpe(CSTR(procName), argv, environ) < 0) {
            PRINT_ERR("proc [%s] exec error!", CSTR(procName));
            exit(errno);
        } else {
            exit(0);
        }
    }
    return pid;
}

void SysUtil::killOther(std::string procName) {
    std::vector<int> pids = getPidsByName(CSTR(procName));
    auto self = getpid();
    for (auto i = 0; i < pids.size(); i++) {
        if (pids[i] != self) {
            kill(pids[i], SIGKILL);
        }
    }
}

std::vector<int> SysUtil::getPidsByName(const std::string& processName) {
    std::vector<int> pidList;
    if (processName.empty()) {
        return pidList;
    }
    Directory dir;
    dir.enterDir("/proc");
    std::vector<std::string> fileNameList = dir.getFileList();
    int num = fileNameList.size();
    for (auto i = 0; i < num; i++) {
        if (!StrUtil::isIntString(fileNameList[i])) {
            continue;
        }
        File file;
        std::string statusFile = "/proc/" + fileNameList[i] + "/status";
        if (!file.open(CSTR(statusFile), IO_MODE_RD_ONLY)) {
            continue;
        }
        std::string firstLine;
        if (RC_ERROR == file.readLine(&firstLine)) {
            continue;
        }
        std::vector<std::string> result = StrUtil::splitString(firstLine, ":");
        if (result.size() != 2) {
            continue;
        }
        std::string pid = StrUtil::trimTailBlank(result[1]);
        if (pid == processName) {
            pidList.push_back(atoi(CSTR(fileNameList[i])));
        }
    }
    return pidList;
}

std::string SysUtil::getEnv(const std::string& name) {
    char* value = getenv(CSTR(name));
    if (value) {
        return std::string(value);
    }
    return "";
}

void SysUtil::setEnv(const std::string& name, const std::string& value) {
    setenv(CSTR(name), CSTR(value), 1);
}

bool SysUtil::setAffinity(int cpuMask) {
    // 使用命令查看进程xxx所运行的CPU: ps -eo pid,args,psr | grep xxx
    cpu_set_t set;
    CPU_ZERO(&set);
    for (int cpu = 0; cpuMask > 0; cpu++, cpuMask >>= 1) {
        if (cpuMask & 1) {
            CPU_SET(cpu, &set);
        }
    }
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        TRACE_ERR("SysUtil::setAffinity, Set CPU affinity error: %s", ERRSTR);
        return false;
    }
    return true;
}

int SysUtil::getProcMemory(const std::string& process) {
    auto pids = getPidsByName(process);
    if (pids.size() <= 0) {
        return -1;
    }
    File file;
    if (!file.open(StrUtil::format("/proc/%d/status", pids[0]),
                   IO_MODE_RD_ONLY)) {
        TRACE_ERR("SysUtil::getProcMemory, cannot read process status file!");
        return -1;
    }
    std::string lineStr;
    while (file.readLine(&lineStr) > 0) {
        if (lineStr.compare(0, 6, "VmRSS:") == 0) {
            auto sections = StrUtil::splitString(lineStr, " ");
            if (sections.size() > 1) {
                return std::stoi(CSTR(sections[1]));
            }
        }
    }
    return -1;
}

bool SysUtil::rebootSystem() {
    if (reboot(LINUX_REBOOT_CMD_RESTART) < 0) {
        TRACE_WARN("system reboot failed!");
        return false;
    }
    TRACE_INFO("system reboot ...");
    return true;
}

SysStat::SysStat() {}
SysStat::~SysStat() { m_thread.stop(); }

bool SysStat::start(int cpuId, uint32 msInterval,
                    std::function<void(void)> statCallback) {
    memset(m_lastCpuStat.name, 0, sizeof(m_lastCpuStat.name));
    memset(m_currCpuStat.name, 0, sizeof(m_currCpuStat.name));
    m_memTotalKB = 0;
    m_memFreeKB = 0;
    m_cpuId = cpuId;
    m_interval = msInterval;
    m_statCallback = statCallback;
    return m_thread.start(*this);
}
void SysStat::stop() { m_thread.stop(); }

float SysStat::getCpuUsage() {
    if (m_lastCpuStat.name[0] != 0 && m_currCpuStat.name[0] != 0) {
        float od = static_cast<float>(
            m_lastCpuStat.user + m_lastCpuStat.nice + m_lastCpuStat.system +
            m_lastCpuStat.idle + m_lastCpuStat.softirq + m_lastCpuStat.iowait +
            m_lastCpuStat.irq);
        float nd = static_cast<float>(
            m_currCpuStat.user + m_currCpuStat.nice + m_currCpuStat.system +
            m_currCpuStat.idle + m_currCpuStat.softirq + m_currCpuStat.iowait +
            m_currCpuStat.irq);
        if ((nd - od) != 0) {
            return (100.0 - (static_cast<float>(m_currCpuStat.idle -
                                                m_lastCpuStat.idle)) /
                                (nd - od) * 100.00);
        } else {
            return 0.0f;
        }
    }
    return 0.0f;
}

uint32 SysStat::getMemTotal() { return m_memTotalKB; }

uint32 SysStat::getMemFree() { return m_memFreeKB; }

void SysStat::run(const Thread& thread) {
    while (thread.isRunning()) {
        updateCpuStat();
        updateMemStat();
        Thread::msleep(m_interval);
        if (m_statCallback) {
            m_statCallback();
        }
    }
}

void SysStat::updateCpuStat() {
    File file;
    if (!file.open("/proc/stat", IO_MODE_RD_ONLY)) {
        TRACE_ERR_CLASS("cannot read cpu stat file !");
        return;
    }
    std::string statStr;
    while (file.readLine(&statStr) > 0) {
        auto sections = StrUtil::splitString(statStr, " ");
        if (sections.size() > 7 && sections[0].compare(0, 3, "cpu") == 0) {
            if (m_cpuId >= 0) {
                std::string cpuName = StrUtil::format("cpu%d", m_cpuId);
                if (sections[0].compare(0, cpuName.size(), cpuName) != 0) {
                    continue;
                }
            }
            m_lastCpuStat = m_currCpuStat;
            memcpy(m_currCpuStat.name, sections[0].data(), sections[0].size());
            m_currCpuStat.user = std::stoi(CSTR(sections[1]));
            m_currCpuStat.nice = std::stoi(CSTR(sections[2]));
            m_currCpuStat.system = std::stoi(CSTR(sections[3]));
            m_currCpuStat.idle = std::stoi(CSTR(sections[4]));
            m_currCpuStat.iowait = std::stoi(CSTR(sections[5]));
            m_currCpuStat.irq = std::stoi(CSTR(sections[6]));
            m_currCpuStat.softirq = std::stoi(CSTR(sections[7]));
            break;
        }
    }
    file.close();
}

void SysStat::updateMemStat() {
    File file;
    if (!file.open("/proc/meminfo", IO_MODE_RD_ONLY)) {
        TRACE_ERR_CLASS("cannot read mem info file !");
        return;
    }
    std::string statStr;
    while (file.readLine(&statStr) > 0) {
        auto sections = StrUtil::splitString(statStr, " ");
        if (sections.size() > 2 &&
            sections[0].compare(0, 9, "MemTotal:") == 0) {
            m_memTotalKB = std::stoi(CSTR(sections[1]));
        }
        if (sections.size() > 2 && sections[0].compare(0, 8, "MemFree:") == 0) {
            m_memFreeKB = std::stoi(CSTR(sections[1]));
            break;
        }
    }
    file.close();
}
}  // namespace zemb
