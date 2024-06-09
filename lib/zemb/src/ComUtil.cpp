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
#include "ComUtil.h"

#include <ctype.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <uuid/uuid.h>

#include "FileUtil.h"
#include "Tracer.h"

namespace zemb {
bool ComUtil::equal(float f1, float f2, float precision) {
    return (fabs(f1 - f2) < precision) ? true : false;
}

int ComUtil::random(int max) {
    if (max <= 0) {
        max = RAND_MAX;
    }
    struct timeval current;
    unsigned int seed;
    gettimeofday(&current, nullptr);
    seed = current.tv_sec + current.tv_usec;
    srand((unsigned)seed);
    return rand() % max;
}

std::string ComUtil::uuid() {
    uuid_t uuid;
    char buf[40] = {0};
    uuid_generate(uuid);
    uuid_unparse(uuid, buf);
    return std::string(buf);
}

bool ComUtil::isBigEndian(void) {
    union EndianWrap_U {
        sint32 a;
        uint8 b;
    };
    union EndianWrap_U ew;
    ew.a = 0x01;
    return (ew.b) ? false : true;
}

uint16 ComUtil::host2LitEndian(uint16 value) {
    union EndianWrap_U {
        uint16 v;
        char c[2];
    };
    if (isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[1];
        ew2.c[1] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

uint32 ComUtil::host2LitEndian(uint32 value) {
    union EndianWrap_U {
        uint32 v;
        char c[4];
    };
    if (isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[3];
        ew2.c[1] = ew1.c[2];
        ew2.c[2] = ew1.c[1];
        ew2.c[3] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

float ComUtil::host2LitEndian(float value) {
    union EndianWrap_U {
        float v;
        char c[4];
    };
    if (isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[3];
        ew2.c[1] = ew1.c[2];
        ew2.c[2] = ew1.c[1];
        ew2.c[3] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

uint16 ComUtil::host2BigEndian(uint16 value) {
    union EndianWrap_U {
        uint16 v;
        char c[2];
    };
    if (!isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[1];
        ew2.c[1] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

uint32 ComUtil::host2BigEndian(uint32 value) {
    union EndianWrap_U {
        uint32 v;
        char c[4];
    };
    if (!isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[3];
        ew2.c[1] = ew1.c[2];
        ew2.c[2] = ew1.c[1];
        ew2.c[3] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

float ComUtil::host2BigEndian(float value) {
    union EndianWrap_U {
        float v;
        char c[4];
    };
    if (!isBigEndian()) {
        union EndianWrap_U ew1, ew2;
        ew1.v = value;
        ew2.c[0] = ew1.c[3];
        ew2.c[1] = ew1.c[2];
        ew2.c[2] = ew1.c[1];
        ew2.c[3] = ew1.c[0];
        return ew2.v;
    }
    return value;
}

std::string ComUtil::unicodeOneToUtf8String(uint32 unicode) {
    int utflen = 0;
    char utf8code[7] = {0};

    unicode = host2LitEndian(unicode);

    if (unicode <= 0x0000007F) {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        utf8code[0] = (unicode & 0x7F);
        utflen = 1;
    } else if (unicode >= 0x00000080 && unicode <= 0x000007FF) {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        utf8code[1] = (unicode & 0x3F) | 0x80;
        utf8code[0] = ((unicode >> 6) & 0x1F) | 0xC0;
        utflen = 2;
    } else if (unicode >= 0x00000800 && unicode <= 0x0000FFFF) {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        utf8code[2] = (unicode & 0x3F) | 0x80;
        utf8code[1] = ((unicode >> 6) & 0x3F) | 0x80;
        utf8code[0] = ((unicode >> 12) & 0x0F) | 0xE0;
        utflen = 3;
    } else if (unicode >= 0x00010000 && unicode <= 0x001FFFFF) {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        utf8code[3] = (unicode & 0x3F) | 0x80;
        utf8code[2] = ((unicode >> 6) & 0x3F) | 0x80;
        utf8code[1] = ((unicode >> 12) & 0x3F) | 0x80;
        utf8code[0] = ((unicode >> 18) & 0x07) | 0xF0;
        utflen = 4;
    } else if (unicode >= 0x00200000 && unicode <= 0x03FFFFFF) {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx
        // 10xxxxxx
        utf8code[4] = (unicode & 0x3F) | 0x80;
        utf8code[3] = ((unicode >> 6) & 0x3F) | 0x80;
        utf8code[2] = ((unicode >> 12) & 0x3F) | 0x80;
        utf8code[1] = ((unicode >> 18) & 0x3F) | 0x80;
        utf8code[0] = ((unicode >> 24) & 0x03) | 0xF8;
        utflen = 5;
    } else if (unicode >= 0x04000000 && unicode <= 0x7FFFFFFF) {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx
        // 10xxxxxx 10xxxxxx
        utf8code[5] = (unicode & 0x3F) | 0x80;
        utf8code[4] = ((unicode >> 6) & 0x3F) | 0x80;
        utf8code[3] = ((unicode >> 12) & 0x3F) | 0x80;
        utf8code[2] = ((unicode >> 18) & 0x3F) | 0x80;
        utf8code[1] = ((unicode >> 24) & 0x3F) | 0x80;
        utf8code[0] = ((unicode >> 30) & 0x01) | 0xFC;
        utflen = 6;
    }
    std::string utf8str = utf8code;
    return utf8str;
}

uint32 ComUtil::utf8OneToUnicode(const char* utf8code) {
    // b1 表示UTF-8编码的高字节, b2 表示次高字节, ...
    uint8 b1, b2, b3, b4, b5, b6;

    uint8 utfbytes = 0;
    uint8 tmp = utf8code[0];
    while ((tmp & 0x80) != 0) {
        utfbytes++;
        tmp = tmp << 1;
    }

    uint32 unicode = 0x0;
    uint8* unibuf = reinterpret_cast<uint8*>(&unicode);
    switch (utfbytes) {
        case 0:
            unibuf[0] = utf8code[0];
            break;
        case 2:
            b1 = utf8code[0];
            b2 = utf8code[1];
            if ((b2 & 0xE0) != 0x80) {
                return 0;
            }
            unibuf[0] = (b1 << 6) + (b2 & 0x3F);
            unibuf[1] = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = utf8code[0];
            b2 = utf8code[1];
            b3 = utf8code[2];
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)) {
                return 0;
            }
            unibuf[0] = (b2 << 6) + (b3 & 0x3F);
            unibuf[1] = (b1 << 4) + ((b2 >> 2) & 0x0F);
            break;
        case 4:
            b1 = utf8code[0];
            b2 = utf8code[1];
            b3 = utf8code[2];
            b4 = utf8code[3];
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) ||
                ((b4 & 0xC0) != 0x80)) {
                return 0;
            }
            unibuf[0] = (b3 << 6) + (b4 & 0x3F);
            unibuf[1] = (b2 << 4) + ((b3 >> 2) & 0x0F);
            unibuf[2] = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = utf8code[0];
            b2 = utf8code[1];
            b3 = utf8code[2];
            b4 = utf8code[3];
            b5 = utf8code[4];
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) ||
                ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)) {
                return 0;
            }
            unibuf[0] = (b4 << 6) + (b5 & 0x3F);
            unibuf[1] = (b3 << 4) + ((b4 >> 2) & 0x0F);
            unibuf[2] = (b2 << 2) + ((b3 >> 4) & 0x03);
            unibuf[3] = (b1 << 6);
            break;
        case 6:
            b1 = utf8code[0];
            b2 = utf8code[1];
            b3 = utf8code[2];
            b4 = utf8code[3];
            b5 = utf8code[4];
            b6 = utf8code[5];
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) ||
                ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) ||
                ((b6 & 0xC0) != 0x80)) {
                return 0;
            }
            unibuf[0] = (b5 << 6) + (b6 & 0x3F);
            unibuf[1] = (b5 << 4) + ((b6 >> 2) & 0x0F);
            unibuf[2] = (b3 << 2) + ((b4 >> 4) & 0x03);
            unibuf[3] = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            return 0;
            break;
    }
    return unicode;
}

uint32 ComUtil::bitsReverse(uint32 ref, uint8 bits) {
    uint32 value = 0;
    if (bits > 32) return value;
    for (uint8 i = 1; i < (bits + 1); i++) {
        if (ref & 0x01) {
            value |= 1 << (bits - i);
        }
        ref >>= 1;
    }
    return (ref << bits) + value;
}

int ComUtil::eval(const char* expression) { return 0; }
}  // namespace zemb
