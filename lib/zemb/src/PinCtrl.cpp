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
#include "PinCtrl.h"

#include <string.h>

#include "FileUtil.h"
#include "StrUtil.h"
#include "Tracer.h"
namespace zemb {

PinCtrl::PinCtrl(const std::string& group, int pin, const std::string& base)
    : m_group(group), m_pin(pin), m_groupBase(base) {}

PinCtrl::~PinCtrl() {
    if (m_pinID >= 0) {
        File unexportFile;
        if (!unexportFile.open(m_unexportFile, IO_MODE_WR_ONLY)) {
            TRACE_ERR_CLASS("cannot export, pin [%s:%d] !", CSTR(m_group),
                            m_pin);
            return;
        }
        std::string data = StrUtil::format("%d", m_pinID);
        if (unexportFile.writeData(data.data(), data.size()) != data.size()) {
            TRACE_ERR_CLASS("export error, pin [%s:%d] !", CSTR(m_group),
                            m_pin);
        }
    }
}

/******************************************************************************************
 * PinGroup的label和base信息可在/sys/class/gpio/gpiochipxx中查看,每种平台可能都不一样.
 * 可通过shell查看:
 * $ cd /sys/class/gpio; for i in gpiochip*; do echo `cat $i/label`: `cat
 *$i/base`; done
 *****************************************************************************************/
int PinCtrl::getPinID(const std::string& group, int pin) {
    if (group.size() == m_groupBase.size()) {
        int grp = StrUtil::compare(group, m_groupBase);
        if (grp >= 0) {
            return 32 * grp + pin;
        }
    }
    return -1;
}

bool PinCtrl::isRequested() { return (m_pinID < 0) ? false : true; }

bool PinCtrl::request(const PinDir& dir) {
    m_pinID = getPinID(m_group, m_pin);
    if (m_pinID < 0) {
        TRACE_ERR_CLASS("get pin id error, pin [%s:%d] !", CSTR(m_group),
                        m_pin);
        return false;
    }
    m_exportFile = "/sys/class/gpio/export";
    m_unexportFile = "/sys/class/gpio/unexport";
    m_gpioDir = "/sys/class/gpio/gpio";
    m_gpioDir += StrUtil::format("%d", m_pinID);
    m_directionFile = m_gpioDir + "/direction";
    m_valueFile = m_gpioDir + "/value";

    /* 导出gpio */
    if (!Directory::exists(m_gpioDir)) {
        File exportFile;
        if (!exportFile.open(m_exportFile, IO_MODE_WR_ONLY)) {
            TRACE_ERR_CLASS("cannot open export file, pin [%s:%d] ",
                            CSTR(m_group), m_pin);
            return false;
        }
        std::string data = StrUtil::format("%d", m_pinID);
        if (exportFile.writeData(data.data(), data.size()) != data.size()) {
            TRACE_ERR_CLASS("cannot export, pin [%s:%d] !", CSTR(m_group),
                            m_pin);
            return false;
        }
    }

    /* 判断是否已导出 */
    if (File::exists(m_directionFile) && File::exists(m_valueFile)) {
        File directionFile;
        if (!directionFile.open(m_directionFile, IO_MODE_WR_ONLY)) {
            TRACE_ERR_CLASS("cannot open direction file, pin [%s:%d] ",
                            CSTR(m_group), m_pin);
            return false;
        }
        std::string data;
        switch (dir) {
            case PinDir::IN:
                data = "in";
                break;
            case PinDir::OUT:
                data = "out";
                break;
            default:
                m_pinID = -1;
                return false;
        }
        if (directionFile.writeData(data.data(), data.size()) != data.size()) {
            TRACE_ERR_CLASS("cannot set direction, pin [%s:%d]", CSTR(m_group),
                            m_pin);
            return false;
        }
    } else {
        TRACE_ERR_CLASS("export error, pin [%s:%d]", CSTR(m_group), m_pin);
        return false;
    }
    return true;
}

bool PinCtrl::set(const PinVal& val) {
    if (m_pin < 0) {
        TRACE_ERR_CLASS("pin not request!");
        return false;
    }
    File valueFile;
    if (!valueFile.open(m_valueFile, IO_MODE_WR_ONLY)) {
        TRACE_ERR_CLASS("cannot open value file, pin [%s:%d]", CSTR(m_group),
                        m_pin);
        return false;
    }
    std::string data;
    switch (val) {
        case PinVal::LOW:
            data = "0";
            break;
        case PinVal::HIGH:
            data = "1";
            break;
        default:
            TRACE_ERR_CLASS("invalid param, pin [%s:%d]", CSTR(m_group), m_pin);
            return false;
    }
    if (valueFile.writeData(data.data(), data.size()) != data.size()) {
        TRACE_ERR_CLASS("cannot set value, pin [%s:%d]", CSTR(m_group), m_pin);
        return false;
    }
    return true;
}

bool PinCtrl::get(PinVal* val) {
    if (m_pin < 0) {
        return false;
    }
    File valueFile;
    if (!valueFile.open(m_valueFile, IO_MODE_RDWR_ONLY)) {
        TRACE_ERR_CLASS("cannot open value file, pin [%s:%d]", CSTR(m_group),
                        m_pin);
        return false;
    }

    char data;
    if (valueFile.readData(&data, 1) != 1) {
        return false;
    }
    switch (data) {
        case '0':
            *val = PinVal::LOW;
            break;
        case '1':
            *val = PinVal::HIGH;
            break;
        default:
            TRACE_ERR_CLASS("invalid value: %c, pin [%s:%d]", data,
                            CSTR(m_group), m_pin);
            return false;
    }
    return true;
}
}  // namespace zemb
