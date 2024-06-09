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
#include "FileUtil.h"

#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

#include "RegExp.h"
#include "StrUtil.h"
#include "SysUtil.h"
#include "Tracer.h"

namespace zemb {
File::File() {}

File::~File() { this->close(); }

bool File::exists(const std::string& filePath) {
    if (0 == access(CSTR(filePath), F_OK)) {
        struct stat fileStat;
        if (0 != lstat(CSTR(filePath), &fileStat)) {
            return false;
        }
        if (S_ISDIR(fileStat.st_mode)) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

bool File::removeFile(const std::string& fileName) {
    if (remove(CSTR(fileName)) != 0) {
        TRACE_ERR("remove file(%s) error: %s.", CSTR(fileName), ERRSTR);
        return false;
    }
    return true;
}

bool File::renameFile(const std::string& oldFile, const std::string& newFile) {
    if (rename(CSTR(oldFile), CSTR(newFile)) != 0) {
        TRACE_ERR("rename file(%s)->(%s) error: %s.", CSTR(oldFile),
                  CSTR(newFile), ERRSTR);
        return false;
    }
    return true;
}

bool File::truncFile(const std::string& fileName, int size) {
    File file;
    if (size < 0) {
        return false;
    }
    if (!file.open(fileName, IO_MODE_RDWR_ONLY)) {
        TRACE_ERR("open file(%s) error: %s", VSTR(fileName));
        return false;
    }
    if (ftruncate(file.fd(), size) != 0) {
        return false;
    }
    return true;
}

std::string File::readAll(const std::string& fileName) {
    File file;
    if (!file.open(fileName, IO_MODE_RD_ONLY)) {
        TRACE_ERR("open file(%s) error: %s", VSTR(fileName));
        return "";
    }
    return file.readString(-1);
}
int File::getSize(const std::string& fileName) {
    struct stat statbuf;
    if (0 == lstat(CSTR(fileName), &statbuf)) {
        int size = statbuf.st_size;
        return size;
    }
    return 0;
}

bool File::isOpen() {
    if (m_fp == nullptr) {
        return false;
    }
    return true;
}

bool File::open(const std::string& fileName, int ioMode) {
    std::string modestr;
    if (fileName.empty()) {
        TRACE_ERR_CLASS("file name is empty.");
        return false;
    }

    switch (ioMode) {
        case IO_MODE_RD_ONLY:
            modestr = "rb";
            break;
        case IO_MODE_WR_ONLY:
            modestr = "wb";
            break;
        case IO_MODE_RDWR_ONLY:
            modestr = "rb+";
            break;
        case IO_MODE_REWR_ORNEW:
            modestr = "wb";
            break;
        case IO_MODE_RDWR_ORNEW:
            modestr = "wb+";
            break;
        case IO_MODE_APPEND_ORNEW:
            modestr = "ab";
            break;
        case IO_MODE_RDAPPEND_ORNEW:
            modestr = "ab+";
            break;
        default:
            TRACE_ERR_CLASS("Unsupport mode(%d)");
            return false;
    }

    if (m_fp != nullptr) {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_fp = fopen(CSTR(fileName), CSTR(modestr));
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("open file:%s, error:%s.", CSTR(fileName), ERRSTR);
        return false;
    }
    m_fd = fileno(m_fp);
    m_name = fileName;
    return true;
}

bool File::close() {
    if (m_fp != nullptr) {
        fclose(m_fp);
        m_fp = nullptr;
    }
    return true;
}

int File::readData(char* buf, int len) {
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("parameter error:buf=0x%x,count=%d.", buf, len);
        return RC_ERROR;
    }
    return fread(buf, 1, len, m_fp);
}

int File::writeData(const char* buf, int len) {
    int rc;
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    if (nullptr == buf || len <= 0) {
        TRACE_ERR_CLASS("parameter error.");
        return RC_ERROR;
    }
    rc = fwrite(buf, 1, len, m_fp);
    fflush(m_fp);
    return rc;
}

void* File::mapMemory() {
    if (m_mmaddr == nullptr) {
        m_mmaddr = mmap(nullptr, getSize(), PROT_READ | PROT_WRITE, MAP_SHARED,
                        fileno(m_fp), 0);
        if (MAP_FAILED == m_mmaddr) {
            TRACE_ERR_CLASS("file[%s] map memory error:%s!", CSTR(m_name),
                            ERRSTR);
            return nullptr;
        }
    }
    return m_mmaddr;
}

bool File::unmapMemory() {
    if (m_mmaddr != nullptr) {
        if (-1 == munmap(m_mmaddr, getSize())) {
            TRACE_ERR_CLASS("file[%s] unmap memory error:%s!", CSTR(m_name),
                            ERRSTR);
            return false;
        }
        m_mmaddr = nullptr;
    }
    return true;
}

int File::readLine(std::string* lineStr) {
    int rc;
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    lineStr->clear();
    for (;;) {
        char buf[2] = {0};
        rc = readData(buf, 1);
        if (RC_ERROR == rc) {
            return RC_ERROR;
        } else if (0 == rc) {
            if (isEnd()) {
                return lineStr->size();
            } else {
                return RC_ERROR;
            }
        } else {
            lineStr->append(buf);
            if (std::string::npos != lineStr->find("\r\n")) {
                *lineStr = lineStr->substr(0, lineStr->size() - 2);
                return lineStr->size();
            } else if (std::string::npos != lineStr->find("\n")) {
                *lineStr = lineStr->substr(0, lineStr->size() - 1);
                return lineStr->size();
            }
        }
    }
}

std::string File::readString(int size) {
    std::string result = "";
    char buf[512] = {0};

    if (size < 0) {
        size = getSize();
    }
    int blocks = size / sizeof(buf);
    int spares = size % sizeof(buf);
    for (int n = 0; n < blocks; n++) {
        int len = readData(buf, sizeof(buf));
        if (len <= 0) {
            return result;
        } else {
            result.append(buf, len);
        }
        if (len != sizeof(buf)) {
            return result;
        }
    }
    if (spares > 0) {
        int len = readData(buf, spares);
        if (len > 0) {
            result.append(buf, len);
        }
    }
    return result;
}

int File::getSize() {
    int fileSize, pos;
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    pos = ftell(m_fp); /* 保存当前读写位置 */
    if (fseek(m_fp, 0, SEEK_END) < 0) {
        return RC_ERROR;
    }
    fileSize = ftell(m_fp);
    if (fileSize < 0) {
        fileSize = RC_ERROR;
    }
    fseek(m_fp, pos, SEEK_SET); /* 恢复当前读写位置 */
    return fileSize;
}

std::string File::getName() { return m_name; }

bool File::isEnd() {
    int pos = getPos();
    if (pos == RC_ERROR || pos != getSize()) {
        return false;
    }
    return true;
}

int File::getPos() {
    int pos;
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    pos = ftell(m_fp);
    if (pos < 0) {
        return RC_ERROR;
    } else {
        return pos;
    }
}

int File::setPos(int pos) {
    if (nullptr == m_fp) {
        TRACE_ERR_CLASS("File not open.");
        return RC_ERROR;
    }
    if (0 == fseek(m_fp, pos, SEEK_SET)) {
        return RC_OK;
    } else {
        return RC_ERROR;
    }
}

FilePath::FilePath(const std::string& filePath) : m_filePath(filePath) {}
FilePath::~FilePath() {}

std::string FilePath::dirName() {
    std::string ret = "";
    if (m_filePath.empty()) {
        return ret;
    }
    char* path = strdup(CSTR(m_filePath));
    if (path) {
        ret = std::string(dirname(path));
        free(path);
    }
    return ret;
}

std::string FilePath::baseName() {
    std::string ret = "";
    if (m_filePath.empty()) {
        return ret;
    }
    char* path = strdup(CSTR(m_filePath));
    if (path) {
        ret = std::string(basename(path));
        free(path);
    }
    return ret;
}

std::string FilePath::suffix() { return StrUtil::suffix(baseName(), "."); }

bool FilePath::createLink(const std::string& filePath,
                          const std::string& linkPath, bool hard) {
    if (filePath.empty() || linkPath.empty()) {
        return false;
    }
    if (hard) {
        if (link(CSTR(filePath), CSTR(linkPath)) != 0) {
            TRACE_DBG("create hard link error:%s", ERRSTR);
            return false;
        }
    } else {
        if (symlink(CSTR(filePath), CSTR(linkPath)) != 0) {
            TRACE_DBG("create soft link error:%s", ERRSTR);
            return false;
        }
    }
    return true;
}

Directory::Directory() {}

Directory::~Directory() {}

bool Directory::isEmpty(const std::string& dirName) {
    DIR* dirp = nullptr;
    struct dirent* dp = nullptr;
    int num = 0;
    dirp = opendir(CSTR(dirName));
    if (nullptr == dirp) {
        return true;
    }
    while ((dp = readdir(dirp)) != nullptr) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        } else {
            num++;
            break;
        }
    }
    closedir(dirp);
    if (num > 0) {
        return false;
    }
    return true;
}

bool Directory::exists(const std::string& dirName) {
    if (dirName.empty()) {
        return false;
    }
    if (0 == access(CSTR(dirName), F_OK)) {
        struct stat fileStat;
        if (0 != lstat(CSTR(dirName), &fileStat)) {
            return false;
        } else {
            if (S_ISDIR(fileStat.st_mode)) {
                return true;
            }
            if (S_ISLNK(fileStat.st_mode)) {
                DIR* dirp = opendir(CSTR(dirName));
                if (dirp != nullptr) {
                    closedir(dirp);
                    return true;
                }
            }
        }
    }
    return false;
}

int Directory::getSize(const std::string& dirName) {
    if (Directory::exists(dirName)) {
        int sum = 0;
        Directory dir;
        std::string path(dirName);
        dir.enterDir(path);
        std::vector<std::string> fileList = dir.getFileList();

        int fileNum = fileList.size();
        for (auto i = 0; i < fileNum; i++) {
            if (fileList[i] == "." || fileList[i] == "..") {
                continue;
            }
            std::string fileName = path + "/";
            fileName.append(fileList[i]);
            if (File::exists(CSTR(fileName))) {
                sum += File::getSize(CSTR(fileName));
            }
            if (Directory::exists(CSTR(fileName))) {
                sum += File::getSize(CSTR(fileName));
                sum += Directory::getSize(CSTR(fileName));
            }
        }
        return sum;
    } else {
        TRACE_ERR("%s not exist!", CSTR(dirName));
        return 0;
    }
}

int File::type(const std::string& fileName) {
    struct stat fileStat;
    if (0 != lstat(CSTR(fileName), &fileStat)) {
        return -1;
    } else {
        if (S_ISREG(fileStat.st_mode))
            return File::FileType::REG;
        else if (S_ISDIR(fileStat.st_mode))
            return File::FileType::DIR;
        else if (S_ISCHR(fileStat.st_mode))
            return File::FileType::CHR;
        else if (S_ISBLK(fileStat.st_mode))
            return File::FileType::BLK;
        else if (S_ISFIFO(fileStat.st_mode))
            return File::FileType::FIFO;
        else if (S_ISLNK(fileStat.st_mode))
            return File::FileType::LINK;
        else if (S_ISSOCK(fileStat.st_mode))
            return File::FileType::SOCK;
        else
            return -1;
    }
}

bool Directory::createDir(const std::string& dirName, int mode,
                          bool recursive) {
    if (dirName.empty()) return false;
    if (recursive) {
        vector<string> dirNames = StrUtil::splitString(dirName, "/", true);
        if (!dirNames.empty()) {
            string dirPath = "";
            for (auto i = 0; i < dirNames.size(); i++) {
                if (dirNames[i] == "/") {
                    if (i == 0) dirPath += "/";
                } else {
                    dirPath += dirNames[i];
                    dirPath += "/";
                    if (!Directory::exists(CSTR(dirPath))) {
                        if (mkdir(CSTR(dirPath), mode) != 0) {
                            TRACE_ERR("Directory::createDir(%s) error:%s.",
                                      CSTR(dirPath), ERRSTR);
                            return false;
                        }
                    }
                }
            }
            return true;
        } else {
            return false;
        }
    } else {
        if (mkdir(CSTR(dirName), mode) != 0) {
            TRACE_ERR("Directory::createDir(%s) error:%s.", CSTR(dirName),
                      ERRSTR);
            return false;
        }
        return true;
    }
}

bool Directory::removeDir(const std::string& dirName, bool recursive) {
    if (dirName.empty()) return false;
    if (recursive) {
        vector<string> dirNames = StrUtil::splitString(dirName, "/", true);
        if (!dirNames.empty()) {
            string dirPath = "";
            for (auto i = 0; i < dirNames.size(); i++) {
                if (dirNames[i] == "/") {
                    if (i == 0 || (i == (dirNames.size() - 1))) {
                        dirPath += "/";
                    }
                } else {
                    dirPath += dirNames[i];
                    dirPath += "/";
                }
            }
            if (Directory::exists(CSTR(dirPath))) {
                if (!Directory::isEmpty(
                        CSTR(dirPath))) { /* 先删除该目录下的所有子目录和文件 */
                    Directory dir;
                    dir.enterDir(CSTR(dirPath));
                    vector<string> allFiles = dir.getFileList();
                    for (auto i = 0; i < allFiles.size(); i++) {
                        if (allFiles[i] == "." || allFiles[i] == "..") {
                            continue;
                        } else {
                            string fileName = dirPath + allFiles[i];
                            if (Directory::exists(CSTR(fileName))) {
                                if (!removeDir(CSTR(fileName), true)) {
                                    TRACE_ERR(
                                        "Directory::removeDir,del1 dir(%s) "
                                        "error:%s.",
                                        CSTR(fileName), ERRSTR);
                                    return false;
                                }
                            } else {
                                if (!File::removeFile(CSTR(fileName))) {
                                    TRACE_ERR(
                                        "Directory::removeDir,del1 file(%s) "
                                        "error:%s.",
                                        CSTR(fileName), ERRSTR);
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
            return removeDir(CSTR(dirPath)); /* 删除目录 */
        }
        return false;
    } else {
        if (remove(CSTR(dirName)) != 0) {
            TRACE_ERR("Directory::removeDir(%s) error:%s.", CSTR(dirName),
                      ERRSTR);
            return false;
        }
        return true;
    }
}

vector<string> Directory::getFileList(const string& dirName,
                                      const string& regexp) {
    vector<string> fileVector;
    std::string fileName;
    DIR* dirp = opendir(CSTR(dirName));
    if (nullptr == dirp) {
        return fileVector;
    }
    struct dirent* dp;
    RegExp reg;
    while ((dp = readdir(dirp)) != nullptr) {
        fileName = std::string(dp->d_name);
        if (regexp.empty() || reg.match(regexp, fileName)) {
            fileVector.emplace_back(fileName);
        }
    }
    closedir(dirp);
    if (fileVector.size() > 1) {
        sort(fileVector.begin(), fileVector.end());
    }
    return fileVector;
}

bool Directory::enterDir(const string& dirName) {
    string resolved;
    if (dirName.empty()) {
        return false;
    }
    if (dirName[0] != '/') {
        resolved = m_currentDir + "/" + dirName;
    } else {
        resolved = dirName;
    }
    DIR* dirp;
    dirp = opendir(CSTR(resolved));
    if (nullptr == dirp) {
        return false;
    }
    closedir(dirp);
    char buf[1024] = {0};
    if (nullptr == realpath(CSTR(resolved), buf)) {
        return false;
    }
    m_currentDir = string(buf);
    return true;
}

string Directory::currentDir() { return m_currentDir; }

vector<string> Directory::getFileList() {
    vector<string> fileVector;
    std::string fileName;
    DIR* dirp = opendir(CSTR(m_currentDir));
    if (nullptr == dirp) {
        return fileVector;
    }
    struct dirent* dp;
    while ((dp = readdir(dirp)) != nullptr) {
        fileName = std::string(dp->d_name);
        fileVector.emplace_back(fileName);
    }
    closedir(dirp);
    if (fileVector.size() > 1) {
        sort(fileVector.begin(), fileVector.end());
    }
    return fileVector;
}
}  // namespace zemb
