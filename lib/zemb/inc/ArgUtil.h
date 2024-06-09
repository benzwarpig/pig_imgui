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
#ifndef __ZEMB_ARG_UTIL_H__
#define __ZEMB_ARG_UTIL_H__
#include <getopt.h>

#include <map>
#include <string>
#include <vector>

#include "BaseType.h"

/**
 * @brief 参数工具类头文件
 * @file ArgUtil.h
 */
namespace zemb {
/**
 *  @file   ArgUtil.h
 *  @class  ArgOption
 *  @brief  参数解析类(自己实现的不依赖于getopt的解析器)
 */
class ArgOption {
    DECL_CLASSNAME(ArgOption)

public:
    enum OptionFlag {
        OPTION_0 = 0, /* 0个参数值 */
        OPTION_1,     /* 1个参数值 */
        OPTION_N      /* N个参数值(大于1) */
    };

public:
    ArgOption();
    ~ArgOption();
    bool parseArgs(int argc, char** argv);
    /**
     * @brief 判断参数是否存在
     * @param option 参数名称
     * @return int 参数值个数(-1:option不存在)
     */
    int hasOption(const std::string& option);
    /**
     * @brief 获取参数值
     * @param option 参数名称
     * @return std::vector<std::string> 返回参数值列表
     * @note 当option为空时,返回的是从argv[1]开始的所有参数列表
     */
    std::vector<std::string> getValue(const std::string& option = "");

private:
    std::map<std::string, std::vector<std::string>> m_optionMap;
    std::vector<std::string> m_argVect;
};

/**
 *  @file   ArgUtil.h
 *  @class  InputReader
 *  @brief  输入获取类
 */
class InputReader {
    DECL_CLASSNAME(InputReader)

public:
    InputReader();
    virtual ~InputReader();
    /**
     * @brief 等待输入(直至Enter按下才会返回)
     * @return 返回实例引用
     */
    InputReader& waitInput();
    /**
     * @brief 检查输入字符串是否相同
     * @param str
     * @return true 匹配
     * @return false 不匹配
     */
    bool isString(const std::string& str);
    /**
     * @brief 获取输入字符串的长度
     * @return int 字符串长度
     */
    int size();
    /**
     * @brief 将输入字符串转换为整数
     * @return int 字符串字面值
     */
    int asInt();
    /**
     * @brief 将输入字符串转换为浮点数
     * @return float 字符串字面值
     */
    float asFloat();
    /**
     * @brief 返回输入字符串
     * @return std::string 输入字符串
     */
    std::string asString();
    /**
     * @brief 返回C语言格式的输入字符串
     * @return const char* 输入字符串
     */
    const char* asCString();

private:
    std::string m_inputStr{""};
};
}  // namespace zemb
#endif
