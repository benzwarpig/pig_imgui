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
#include "BaseType.h"

#include <stdio.h>
#include <stdlib.h>

#include "StrUtil.h"
#include "Tracer.h"
namespace zemb {

class VersionBuilder {
public:
    VersionBuilder(const std::string& appName, int major, int minor, int patch,
                   const std::string& date = BUILD_DATE,
                   const std::string& time = BUILD_TIME)
        : m_major(major), m_minor(minor), m_patch(patch) {
        m_version = std::string(appName) + std::string("V") +
                    std::to_string(m_major) + std::string(".") +
                    std::to_string(m_minor) + std::string(".") +
                    std::to_string(m_patch);
        {
            std::string monthStr[12] = {"Jan", "Feb", "Mar", "Apr",
                                        "May", "Jun", "Jul", "Aug",
                                        "Sep", "Oct", "Nov", "Dec"};
            auto sections = StrUtil::splitString(date, " ");
            uint32 year = 0, month = 0, date = 0;
            if (sections.size() == 3) {
                for (auto i = 0; i < 12; ++i) {
                    if (sections[0].compare(0, 3, monthStr[i]) == 0) {
                        month = i;
                        break;
                    }
                }
                date = std::stoi(sections[1]);
                year = std::stoi(sections[2]);
            }
            m_buildDate = StrUtil::format("%04d%02d%02d", year, month, date);
        }
        {
            auto sections = StrUtil::splitString(time, ":");
            uint32 hour = 0, min = 0, sec = 0;
            if (sections.size() == 3) {
                hour = std::stoi(sections[0]);
                min = std::stoi(sections[1]);
                sec = std::stoi(sections[2]);
            }
            m_buildTime = StrUtil::format("%02d%02d%02d", hour, min, sec);
        }
    }

private:
    friend AppVersion;
    int m_major{0}; /* 主版本 */
    int m_minor{0}; /* 小版本 */
    int m_patch{0}; /* 补丁版本 */
    std::string m_version;
    std::string m_buildDate;
    std::string m_buildTime;
};

VersionBuilder* g_appVersionBuilder{nullptr};

AppVersion::AppVersion() {}

int AppVersion::majorVersion() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_major;
    } else {
        return -1;
    }
}

int AppVersion::minorVersion() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_minor;
    } else {
        return -1;
    }
}

int AppVersion::patchVersion() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_patch;
    } else {
        return -1;
    }
}

std::string AppVersion::version() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_version;
    } else {
        return "";
    }
}

std::string AppVersion::date() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_buildDate;
    } else {
        return "";
    }
}

std::string AppVersion::time() {
    if (g_appVersionBuilder) {
        return g_appVersionBuilder->m_buildTime;
    } else {
        return "";
    }
}

void AppVersion::build(const std::string& appName, int major, int minor,
                       int patch, const std::string& date,
                       const std::string& time) {
    if (!g_appVersionBuilder) {
        g_appVersionBuilder =
            new VersionBuilder(appName, major, minor, patch, date, time);
    }
}

bool IntArray::initWithString(const std::string& arrayString) {
    m_array.clear();
    std::vector<std::string> strArray =
        StrUtil::cutString(arrayString, "[", "]", ",");
    int arraySize = strArray.size();
    if (arraySize > 0) {
        for (auto i = 0; i < arraySize; i++) {
            m_array.push_back(std::stoi(strArray[i]));
        }
        return true;
    }
    return false;
}

std::string IntArray::serialize() {
    int arraySize = m_array.size();
    std::string arrayString = "[";
    for (auto i = 0; i < arraySize; i++) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "%d", m_array[i]);
        arrayString += buf;
        if (i < (arraySize - 1)) {
            arrayString += ",";
        }
    }
    arrayString += "]";
    return arrayString;
}

bool DoubleArray::initWithString(const std::string& arrayString) {
    m_array.clear();
    std::vector<std::string> strArray =
        StrUtil::cutString(arrayString, "[", "]", ",");
    int arraySize = strArray.size();
    if (arraySize > 0) {
        for (auto i = 0; i < arraySize; i++) {
            m_array.push_back(atof(CSTR(strArray[i])));
        }
        return true;
    }
    return false;
}

std::string DoubleArray::serialize() {
    int arraySize = m_array.size();
    std::string arrayString = "[";
    for (auto i = 0; i < arraySize; i++) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "%lf", m_array[i]);
        arrayString += buf;
        if (i < (arraySize - 1)) {
            arrayString += ",";
        }
    }
    arrayString += "]";
    return arrayString;
}

bool StringArray::initWithString(const std::string& arrayString) {
    m_array.clear();
    std::string tmpString = StrUtil::trimTailBlank(arrayString);
    int strLen = tmpString.size();

    if (strLen < 2 || tmpString[0] != '[' || tmpString[strLen - 1] != ']') {
        return false;
    }
    int quotNum = 0;
    int commaNum = 0;
    std::string item;
    for (auto i = 1; i < strLen - 1; i++) {
        if (tmpString[i] == '\"') {
            quotNum++;
            if (quotNum % 2 == 1) {
                item = "";
                commaNum = 0;
            } else {
                m_array.emplace_back(item);
            }
            continue;
        } else {
            if (quotNum % 2 == 0) {
                if (tmpString[i] == ' ') {
                    continue;
                } else if (tmpString[i] == ',') {
                    commaNum++;
                    if (commaNum > 1) {
                        item = "";
                        m_array.emplace_back(item);
                    }
                } else {
                    m_array.clear();
                    return false;
                }
            } else {
                item.append(1, tmpString[i]);
            }
        }
    }
    if (quotNum % 2 == 1) {
        m_array.clear();
        return false;
    }
    return true;
}

std::string StringArray::serialize() {
    int arraySize = m_array.size();
    std::string arrayString = "[";
    for (auto i = 0; i < arraySize; i++) {
        arrayString += "\"";
        arrayString += m_array[i];
        arrayString += "\"";
        if (i < (arraySize - 1)) {
            arrayString += ",";
        }
    }
    arrayString += "]";
    return arrayString;
}

TupleItem::TupleItem(int value) {
    m_type = BASETYPE_INT;
    m_value = value;
    m_string = StrUtil::format("%d", static_cast<int>(m_value));
}

TupleItem::TupleItem(double value) {
    m_type = BASETYPE_DOUBLE;
    m_value = value;
    m_string = StrUtil::format("%f", m_value);
}

TupleItem::TupleItem(std::string value) {
    m_type = BASETYPE_STRING;
    m_string = value;
    m_value = atof(CSTR(m_string));
}

TupleItem::TupleItem(const TupleItem& item) {
    m_type = item.m_type;
    m_value = item.m_value;
    m_string = item.m_string;
}

TupleItem::~TupleItem() {}

int TupleItem::baseType() { return m_type; }

int TupleItem::toInt() { return static_cast<int>(m_value); }

double TupleItem::toDouble() { return m_value; }

std::string TupleItem::toString() { return m_string; }

Tuple::Tuple() {}

Tuple::Tuple(const Tuple& tuple) {
    int num = tuple.m_itemVect.size();
    for (auto i = 0; i < num; i++) {
        switch (tuple.m_itemVect[i]->baseType()) {
            case BASETYPE_INT: {
                auto itemPtr =
                    std::make_unique<TupleItem>(tuple.m_itemVect[i]->toInt());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            case BASETYPE_DOUBLE: {
                auto itemPtr = std::make_unique<TupleItem>(
                    tuple.m_itemVect[i]->toDouble());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            case BASETYPE_STRING: {
                auto itemPtr = std::make_unique<TupleItem>(
                    tuple.m_itemVect[i]->toString());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            default:
                break;
        }
    }
}

Tuple::~Tuple() { clear(); }

bool Tuple::initWithString(const std::string& tupleString) {
    m_itemVect.clear();
    string tmpString = StrUtil::trimTailBlank(tupleString);
    int strLen = tmpString.size();
    if (strLen < 2 || tmpString[0] != '(' || tmpString[strLen - 1] != ')') {
        return false;
    }
    int quotNum = 0;
    int commaNum = 0;
    int type = BASETYPE_NONE;
    std::string item;
    for (auto i = 1; i < strLen - 1; i++) {
        if (tmpString[i] != ' ' && tmpString[i] != ',' &&
            tmpString[i] != '\"') {
            if (commaNum != 0) {
                TRACE_ERR_CLASS("Invalid tuple: %s,i=%d", CSTR(tmpString), i);
                return false;
            }
            item.append(1, tmpString[i]);
            continue;
        }
        if (tmpString[i] == '\"') {
            quotNum++;
            if (quotNum % 2 == 0) { /* 字符串结束 */
                this->append(item);
                commaNum = 0;
                type = BASETYPE_NONE;
            } else { /* 字符串开始 */
                type = BASETYPE_STRING;
            }
            item.clear();
            continue;
        } else if (tmpString[i] == ' ') {
            if (type == BASETYPE_STRING) {
                item.append(1, tmpString[i]);
            }
            continue;
        } else if (tmpString[i] == ',') {
            if (type == BASETYPE_STRING) { /* 字符串里面的逗号 */
                item.append(1, tmpString[i]);
            } else { /* 当前元素结束 */
                commaNum++;
                if (commaNum <= 1) {
                    if (!item.empty()) {
                        if (item.find(".") != string::npos) {
                            double val = atof(CSTR(item));
                            this->append(val);
                        } else {
                            int val = atoi(CSTR(item));
                            this->append(val);
                        }
                    }
                    item.clear();
                    commaNum = 0;
                    type = BASETYPE_NONE;
                    continue;
                } else {
                    clear();
                    return false;
                }
            }
            continue;
        }
    }
    if (!item.empty()) {
        if (item.find(".") != std::string::npos) {
            double val = atof(CSTR(item));
            this->append(val);
        } else {
            int val = atoi(CSTR(item));
            this->append(val);
        }
    }
    return true;
}

int Tuple::size() { return m_itemVect.size(); }

int Tuple::type() { return BASETYPE_TUPLE; }

void Tuple::clear() { m_itemVect.clear(); }

std::string Tuple::serialize() {
    std::string result = "(";
    int num = m_itemVect.size();
    for (auto i = 0; i < num; i++) {
        switch (m_itemVect[i]->baseType()) {
            case BASETYPE_INT: {
                char buf[32] = {0};
                snprintf(buf, sizeof(buf), "%d", m_itemVect[i]->toInt());
                result += std::string(buf);
                break;
            }
            case BASETYPE_DOUBLE: {
                char buf[32] = {0};
                snprintf(buf, sizeof(buf), "%lf", m_itemVect[i]->toDouble());
                result += std::string(buf);
                break;
            }
            case BASETYPE_STRING: {
                result += "\"";
                result += m_itemVect[i]->toString();
                result += "\"";
                break;
            }
            default:
                return "";
        }
        if (i != (num - 1)) {
            result += ",";
        }
    }
    result += ")";
    return result;
}

TupleItem& Tuple::operator[](int idx) {
    TupleItem& rc = *(m_itemVect[idx]);
    return rc;
}

Tuple& Tuple::operator=(const Tuple& tuple) {
    if (this == &tuple) {
        return (*this);
    }
    /* 清除所有元素 */
    clear();
    /* 重新拷贝tuple的元素 */
    int num = tuple.m_itemVect.size();
    for (auto i = 0; i < num; i++) {
        switch (tuple.m_itemVect[i]->baseType()) {
            case BASETYPE_INT: {
                auto itemPtr =
                    std::make_unique<TupleItem>(tuple.m_itemVect[i]->toInt());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            case BASETYPE_DOUBLE: {
                auto itemPtr = std::make_unique<TupleItem>(
                    tuple.m_itemVect[i]->toDouble());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            case BASETYPE_STRING: {
                auto itemPtr = std::make_unique<TupleItem>(
                    tuple.m_itemVect[i]->toString());
                m_itemVect.push_back(std::move(itemPtr));
                break;
            }
            default:
                break;
        }
    }
    return (*this);
}
}  // namespace zemb
