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
#ifndef __ZEMB_FILE_UTIL_H__
#define __ZEMB_FILE_UTIL_H__

#include <string>
#include <vector>

#include "BaseType.h"
#include "IODevice.h"
/**
 *  @file   FileUtil.h
 *  @brief  文件工具类
 */
namespace zemb {
/**
 * @class File
 * @brief 文件类
 */
class File : public IODevice {
    DECL_CLASSNAME(File)

public:
    enum FileType { REG = 0, DIR, CHR, BLK, LINK, SOCK, FIFO };

public:
    File();
    virtual ~File();
    /**
     * @brief 判断文件是否存在
     * @param fileName 文件名
     * @return true 文件存在
     * @return false 文件不存在(如果fileName是目录也返回false)
     */
    static bool exists(const std::string& fileName);

    /**
     * @brief 获取文件类型
     * @param fileName
     * @return int
     */
    static int type(const std::string& fileName);
    /**
     * @brief 获取文件大小
     * @param fileName
     * @return int
     */
    static int getSize(const std::string& fileName);
    /**
     * @brief 删除文件
     * @param fileName
     * @return true
     * @return false
     */
    static bool removeFile(const std::string& fileName);

    /**
     * @brief 重命名文件
     * @param oldFile
     * @param newFile
     * @return true
     * @return false
     */
    static bool renameFile(const std::string& oldFile,
                           const std::string& newFile);

    /**
     * @brief 截取文件
     * @param fileName
     * @param size
     * @return bool
     */
    static bool truncFile(const std::string& fileName, int size);

    /**
     * @brief 读取文件
     * @param fileName 文件名
     * @return std::string 文件内容
     */
    static std::string readAll(const std::string& fileName);

    /**
     * @brief 判断文件是否打开
     * @return true
     * @return false
     */
    bool isOpen();

    /**
     * @brief 打开文件(继承自IODevice)
     * @param fileName 文件名
     * @param ioMode IO_MODE_E
     * @return true
     * @return false
     */
    bool open(const std::string& fileName, int ioMode) override;
    /**
     * @brief 关闭文件(继承自IODevice)
     * @return true
     * @return false
     */
    bool close() override;
    /**
     * @brief 读取文件(继承自IODevice)
     * @param buf
     * @param len
     * @return int 读取数据的长度,读取失败返回-1
     */
    int readData(char* buf, int len) override;
    /**
     * @brief 写入文件(继承自IODevice)
     * @param buf
     * @param len
     * @return int 写入数据的长度,写入失败返回-1
     */
    int writeData(const char* buf, int len) override;

    /**
     * @brief 将文件映射到内存
     * @return void* 内存地址
     */
    void* mapMemory();
    /**
     * @brief 取消内存映射
     * @return true
     * @return false
     */
    bool unmapMemory();
    /**
     * @brief 读取一行
     * @param lineStr
     * @return int
     */
    int readLine(std::string* lineStr);

    /**
     * @brief 读取指定长度字符串
     * @param size -1表示读取所有
     * @return std::string
     */
    std::string readString(int size = -1);
    /**
     * @brief 获取文件大小
     * @return int
     */
    int getSize();
    /**
     * @brief 获取文件名称
     * @return std::string
     */
    std::string getName();
    /**
     * @brief 是否到文件结尾
     * @return true
     * @return false
     */
    bool isEnd();
    /**
     * @brief 获取当前读写位置
     * @return int
     */
    int getPos();
    /**
     * @brief 设置当前读写位置
     * @param pos
     * @return int
     */
    int setPos(int pos);

private:
    FILE* m_fp{nullptr};
    std::string m_name{""};
    void* m_mmaddr{nullptr};
};

/**
 * @class FilePath
 * @brief 文件路径
 */
class FilePath {
    DECL_CLASSNAME(FilePath)

public:
    explicit FilePath(const std::string& filePath);
    ~FilePath();
    /**
     * @brief 获取文件路径中的文件夹部分
     * @return std::string
     */
    std::string dirName();
    /**
     * @brief 获取文件路径中的文件名部分
     * @return std::string
     */
    std::string baseName();

    /**
     * @brief 获取文件名后缀
     * @return std::string
     * @note 如文件名为~/Document/note.txt,调用该方法将返回".txt"
     */
    std::string suffix();
    /**
     * @brief 创建文件链接
     * @param filePath 文件路径
     * @param linkPath 链接路径
     * @param hard 是否是硬链接
     * @return true 创建成功
     * @return false 创建失败
     */
    static bool createLink(const std::string& filePath,
                           const std::string& linkPath, bool hard = false);

private:
    std::string m_filePath{""};
};

/**
 *  @class  Directory
 *  @brief  目录类.
 */
class Directory {
    DECL_CLASSNAME(Directory)

public:
    Directory();
    ~Directory();
    /**
     * @brief 判断目录是否为空
     * @param dirName
     * @return true 目录为空或不存在
     * @return false 目录不为空
     */
    static bool isEmpty(const std::string& dirName);
    /**
     * @brief 判断目录是否存在
     * @param dirName
     * @return true 目录存在
     * @return false 目录不存在或dirName不是目录
     */
    static bool exists(const std::string& dirName);
    /**
     * @brief 获取目录大小(包含子目录及所有文件，包括硬链接文件)
     * @param dirName
     * @return int
     */
    static int getSize(const std::string& dirName);
    /**
     * @brief 创建目录
     * @param dirName
     * @param recursive
     * @return true
     * @return false
     */
    static bool createDir(const std::string& dirName, int mode,
                          bool recursive = false);
    /**
     * @brief 删除目录
     * @param dirName
     * @param recursive
     * @return true
     * @return false
     */
    static bool removeDir(const std::string& dirName, bool recursive = false);
    /**
     * @brief 获取目录下的文件列表(包含目录)
     * @param dirName 目录名称
     * @param regexp 正则表达式(用于匹配文件名)
     * @return std::vector<std::string>
     */
    static std::vector<std::string> getFileList(const std::string& dirName,
                                                const std::string& regexp);
    /**
     * @brief 进入目录
     * @param dirName
     * @return true
     * @return false
     */
    bool enterDir(const std::string& dirName);
    /**
     * @brief 获取当前目录路径
     * @return std::string
     */
    std::string currentDir();
    /**
     * @brief 列出当前目录下的所有文件(包含目录)
     * @return std::vector<std::string>
     */
    std::vector<std::string> getFileList();

private:
    std::string m_currentDir{"."};
};

}  // namespace zemb
#endif
