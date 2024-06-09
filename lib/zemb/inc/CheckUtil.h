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
 * Copyright 2014~2017 @ ShenZhen ,China
 *******************************************************************************/
#ifndef __ZEMB_CHECK_UTIL_H__
#define __ZEMB_CHECK_UTIL_H__

#include <string>

#include "BaseType.h"
/**
 * @file CheckUtil.h
 * @brief 校验工具
 */
namespace zemb {

/**
 * @class CRCCheck
 * @brief CRC校验
 */
class CRCCheck {
public:
    CRCCheck();
    ~CRCCheck();
    /**
     * @brief CRC16校验(多项式8005)
     * @param content 校验内容
     * @return uint16 校验值
     */
    static uint16 check16CRC8005(const std::string& content);
};

struct MD5Context;
/**
 *  @class  MD5Check
 *  @brief  MD5校验类
 */
class MD5Check {
public:
    MD5Check();
    ~MD5Check();
    /**
     * @brief 获取文件的MD5值
     * @param fileName 文件名
     * @param md5sum 校验和
     * @return true
     * @return false
     */
    bool checkFile(const std::string& fileName, std::string* md5sum);
    /**
     * @brief 获取字符换的MD5值
     * @param srcString 字符串
     * @param md5sum 校验和
     * @return true
     * @return false
     */
    bool checkString(const std::string& srcString, std::string* md5sum);

private:
    void initContext(MD5Context* ctx);
    void calculate(MD5Context* ctx, uint8* data);
    void md5Write(MD5Context* hd, uint8* inbuf, int inlen);
    void md5Final(MD5Context* hd);
};
}  // namespace zemb
#endif
