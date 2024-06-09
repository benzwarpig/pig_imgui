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
#ifndef __ZEMB_COM_UTIL_H__
#define __ZEMB_COM_UTIL_H__

#include <iostream>
#include <string>
#include <vector>

#include "BaseType.h"

/**
 *  @file   ComUtil.h
 *  @brief  通用工具类
 */
namespace zemb {

/**
 * @class ComUtil
 * @brief 通用工具类
 */
class ComUtil {
public:
    /**
     * @brief 判断两个浮点数是否相等
     * @param f1 浮点数1
     * @param f2 浮点数2
     * @param precision 浮点精度
     * @return true 相等
     * @return false 不相等
     */
    static bool equal(float f1, float f2, float precision);
    /**
     * @brief 生成伪随机数
     * @param max 最大值
     * @return int 生成的伪随机数
     */
    static int random(int max);
    /**
     * @brief 生成UUID
     * @return std::string
     */
    static std::string uuid(void);
    /**
     * @brief 判断本机是否是大端字节序
     * @return true 是
     * @return false 不是
     */
    static bool isBigEndian(void);
    /**
     * @brief 16位整数转换为小端字节序
     * @param value 16位整数
     * @return uint16 小端16位整数
     */
    static uint16 host2LitEndian(uint16 value);
    /**
     * @brief 32位整数转换为小端字节序
     * @param value 32位整数
     * @return uint32 小端32位整数
     */
    static uint32 host2LitEndian(uint32 value);
    /**
     * @brief 32位浮点数转换为小端字节序
     * @param value 32位浮点数
     * @return float 小端32位浮点数
     */
    static float host2LitEndian(float value);
    /**
     * @brief 16位整数转换为大端字节序
     * @param value 16位整数
     * @return uint16 大端16位整数
     */
    static uint16 host2BigEndian(uint16 value);
    /**
     * @brief 32位整数转换为大端字节序
     * @param value 32位整数
     * @return uint32 大端32位整数
     */
    static uint32 host2BigEndian(uint32 value);
    /**
     * @brief 32位浮点数转换为大端字节序
     * @param value 32位浮点数
     * @return float 大端32位浮点数
     */
    static float host2BigEndian(float value);
    /**
     * @brief unicode转换位utf-8字符串
     * @param unicode unicode编码
     * @return string utf-8字符串(1~6字节)
     */
    static std::string unicodeOneToUtf8String(uint32 unicode);
    /**
     * @brief utf-8字符串转换位unicode编码
     * @param utf8code utf-8字符串(1~6字节)
     * @return uint32 unicode编码
     */
    static uint32 utf8OneToUnicode(const char* utf8code);
    /**
     * @brief 位反转
     * @param value 值
     * @param bits 要反转的位数(从最低位开始)
     * @return uint32 位反转后的值
     */
    static uint32 bitsReverse(uint32 value, uint8 bits);
    /**
     * @brief 表达式计算(暂未实现)
     * @param expression 表达式
     * @return int 计算结果
     */
    static int eval(const char* expression);
};
}  // namespace zemb
#endif
