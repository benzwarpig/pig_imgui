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
#include "Pluglet.h"

#include <dlfcn.h>

#include "FileUtil.h"
#include "Tracer.h"

namespace zemb {
Pluglet::Pluglet() {}

Pluglet::~Pluglet() { close(); }

bool Pluglet::isOpen() { return !!m_plugHandle; }

bool Pluglet::open(std::string plugPath, PlugletType type) {
    int flags = -1;
    if (plugPath.empty() || !File::exists(plugPath)) {
        return false;
    }
    switch (type) {
        case PlugletType::Lazy:
            flags = RTLD_LAZY;
            break;
        case PlugletType::Now:
            flags = RTLD_NOW;
        default:
            return false;
    }
    m_plugHandle = dlopen(CSTR(plugPath), flags);
    if (!m_plugHandle) {
        TRACE_ERR_CLASS("open pluglet error:%s", dlerror());
        return false;
    }
    return true;
}

bool Pluglet::putSymbol(std::string symName) {
    if (!m_plugHandle) {
        return false;
    }
    void* sym = dlsym(m_plugHandle, CSTR(symName));
    if (!sym) {
        return false;
    }
    m_symbolMap.insert(std::make_pair(symName, sym));
    return true;
}

void* Pluglet::getSymbol(std::string symName) {
    auto iter = m_symbolMap.find(symName);
    if (iter != m_symbolMap.end()) {
        return iter->second;
    }
    return nullptr;
}

void Pluglet::close() {
    if (m_plugHandle) {
        dlclose(m_plugHandle);
        m_plugHandle = nullptr;
    }
}
}  // namespace zemb
