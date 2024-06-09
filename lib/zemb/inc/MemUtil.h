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
#ifndef __ZEMB_MEM_UTIL_H__
#define __ZEMB_MEM_UTIL_H__

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "BaseType.h"
#include "ThreadUtil.h"

/**
 * @file MemUtil.h
 * @brief 内存工具
 */
namespace zemb {

/**
 * @class MemPool
 * @brief 内存池
 */
class MemPool {
    DECL_CLASSNAME(MemPool)

public:
    MemPool();
    ~MemPool();
    /**
     * @brief 初始化内存池
     * @param blockNum
     * @param blockSize
     * @param memStart 如果指定了memStart则在memStart内存地址处创建内存池
     * @return true
     * @return false
     */
    bool init(int blockNum, int blockSize, void* memStart = nullptr);
    /**
     * @brief 申请内存
     * @param memoryName 内存名称
     * @param memorySize 内存大小
     * @return void*
     */
    void* getMemory(const std::string& memoryName, int memorySize);
    /**
     * @brief 释放内存
     * @param memoryName 内存名称
     * @return true
     * @return false
     */
    bool putMemory(const std::string& memoryName);
    /**
     * @brief 显示内存
     *
     */
    void showMemory();

private:
    struct MemBlock {
        bool isUsed{false};
        void* memStart{nullptr};
        std::string memName{""};
    };
    int m_blockNum{0};
    int m_blockSize{0};
    void* m_poolStart{nullptr};
    std::vector<std::unique_ptr<MemBlock>> m_memBlocks;
    std::mutex m_poolMutex;
};

/**
 *  @class  MemShared
 *  @brief  共享内存类(可以多进程共享)
 */
class MemShared {
    DECL_CLASSNAME(MemShared)

public:
    enum Type { SHM_MMAP = 0, FILE_MMAP };

public:
    MemShared();
    virtual ~MemShared();
    /**
     * @brief 打开共享内存
     * @param name 共享内存名称
     * @param size 共享内存大小
     * @param mode IO_MODE_E,仅支持部分模式
     * @param type MemShared::Type
     * @return true
     * @return false
     */
    bool open(std::string name, int size, int mode,
              MemShared::Type type = SHM_MMAP);
    /**
     * @brief 关闭共享内存
     * @return true
     * @return false
     */
    bool close();
    /**
     * @brief 放置数据到内存
     * @param data
     * @param size
     * @return int
     */
    int putData(void* data, int size);
    /**
     * @brief 获取内存数据
     * @param buffer
     * @param size
     * @return int
     */
    int getData(void* buffer, int size);
    /**
     * @brief 获取内存大小
     * @return int
     */
    int size();
    /**
     * @brief 获取内存地址
     * @return void*
     */
    void* data();

    /**
     * @brief 返回文件描述符
     * @return int
     */
    int fd() { return m_fd; }

private:
    MemShared::Type m_type{SHM_MMAP};
    int m_fd{-1};
    int m_size{0};
    void* m_memStart{nullptr};
    std::string m_name{""};
    std::shared_ptr<Semaphore> m_procSem{nullptr};
};
}  // namespace zemb
#endif
