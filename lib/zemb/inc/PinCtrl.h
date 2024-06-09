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
 * Copyright 2014-2020 @ ShenZhen ,China
 *******************************************************************************/
#ifndef __ZEMB_PINCTRL_H__
#define __ZEMB_PINCTRL_H__
#include <string>

#include "BaseType.h"
namespace zemb {
class PinCtrl {
    DECL_CLASSNAME(PinCtrl)

public:
    enum class PinDir { IN = 0, OUT };
    enum class PinVal {
        LOW = 0,
        HIGH,
    };

public:
    /**
     * @brief 创建GPIO控制器
     * @param group GPIO组名,如"PA","PB","GPIOA","GPIOB"等
     * @param pin PIN脚序号
     * @param base GPIO基址
     */
    PinCtrl(const std::string& group, int pin, const std::string& base = "PA");
    virtual ~PinCtrl();
    bool isRequested();
    bool request(const PinDir& dir);
    bool set(const PinVal& val);
    bool get(PinVal* val);

private:
    int getPinID(const std::string& group, int pin);

private:
    int m_pinID{-1};
    int m_pin{0};
    std::string m_groupBase;
    std::string m_group;
    std::string m_gpioDir;
    std::string m_exportFile;
    std::string m_unexportFile;
    std::string m_directionFile;
    std::string m_valueFile;
};
}  // namespace zemb
#endif
