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
#ifndef __ZEMB_PLUGLET_H__
#define __ZEMB_PLUGLET_H__

#include <map>
#include <string>

#include "BaseType.h"
/**
 * @file Pluglet.h
 * @brief 插件
 */
namespace zemb {
/**
 * @enum PlugletType
 * @brief 插件类型
 */
enum class PlugletType : uint8 {
    Lazy = 0, /* 懒惰式加载符号 */
    Now       /* 立即加载所有符号 */
};

/**
 * @class Pluglet
 * @brief 插件类
 */
class Pluglet {
    DECL_CLASSNAME(Pluglet)

public:
    Pluglet();
    ~Pluglet();
    /**
     * @brief 判断插件是否加载
     * @return true
     * @return false
     */
    bool isOpen();
    /**
     * @brief 打开插件
     * @param plugPath
     * @param type
     * @return true
     * @return false
     */
    bool open(std::string plugPath, PlugletType type);
    /**
     * @brief 加载符号
     * @param symName
     * @return true
     * @return false
     */
    bool putSymbol(std::string symName);
    /**
     * @brief 获取符号
     * @param symName
     * @return void*
     */
    void* getSymbol(std::string symName);
    /**
     * @brief 关闭插件
     * @param void
     */
    void close();

private:
    void* m_plugHandle{nullptr};
    std::map<std::string, void*> m_symbolMap;
};
}  // namespace zemb
#endif
