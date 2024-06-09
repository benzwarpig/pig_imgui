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
#ifndef __ZEMB_REGEXP_H__
#define __ZEMB_REGEXP_H__

#include <iostream>
#include <string>

#include "BaseType.h"
#include "RegExp.h"

/**
 * @file RegExp
 * @brief 正则表达式
 */

/*----------------------------------
  RegExp支持的元字符:
  *     匹配0个或多个在*字符前的字符
  .     匹配任意字符
  ^     匹配行首;后面的字符非
  $     匹配行尾
  []    字符集合
  \     转义符
  <>    精确匹配
  {n}   匹配前面的字符出现n次
  {n,}  匹配前面的字符至少出现n次
  {n,m} 匹配前面的字符出现n-m次
  RegExp支持的扩展元字符:
  \w
 ----------------------------------*/
namespace zemb {
/**
 * @class RegExp
 * @brief 正则表达式类
 */
class RegExp {
    DECL_CLASSNAME(RegExp)

public:
    RegExp();
    ~RegExp();
    /**
     * @brief 正则匹配
     * @param pattern 正则表达式
     * @param source 要匹配的字符串
     * @return true 匹配成功
     * @return false 匹配失败
     */
    bool match(const std::string& pattern, const std::string& source);
    /**
     * @brief 正则匹配
     * @param pattern 正则表达式
     * @param source 要匹配的字符串
     * @param result 匹配结果
     * @param pos 匹配结果的位置
     * @return true 匹配成功
     * @return false 匹配失败
     */
    bool match(const std::string& pattern, const std::string& source,
               std::string* result, int* pos);
    /**
     * @brief 正则匹配
     * @param pattern 正则表达式
     * @param source 要匹配的字符串
     * @param strArray 匹配结果
     * @param posArray 匹配结果对应位置
     * @param maxMatches 最多匹配个数
     * @return true 匹配成功
     * @return false 匹配失败
     */
    bool match(const std::string& pattern, const std::string& source,
               StringArray* strArray, IntArray* posArray, int maxMatches = 1);
};
}  // namespace zemb
#endif
