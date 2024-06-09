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
#include "RegExp.h"

#include <regex.h>
#include <stdlib.h>

#include "Tracer.h"

namespace zemb {
RegExp::RegExp() {}

RegExp::~RegExp() {}

bool RegExp::match(const std::string& pattern, const std::string& source) {
    std::string str;
    int pos;
    return match(pattern, source, &str, &pos);
}

bool RegExp::match(const std::string& pattern, const std::string& source,
                   std::string* result, int* pos) {
    StringArray strArray;
    IntArray posArray;
    result->clear();
    bool rc = match(pattern, source, &strArray, &posArray, 1);
    if (rc) {
        *result = strArray[0];
        *pos = posArray[0];
    }
    return rc;
}

bool RegExp::match(const std::string& pattern, const std::string& source,
                   StringArray* strArray, IntArray* posArray, int maxMatches) {
    bool isMatched = false;
    regex_t regex;
    if (maxMatches <= 0) {
        maxMatches = 1;
    }
    if (0 != regcomp(&regex, CSTR(pattern), REG_EXTENDED)) {
        // TRACE_ERR_CLASS("regcomp error:%s, source:%s, pattern:%s!", ERRSTR,
        //                 CSTR(source), CSTR(pattern));
        return false;
    }
    strArray->clear();
    posArray->clear();
    char* pStr = const_cast<char*>(CSTR(source));
    char* pStart = pStr;
    for (auto i = 0; i < maxMatches; i++) {
        regmatch_t regmatch;
        int rc = regexec(&regex, pStr, 1, &regmatch, 0);
        if (rc == REG_NOMATCH) {
            break;
        } else if (rc < 0) {
            TRACE_ERR_CLASS("regexec error!");
            break;
        } else {
            isMatched = true;
            if (regmatch.rm_so != -1) {
                std::string result = std::string(pStr).substr(
                    regmatch.rm_so, regmatch.rm_eo - regmatch.rm_so);
                posArray->append(
                    static_cast<int>(pStr + regmatch.rm_so - pStart));
                strArray->append(result);
                pStr = pStr + regmatch.rm_eo;
            } else {
                break;
            }
        }
    }
    regfree(&regex);
    return isMatched;
}
}  // namespace zemb
