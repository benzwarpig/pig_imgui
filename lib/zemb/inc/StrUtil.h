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
#ifndef __ZEMB_STR_UTIL_H__
#define __ZEMB_STR_UTIL_H__

#include <iostream>
#include <string>
#include <vector>

#include "BaseType.h"
/**
 *  @file   StrUtil.h
 *  @brief  字符串工具
 */
namespace zemb {
/**
 *  @class  StrUtil
 *  @brief  字符串工具类
 */
class StrUtil {
    DECL_CLASSNAME(StrUtil)

public:
    /**
     *  @brief  将字符串转换为小写
     *  @param  str 源字符串
     *  @return 返回转换好的字符串
     */
    static std::string toLower(const std::string& str);
    /**
     *  @brief  将字符串转换为大写
     *  @param  str 源字符串
     *  @return 返回转换好的字符串
     */
    static std::string toUpper(const std::string& str);
    /**
     *  @brief  字符串格式化
     *  @param  fmt 格式化字符串
     *  @return 返回转换好的字符串
     *  @note   将C风格字符串格式化为string,最长1024字节
     */
    static std::string format(const char* fmt, ...);
    /**
     * @brief 比较字符串大小
     * @param s1
     * @param s2
     * @return int 返回s1与s2第一个不同的字符的ASCII码差值(S1-S2)
     * @note
     * 由于strcmp在不同的平台下返回值不同(不一定返回ASCII码的差值),因此自己实现该功能
     */
    static int compare(const std::string& s1, const std::string& s2);
    /**
     *  @brief  用rep替换source中的str
     *  @param  source 源字串
     *  @param  str 被替换字符串
     *  @param  rep 替换字符串
     *  @return 返回新字符串
     */
    static std::string replaceString(const std::string& source,
                                     const std::string& str,
                                     const std::string& rep);
    /**
     * @brief 获取后缀
     * @param source
     * @param suffixFlag 后缀标识
     * @return string 返回后缀名称(包含后缀标识)
     * @note 如suffix("/home/user/log.txt","."),将返回".txt"
     */
    static std::string suffix(const std::string& source,
                              const std::string& suffixFlag);
    /**
     *  @brief  查找start开始与end结束的字符串
     *  @param  source 源字串
     *  @param  start 开始字串
     *  @param  end 结束字串
     *  @return 返回找到的字符串
     */
    static std::string findString(const std::string& source,
                                  const std::string& start,
                                  const std::string& end);
    /**
     *  @brief  查找匹配字段
     *  @param  source 源字串
     *  @param  pattern 匹配字串
     *  @param  before 匹配字串之前的字串
     *  @param  after 匹配字串之后的字串
     *  @return 返回找到的字符串
     */
    static std::string findString(const std::string& source,
                                  const std::string& pattern,
                                  const std::string& before,
                                  const std::string& after);
    /**
     *  @brief  从字符串头部去除trimch包含的字符,直至遇到非trimch字符为止
     *  @param  source 源字串
     *  @return 返回trim后得到的字符串
     *  @note   例:result = trimHeadWith(" 0001234","0 ");//result="1234"
     */
    static std::string trimHeadWith(const std::string& source,
                                    const std::string& trimch);
    /**
     *  @brief  去除字符串首尾两端的不可见字符
     *  @param  source 源字串
     *  @param  trimHead 是否去除头部,默认为true
     *  @param  trimTail 是否去除尾部,默认为true
     *  @return 返回trim后得到的字符串
     */
    static std::string trimTailBlank(const std::string& source,
                                     bool trimHead = true,
                                     bool trimTail = true);
    /**
     *  @brief  去除字符串中所有不可见字符
     *  @param  source 源字串
     *  @return 返回trim后得到的字符串
     */
    static std::string trimAllBlank(const std::string& source);
    /**
     *  @brief  从头开始获取可见字符，到第一个不可见字符结束；
     *  @param  source 源字串
     *  @return 返回截取的字符串
     *  @note   例:result = trimHeadUnvisible("000123 4");//result="000123"
     */
    static std::string trimHeadUnvisible(const std::string& source);
    /**
     *  @brief  拆分字符串
     *  @param  source 源字串
     *  @param  splitFlag 拆分符
     *  @param  withFlag 是否包含拆分符
     *  @return 返回拆分后的字串数组
     *  @note   以splitFlag为标识拆分整个字串
     */
    static vector<std::string> splitString(const std::string& source,
                                           const std::string& splitFlag,
                                           bool withFlag = false);
    /**
     *  @brief  分割字符串
     *  @param  source 源字串
     *  @return 返回分割后的字串数组(不包括分割符)
     *  @note   分割从startFlag后到endFlag之前的字串,分割符为cutFlag,例：
     *          s="+ABC:1,2,3,,,6,7\r"  ==> cutString(s,"+ABC:","\r",",")
     *          返回结果:["1","2","3","","","6","7"]
     */
    static vector<std::string> cutString(const std::string& source,
                                         const std::string& startFlag,
                                         const std::string& stopFlag,
                                         const std::string& cutFlag);
    /**
     *  @brief  获取匹配次数
     *  @param  source 源字串
     *  @param  pattern 匹配字串
     *  @return 返回匹配次数
     *  @note   none
     */
    static int patternCount(const string& source, const string& pattern);
    /**
     *  @brief  ASCII码转换为数字
     *  @param  ch ASCII码字符
     *  @return ch对应的数字
     *  @note
     * 字符'0'~'9'转换后为数字0~9,'a'~'f'或'A'~'F'转换后为10~16,其余返回值为-1
     */
    static int ctoi(char ch);
    /**
     *  @brief  数字转换为ASCII码
     *  @param  val 数字
     *  @return 数字对应的ASCII码字符
     *  @note
     * 数字0~9转成字符'0'~'9',数字10~16转成'A'~'F',其余返回值为0,为字符串结束符
     */
    static char itoc(int val);
    /**
     *  @brief  16进制字符串转换为数字
     *  @param  hexStr 16进制字符串
     *  @return 返回计算结果
     *  @note  "80FF" -> 33023
     */
    static int htoi(const std::string& hexStr);
    /**
     *  @brief  16进制字符串转为二进制字节
     *  @param  hexStr 待转换16进制字符串
     *  @param  bytesBuf 转换好的编码缓存
     *  @param  len 缓存大小
     *  @return 成功返回转换结果的长度,失败返回STATUS_ERROR
     *  @note   "0041001F"  ==>[0x00][0x41][0x00][0x1F]
     */
    static int hexBinarize(const string& hexStr, char* bytesBuf, int len);
    /**
     * @brief 编码转为可视字符串
     * @param codestr
     * @return std::string
     */
    static std::string hexVisualize(const std::string& codestr);
    /**
     *  @brief  编码转为可视字符串
     *  @param  codebuf 待转换的编码缓存
     *  @param  len 缓存大小
     *  @return 成功返回转换后的字符串,失败返回空字符串
     *  @note   [0x00][0x41][0x00][0x1F]==>"0041001F"
     */
    static std::string hexVisualize(const char* codebuf, int len);
    /**
     * @brief 判断是否是16进制字符串
     * @param hexString
     * @return true
     * @return false
     */
    static bool isHexString(const std::string& hexString);
    /**
     * @brief 判断是否是10进制整数字符串
     * @param intString
     * @return true
     * @return false
     */
    static bool isIntString(const std::string& intString);
    /**
     * @brief 搜索内存字符
     * @param mem
     * @param n
     * @param str
     * @param size
     * @return char*
     * @note 在[mem,mem+n)区间搜索字符串str,返回地址,未找到返回NULL
     */
    static char* memString(char* mem, int n, char* str, int size);
    /**
     * @brief 解析下一个字段
     * @param source
     * @param startPos
     * @param endflag
     * @param section_len
     * @return char*
     * @note C风格的解析函数(一般用来解析固定分隔符的字符串,如AT指令)
     */
    char* parseNextSection(const char* source, uint32* startPos,
                           const char* endflag, uint32* section_len);
    /**
     * @brief 解析下一个字符串字段
     * @param source
     * @param startPos
     * @param endflag
     * @param strbuf
     * @param buflen
     * @return true
     * @return false
     * @note C风格的解析函数(一般用来解析固定分隔符的字符串,如AT指令)
     */
    bool parseNextString(const char* source, uint32* startPos,
                         const char* endflag, char* strbuf, uint32 buflen);
    /**
     * @brief 解析下一个整数字段
     * @param source
     * @param startPos
     * @param endflag
     * @param result
     * @return true
     * @return false
     * @note C风格的解析函数(一般用来解析固定分隔符的字符串,如AT指令)
     */
    bool parseNextInt(const char* source, uint32* startPos, const char* endflag,
                      sint32* result);
};
}  // namespace zemb
#endif
