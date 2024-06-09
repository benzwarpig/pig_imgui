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
#include "CodecUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BaseType.h"
#include "StrUtil.h"

namespace zemb {

/* base64编码表 */
static const char* s_base64Table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char Base64::findIndex(const char* str, char c) {
    const char* p = strchr(str, c);
    if (nullptr == p) {
        return -1;
    }
    return static_cast<char>(p - str);
}

bool Base64::encode(const std::string& binDataIn, std::string* b64DataOut) {
    b64DataOut->clear();
    const char* base64Table = s_base64Table;
    uint32 srcLen = binDataIn.size();
    if (srcLen == 0) {
        return false;
    }
    uint32 i, j;
    for (i = 0, j = 0; i < srcLen / 3; i++) {
        b64DataOut->append(CH2STR(
            base64Table[(binDataIn[j] >> 2) & 0x3f])); /* 取第一字符前6bit */
        b64DataOut->append(CH2STR(
            base64Table[((binDataIn[j] << 4) & 0x30) |
                        ((binDataIn[j + 1] >> 4) &
                         0x0f)]));  // 第一字符的后2bit与第二字符的前4位进行合并
        b64DataOut->append(CH2STR(
            base64Table
                [((binDataIn[j + 1] << 2) & 0x3c) |
                 ((binDataIn[j + 2] >> 6) &
                  0x03)]));  // 将第二字符的后4bit与第三字符的前2bit组合并
        b64DataOut->append(CH2STR(
            base64Table[binDataIn[j + 2] & 0x3f])); /* 取第三字符的后6bit */
        j += 3;
    }

    /* 非3的整数倍补“=” */
    if ((srcLen % 3) == 1) {
        b64DataOut->append(CH2STR(base64Table[(binDataIn[j] >> 2) & 0x3f]));
        b64DataOut->append(
            CH2STR(base64Table[((binDataIn[j] << 4) & 0x30) |
                               ((binDataIn[j + 1] >> 4) & 0x0f)]));
        b64DataOut->append("=");
        b64DataOut->append("=");
    } else if ((srcLen % 3) == 2) {
        b64DataOut->append(CH2STR(base64Table[(binDataIn[j] >> 2) & 0x3f]));
        b64DataOut->append(
            CH2STR(base64Table[((binDataIn[j] << 4) & 0x30) |
                               ((binDataIn[j + 1] >> 4) & 0x0f)]));
        b64DataOut->append(CH2STR(base64Table[(binDataIn[j + 1] << 2) & 0x3c]));
        b64DataOut->append("=");
    }
    return true;
}
bool Base64::decode(const std::string& b64DataIn, std::string* binDataOut) {
    binDataOut->clear();
    const char* base64Table = s_base64Table;
    uint32 srcLen = b64DataIn.size();
    if (srcLen % 4 != 0) {
        return false;
    }
    uint32 i = 0;
    char buf[4] = {0};
    // uint32 destLen = (srcLen>>2)*3-2;
    for (i = 0; i < srcLen; i += 4) {
        /* 四个码译成三个字符 */
        buf[0] = findIndex(base64Table, b64DataIn[i]);
        buf[1] = findIndex(base64Table, b64DataIn[i + 1]);

        binDataOut->append(
            CH2STR(((buf[0] << 2) & 0xfc) | ((buf[1] >> 4) & 0x03)));
        if (b64DataIn[i + 2] == '=') {
            break;
        }
        buf[2] = findIndex(base64Table, b64DataIn[i + 2]);

        binDataOut->append(
            CH2STR(((buf[1] << 4) & 0xf0) | ((buf[2] >> 2) & 0x0f)));
        if (b64DataIn[i + 3] == '=') {
            break;
        }
        buf[3] = findIndex(base64Table, b64DataIn[i + 3]);

        binDataOut->append(CH2STR(((buf[2] << 6) & 0xc0) | (buf[3] & 0x3f)));
    }
    return true;
}

/* "+","/"和"="在url中会被转码,需要替换为"-","_"和"" */
bool Base64::encodeUrlSafe(const std::string& binDataIn,
                           std::string* b64DataOut) {
    if (encode(binDataIn, b64DataOut)) {
        *b64DataOut = StrUtil::replaceString(*b64DataOut, "+", "-");
        *b64DataOut = StrUtil::replaceString(*b64DataOut, "/", "_");
        uint32 idx = b64DataOut->size() - 1;
        for (uint32 i = idx; i >= 0; i--) {
            if (b64DataOut->at(i) != '=')
                break;
            else
                *b64DataOut = b64DataOut->erase(i, 1);
        }
        return true;
    }
    return false;
}

bool Base64::decodeUrlSafe(const std::string& b64DataIn,
                           std::string* binDataOut) {
    std::string b64Data = b64DataIn;
    b64Data = StrUtil::replaceString(b64Data, "-", "+");
    b64Data = StrUtil::replaceString(b64Data, "_", "/");
    uint32 mod4 = b64Data.size() % 4;
    if (mod4 != 0) {
        b64Data.append(4 - mod4, '='); /* 补足"=" */
    }
    return decode(b64Data, binDataOut);
}

}  // namespace zemb
