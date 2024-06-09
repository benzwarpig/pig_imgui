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
#include "StrUtil.h"

#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "FileUtil.h"
#include "Tracer.h"

namespace zemb {
std::string StrUtil::toLower(const std::string& str) {
    std::string rc = "";
    const char* pstr = CSTR(str);
    int i = 0;
    char ch = *(pstr + i);
    while (ch != 0) {
        rc.append(1, static_cast<char>(tolower(ch)));
        ch = *(pstr + (++i));
    }
    return rc;
}

std::string StrUtil::toUpper(const std::string& str) {
    std::string rc = "";
    const char* pstr = CSTR(str);
    int i = 0;
    char ch = *(pstr + i);
    while (ch != 0) {
        rc.append(1, static_cast<char>(toupper(ch)));
        ch = *(pstr + (++i));
    }
    return rc;
}

string StrUtil::format(const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    char buf[1024] = {0};
    vsnprintf(buf, sizeof(buf), fmt, argp);
    va_end(argp);
    return std::string(buf);
}

int StrUtil::compare(const std::string& str1, const std::string& str2) {
    size_t p1 = 0;
    size_t p2 = 0;
    while (p1 < str1.size() && p2 < str2.size() && str1[p1] == str2[p2]) {
        p1++;
        p2++;
    }
    if (p1 == str1.size()) {
        p1 = 0;
    } else {
        p1 = (uint8)str1[p1];
    }
    if (p2 == str2.size()) {
        p2 = 0;
    } else {
        p2 = (uint8)str2[p2];
    }
    return p1 - p2;
}

std::string StrUtil::replaceString(const string& source, const string& str,
                                   const string& rep) {
    std::string result = "";
    std::size_t pos = 0;
    while (1) {
        std::size_t findPos = source.find(str, pos);
        if (findPos != std::string::npos) {
            result.append(source, pos, findPos - pos);
            result.append(rep);
            pos = findPos + str.size();
        } else {
            result.append(source, pos, source.size() - pos);
            break;
        }
    }
    return result;
}

std::string StrUtil::suffix(const string& source, const string& suffixFlag) {
    auto startPos = source.rfind(suffixFlag);
    if (startPos == string::npos) {
        return "";
    }
    std::string sfx = source.substr(startPos);
    if (sfx.find("/") != string::npos) {
        return "";
    }
    return sfx;
}

std::string StrUtil::findString(const string& source, const string& start,
                                const string& end) {
    std::string restStr;
    std::size_t startPos, endPos;
    /* 找出匹配关键字 */
    startPos = source.find(start, 0);
    if (startPos == string::npos) {
        TRACE_DBG("StrUtil::findString,no start[%s]...", CSTR(start));
        return "";
    }

    endPos = source.find(end, 1);
    if (endPos == string::npos) {
        TRACE_DBG("StrUtil::findString,no end[%s]...", CSTR(end));
        return "";
    }

    return source.substr(startPos, endPos - startPos + end.size());
}

string StrUtil::findString(const string& source, const string& pattern,
                           const string& before, const string& after) {
    std::string restStr;
    std::size_t startPos, endPos;
    /* 找出匹配关键字 */
    startPos = source.find(pattern, 0);
    if (startPos == string::npos) {
        TRACE_DBG("StrUtil::findString,no pattern[%s]...", CSTR(pattern));
        return "";
    }

    /* 从匹配关键字开始查找字符串 */
    restStr = source.substr(startPos);
    startPos = restStr.find(before, 0);
    if (startPos == string::npos) {
        TRACE_DBG("StrUtil::findString,no before[%s]...", CSTR(before));
        return "";
    }
    /* 跳过before字串 */
    startPos += before.size();
    endPos = restStr.find(after, startPos);
    if (endPos == string::npos) {
        TRACE_DBG("StrUtil::findString,no after[%s]...", CSTR(after));
        return "";
    }
    /* 截取before与after之间的字串 */
    return restStr.substr(startPos, endPos - startPos);
}

string StrUtil::trimHeadWith(const string& source, const string& trimch) {
    std::string result;
    int len = source.size();
    int trimLen = trimch.size();
    int i = 0;
    while (i < len) {
        int j;
        for (j = 0; j < trimLen; j++) {
            if (source[i] == trimch[j]) {
                break;
            }
        }
        if (j == trimLen) {
            result = source.substr(i);
            break;
        }
        i++;
    }
    return result;
}

std::string StrUtil::trimTailBlank(const string& source, bool trimHead,
                                   bool trimTail) {
    std::string result;
    if (trimHead) {
        int len = source.size();
        for (auto i = 0; i < len; i++) {
            if (source[i] > 32) {
                result = source.substr(i);
                break;
            }
        }
    }
    if (trimTail) {
        int len = result.size();
        for (auto i = 0; i < len; i++) {
            if (result[len - 1 - i] > 32) {
                result = result.substr(0, len - i);
                break;
            }
        }
    }
    return result;
}

std::string StrUtil::trimAllBlank(const string& source) {
    int i = 0;
    int len = source.size();
    std::string result = "";
    while (i < len) {
        if (source[i] > 32) {
            result.append(1, source[i]);
        }
        i++;
    }
    return result;
}

string StrUtil::trimHeadUnvisible(const string& source) {
    int i = 0;
    int len = source.size();
    std::string result = "";
    while (i < len) {
        if (source[i] > 32) {
            result.append(1, source[i]);
        } else {
            break;
        }
        i++;
    }
    return result;
}

vector<string> StrUtil::splitString(const string& source,
                                    const string& splitFlag, bool withFlag) {
    vector<string> vecResult;
    if (source.empty()) {
        return vecResult;
    }
    if (splitFlag.empty()) {
        vecResult.push_back(source);
        return vecResult;
    }
    std::size_t startPos = 0;
    std::size_t findPos = source.find(splitFlag, 0);
    while (findPos != string::npos) {
        if (findPos != startPos) {
            vecResult.emplace_back(source.substr(startPos, findPos - startPos));
        }
        if (withFlag) {
            vecResult.emplace_back(source.substr(findPos, splitFlag.size()));
        }
        startPos = findPos + splitFlag.size();
        findPos = source.find(splitFlag, startPos);
    }
    if (startPos < source.size()) {
        vecResult.emplace_back(
            source.substr(startPos, source.size() - startPos));
    }
    return vecResult;
}

vector<string> StrUtil::cutString(const string& source, const string& startFlag,
                                  const string& endFlag,
                                  const string& cutFlag) {
    vector<string> vecResult;
    std::size_t startPos = source.find(startFlag);
    std::size_t endPos = source.find(endFlag);
    if (source.empty() || startFlag.empty() || endFlag.empty() ||
        cutFlag.empty() || startPos == string::npos || endPos == string::npos) {
        return vecResult;
    }
    startPos += startFlag.length();
    if (endPos <= startPos) {
        return vecResult;
    }

    std::size_t searchPos;
    while (((endPos != string::npos) && (endPos > startPos)) ||
           (endPos == string::npos)) {
        searchPos = source.find(cutFlag, startPos);
        if (searchPos == string::npos) {
            if (endPos != string::npos)
                vecResult.emplace_back(
                    source.substr(startPos, endPos - startPos));
            else
                vecResult.emplace_back(source.substr(startPos));
            break;
        }
        vecResult.emplace_back(source.substr(startPos, searchPos - startPos));
        startPos = searchPos + cutFlag.length();
    }
    return vecResult;
}

int StrUtil::patternCount(const string& source, const string& pattern) {
    std::size_t atFind = 0;
    int i = 0;
    if (source.empty() || pattern.empty()) {
        return 0;
    }
    for (;;) {
        atFind = source.find(pattern, atFind);
        if (atFind != string::npos) {
            atFind += pattern.size();
            i++;
        } else {
            break;
        }
    }
    return i;
}

int StrUtil::ctoi(char ch) {
    if (ch >= 'a' && ch <= 'f') {
        return (ch - 'a' + 0x0A);
    } else if (ch >= 'A' && ch <= 'F') {
        return (ch - 'A' + 0x0A);
    } else if (ch >= '0' && ch <= '9') {
        return (ch - '0');
    } else {
        return -1;
    }
}

char StrUtil::itoc(int val) {
    if (val >= 0 && val <= 9) {
        return (val + '0');
    } else if (val >= 0x0A && val <= 0x0F) {
        return (val - 0x0A + 'A');
    } else {
        return 0;
    }
}

int StrUtil::htoi(const std::string& hexStr) {
    int result = 0;
    const char* pHex = CSTR(hexStr);
    while ((*pHex) != '\0') {
        result = (result << 4) + StrUtil::ctoi(*pHex);
        pHex++;
    }
    return result;
}

int StrUtil::hexBinarize(const std::string& hexStr, char* buffer, int len) {
    int i;
    if (hexStr.empty() || nullptr == buffer) {
        TRACE_ERR("StrUtil::hexBinarize parameter error.");
        return RC_ERROR;
    }
    if (len < hexStr.size() / 2) {
        TRACE_ERR("StrUtil::hexBinarize buf len(%d) error.", len);
        return RC_ERROR;
    }

    for (i = 0; i < hexStr.size() / 2; i++) {
        sint8 a, b;
        a = StrUtil::ctoi(hexStr[2 * i]);
        b = StrUtil::ctoi(hexStr[2 * i + 1]);
        if (-1 == a || -1 == b) {
            TRACE_ERR("StrUtil::hexBinarize invalid string.");
            return RC_ERROR;
        }
        buffer[i] = (a << 4) + b;
    }
    return i;
}

std::string StrUtil::hexVisualize(const std::string& codestr) {
    return hexVisualize(codestr.data(), codestr.size());
}

std::string StrUtil::hexVisualize(const char* buffer, int len) {
    if (buffer == nullptr || len <= 0) {
        TRACE_ERR("StrUtil::hexVisualize parameter error.");
        return "";
    }

    std::string hexStr = "";
    for (auto i = 0; i < len; i++) {
        char hex[3] = {0};
        hex[0] = StrUtil::itoc((buffer[i] >> 4) & 0x0F);
        hex[1] = StrUtil::itoc(buffer[i] & 0x0F);
        if (0 == hex[0] || 0 == hex[1]) {
            TRACE_ERR("StrUtil::hexVisualize error.");
            return "";
        }
        hexStr += hex;
        hexStr += " ";
    }
    return hexStr;
}

bool StrUtil::isHexString(const std::string& hexStr) {
    const char* pstr = CSTR(hexStr);
    int len = strlen(pstr);
    int i = 0;
    while (i < len) {
        if ((*(pstr + i) >= '0' && *(pstr + i) <= '9') ||
            (*(pstr + i) >= 'A' && *(pstr + i) <= 'F') ||
            (*(pstr + i) >= 'a' && *(pstr + i) <= 'f')) {
            i++;
            continue;
        } else {
            return false;
        }
    }
    return true;
}
bool StrUtil::isIntString(const std::string& intStr) {
    const char* pstr = CSTR(intStr);
    auto len = strlen(pstr);
    size_t i = 0;
    while (i < len) {
        if (*(pstr + i) >= '0' && *(pstr + i) <= '9') {
            i++;
            continue;
        } else {
            return false;
        }
    }
    return true;
}

/* 在[mem,mem+n)区间查找str,找到返回位置 */
char* StrUtil::memString(char* mem, int n, char* str, int size) {
    char* find = reinterpret_cast<char*>(memchr(mem, str[0], n));
    if (find) {
        char* end = find + size;
        if (end <= mem + n) {
            if (memcmp(find, str, size) == 0) {
                return find;
            } else {
                return memString(find + 1, mem + n - find - 1, str, size);
            }
        }
    }
    return nullptr;
}

/******************************************************************************
 * parseNextSection  解析下一个字段
 * source    : 要解析的字符串
 * startPos  : 开始解析的位置
 * endflag   : 下一个结束标识
 * sectionLen: 解析出来的字段长度(只有在返回值非NULL的情况下才有效)
 * return    : 返回查找字段的首地址,如果没有查找到字段,则返回NULL
 *****************************************************************************/
char* StrUtil::parseNextSection(const char* source, uint32* startPos,
                                const char* endflag, uint32* sectionLen) {
    unsigned int len = 0;
    char* pstart =
        reinterpret_cast<char*>(const_cast<char*>(source + *startPos));
    char* ptr = pstart;
    while ((*ptr != *endflag) && (*ptr != 0)) {
        ptr++;
    }
    len = ptr - pstart;
    if (len == 0) {
        return nullptr;
    }
    *sectionLen = len;
    if (*ptr == 0) {
        *startPos += len;
    } else if (*ptr == *endflag) {
        *startPos = *startPos + len + 1;
    }
    return pstart;
}
/******************************************************************************
 * parseNextString  解析下一个字符串
 * source   : 要解析的字符串
 * startPos : 开始解析的位置
 * endflag  : 下一个结束标识
 * strbuf   : 解析出来的字符串缓存
 * buflen   : 字符串缓存长度
 * return   : 成功返回true,失败返回false
 *****************************************************************************/
bool StrUtil::parseNextString(const char* source, uint32* startPos,
                              const char* endflag, char* strbuf,
                              uint32 buflen) {
    unsigned int sectionLen = 0;
    char* sectionStart = nullptr;
    sectionStart = parseNextSection(source, startPos, endflag, &sectionLen);
    if (sectionStart == nullptr) {
        return false;
    }
    if (sectionLen > 0 && sectionLen < buflen) {
        memset(strbuf, 0, buflen);
        strncpy(strbuf, sectionStart, sectionLen);
        return true;
    }
    return false;
}
/******************************************************************************
 * parseNextInt  解析下一个整数
 * source   : 要解析的字符串
 * startPos : 开始解析的位置
 * endflag  : 下一个结束标识
 * result   : 解析出来的整数
 * return   : 成功返回true,失败返回false
 *****************************************************************************/
bool StrUtil::parseNextInt(const char* source, uint32* startPos,
                           const char* endflag, sint32* result) {
    char section[32] = {0};
    if (!parseNextString(source, startPos, endflag, section, sizeof(section))) {
        return false;
    }
    *result = atoi(section);
    return true;
}

}  // namespace zemb
