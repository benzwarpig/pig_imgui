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
#ifndef __ZEMB_CODEC_UTIL_H__
#define __ZEMB_CODEC_UTIL_H__
#include <string>

#include "BaseType.h"
/**
 * @file CodecUtil.h
 * @brief 编解码器头文件
 */
namespace zemb {
/**
 * @class Base64
 * @brief BASE64编解码器
 */
class Base64 {
    DECL_CLASSNAME(Base64)

public:
    Base64() {}
    virtual ~Base64() {}
    /**
     * @brief base64编码
     * @param binDataIn 输入数据
     * @param b64DataOut 输出数据
     * @return true 编码成功
     * @return false 编码失败
     */
    bool encode(const std::string& binDataIn, std::string* b64DataOut);
    /**
     * @brief base64解码
     * @param b64DataIn 输入数据
     * @param binDataOut 输出数据
     * @return true 解码成功
     * @return false 解码失败
     */
    bool decode(const std::string& b64DataIn, std::string* binDataOut);
    /**
     * @brief base64编码(URL安全)
     * @param binDataIn 输入数据
     * @param b64DataOut 输出数据
     * @return true 编码成功
     * @return false 编码失败
     */
    bool encodeUrlSafe(const std::string& binDataIn, std::string* b64DataOut);
    /**
     * @brief base64解码(URL安全)
     * @param b64DataIn 输入数据
     * @param binDataOut 输出数据
     * @return true 解码成功
     * @return false 解码失败
     */
    bool decodeUrlSafe(const std::string& b64DataIn, std::string* binDataOut);

private:
    char findIndex(const char* str, char c);
};

}  // namespace zemb
#endif
