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
#include "ArgUtil.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Tracer.h"

namespace zemb {
ArgOption::ArgOption() {}
ArgOption::~ArgOption() {}

bool ArgOption::parseArgs(int argc, char** argv) {
    m_argVect.clear();
    m_optionMap.clear();
    for (auto i = 1; i < argc; i++) {
        m_argVect.emplace_back(std::string(argv[i]));
    }
    std::vector<std::string> valueList;
    std::string option;
    for (auto i = 0; i < m_argVect.size(); i++) {
        std::string arg = m_argVect[i];
        if (arg.size() == 1 && arg[0] == '-') {
            PRINT_ERR("ArgOption, invalid option: %s", CSTR(arg));
            m_argVect.clear();
            m_optionMap.clear();
            return false;
        }
        if (arg.size() >= 2) {
            if (arg[0] == '-') {
                if (!option.empty()) {
                    /* 上一个选项结束 */
                    m_optionMap.insert({option, valueList});
                    option.clear();
                }
                if (arg[1] == '-') {
                    option = arg.substr(2);
                } else {
                    option = arg.substr(1);
                }
                if (option.empty() || !isalpha(option[0]) ||
                    option.find("=") != std::string::npos) {
                    PRINT_ERR("ArgOption invalid option: %s", CSTR(arg));
                    m_argVect.clear();
                    m_optionMap.clear();
                    return false;
                }
                valueList.clear();
                continue;
            }
        }
        if (!option.empty()) {
            valueList.push_back(arg);
        }
    }
    if (!option.empty()) {
        m_optionMap.insert({option, valueList});
    }
    return true;
}

int ArgOption::hasOption(const std::string& option) {
    if (option.empty()) {
        return -1;
    }
    auto iter = m_optionMap.find(option);
    if (iter == m_optionMap.end()) {
        return -1;
    }
    return iter->second.size();
}

std::vector<std::string> ArgOption::getValue(const std::string& option) {
    std::vector<std::string> valueList;
    if (option.empty()) {
        return m_argVect;
    }
    auto iter = m_optionMap.find(option);
    if (iter != m_optionMap.end()) {
        return iter->second;
    }
    return valueList;
}

InputReader::InputReader() {}

InputReader::~InputReader() {}

InputReader& InputReader::waitInput() {
    std::cin >> m_inputStr;
    return *this;
}

bool InputReader::isString(const std::string& str) {
    if (m_inputStr == str) {
        return true;
    }
    return false;
}
int InputReader::size() { return m_inputStr.size(); }

int InputReader::asInt() {
    if (m_inputStr.empty()) {
        return 0;
    }
    return atoi(CSTR(m_inputStr));
}

float InputReader::asFloat() {
    if (m_inputStr.empty()) {
        return 0.0;
    }
    return atof(CSTR(m_inputStr));
}

std::string InputReader::asString() { return m_inputStr; }

const char* InputReader::asCString() { return CSTR(m_inputStr); }

}  // namespace zemb
