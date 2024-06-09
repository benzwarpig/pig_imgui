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
 * Copyright 2014~2017 @ ShenZhen ,China
 *******************************************************************************/
#include "CheckUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ComUtil.h"
#include "FileUtil.h"

/* Rotate a 32 bit integer by n bytes */
#define rol(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

namespace zemb {
/**
 * Polynomial:0x8005
 * Initial Value:0xffff
 * Final Xor Value:0x0
 * Input & Result Reflected:True
 */
const uint16 CRC16Table_8005[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601,
    0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440, 0xcc01, 0x0cc0,
    0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81,
    0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941,
    0x1b00, 0xdbc1, 0xda81, 0x1a40, 0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01,
    0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0,
    0x1680, 0xd641, 0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081,
    0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441, 0x3c00,
    0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0,
    0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1, 0xe981,
    0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41,
    0x2d00, 0xedc1, 0xec81, 0x2c40, 0xe401, 0x24c0, 0x2580, 0xe541, 0x2700,
    0xe7c1, 0xe681, 0x2640, 0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0,
    0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281,
    0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01,
    0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840, 0x7800, 0xb8c1,
    0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41, 0xbe01, 0x7ec0, 0x7f80,
    0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541,
    0x7700, 0xb7c1, 0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101,
    0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0,
    0x5280, 0x9241, 0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481,
    0x5440, 0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841, 0x8801,
    0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1,
    0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41, 0x4400, 0x84c1, 0x8581,
    0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341,
    0x4100, 0x81c1, 0x8081, 0x4040};

uint16 CRCCheck::check16CRC8005(const std::string& content) {
#if 1
    uint16 crc = 0xffff;
    for (auto& c : content) {
        crc = CRC16Table_8005[(crc ^ (c)) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0x0;
#else
    uint16 crcSum = 0xFFFF;
    for (auto i = 0; i < content.size(); i++) {
        crcSum ^= (uint8)content[i];
        for (auto j = 8; j > 0; j--) {
            if (crcSum & 0x0001) {
                crcSum >>= 1;
                crcSum ^= 0xA001;
            } else {
                crcSum >>= 1;
            }
        }
    }
    return crcSum;
#endif
}

/**
 * @struct MD5Context
 * @brief MD5上下文
 */
struct MD5Context {
    uint32 A, B, C, D; /* chaining variables */
    uint32 nblocks;
    uint8 buf[64];
    sint32 count;
};

MD5Check::MD5Check() {}

MD5Check::~MD5Check() {}

bool MD5Check::checkFile(const std::string& fileName, std::string* md5sum) {
    MD5Context ctx;
    initContext(&ctx);
    File file;
    uint8 buffer[4096];
    if (!file.open(CSTR(fileName), IO_MODE_RD_ONLY)) {
        return false;
    }
    while (1) {
        int n = file.readData(reinterpret_cast<char*>(buffer), sizeof(buffer));
        if (n < 0) {
            return false;
        } else if (0 == n) {
            break;
        } else {
            md5Write(&ctx, buffer, n);
        }
    }
    md5Final(&ctx);
    file.close();
    *md5sum = "";
    for (int i = 0; i < 16; i++) {
        char tmp[8] = {0};
        snprintf(tmp, sizeof(tmp) - 1, "%02x", ctx.buf[i]);
        md5sum->append(tmp);
    }
    return true;
}

bool MD5Check::checkString(const std::string& srcString, std::string* md5sum) {
    MD5Context ctx;
    initContext(&ctx);
    char* buf = const_cast<char*>(CSTR(srcString));
    if (srcString.empty()) {
        return false;
    }
    md5Write(&ctx, reinterpret_cast<uint8*>(buf), srcString.size());
    md5Final(&ctx);
    md5sum->clear();
    for (int i = 0; i < 16; i++) {
        char tmp[8] = {0};
        snprintf(tmp, sizeof(tmp) - 1, "%02x", ctx.buf[i]);
        md5sum->append(tmp);
    }
    return true;
}

void MD5Check::initContext(MD5Context* ctx) {
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;
    ctx->nblocks = 0;
    ctx->count = 0;
}

/* transform n*64 bytes */
void MD5Check::calculate(MD5Context* ctx, uint8* data) {
    uint32 correct_words[16];
    uint32 A = ctx->A;
    uint32 B = ctx->B;
    uint32 C = ctx->C;
    uint32 D = ctx->D;
    uint32* cwp = correct_words;

    if (ComUtil::isBigEndian()) {
        int i;
        uint8 *p2, *p1;
        for (i = 0, p1 = data, p2 = reinterpret_cast<uint8*>(correct_words);
             i < 16; i++, p2 += 4) {
            p2[3] = *p1++;
            p2[2] = *p1++;
            p2[1] = *p1++;
            p2[0] = *p1++;
        }
    } else {
        memcpy(correct_words, data, 64);
    }

/* These are the four functions used in the four steps of the MD5 algorithm
   and defined in the RFC 1321.  The first function is a little bit optimized
   (as found in Colin Plumbs public domain implementation).  */
/* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF(d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

#define OP(a, b, c, d, s, T)             \
    do {                                 \
        a += FF(b, c, d) + (*cwp++) + T; \
        a = rol(a, s);                   \
        a += b;                          \
    } while (0)

    /* Before we start, one word about the strange constants.
       They are defined in RFC 1321 as

       T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64
     */

    /* Round 1.  */
    OP(A, B, C, D, 7, 0xd76aa478);
    OP(D, A, B, C, 12, 0xe8c7b756);
    OP(C, D, A, B, 17, 0x242070db);
    OP(B, C, D, A, 22, 0xc1bdceee);
    OP(A, B, C, D, 7, 0xf57c0faf);
    OP(D, A, B, C, 12, 0x4787c62a);
    OP(C, D, A, B, 17, 0xa8304613);
    OP(B, C, D, A, 22, 0xfd469501);
    OP(A, B, C, D, 7, 0x698098d8);
    OP(D, A, B, C, 12, 0x8b44f7af);
    OP(C, D, A, B, 17, 0xffff5bb1);
    OP(B, C, D, A, 22, 0x895cd7be);
    OP(A, B, C, D, 7, 0x6b901122);
    OP(D, A, B, C, 12, 0xfd987193);
    OP(C, D, A, B, 17, 0xa679438e);
    OP(B, C, D, A, 22, 0x49b40821);

#undef OP
#define OP(f, a, b, c, d, k, s, T)              \
    do {                                        \
        a += f(b, c, d) + correct_words[k] + T; \
        a = rol(a, s);                          \
        a += b;                                 \
    } while (0)

    /* Round 2.  */
    OP(FG, A, B, C, D, 1, 5, 0xf61e2562);
    OP(FG, D, A, B, C, 6, 9, 0xc040b340);
    OP(FG, C, D, A, B, 11, 14, 0x265e5a51);
    OP(FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
    OP(FG, A, B, C, D, 5, 5, 0xd62f105d);
    OP(FG, D, A, B, C, 10, 9, 0x02441453);
    OP(FG, C, D, A, B, 15, 14, 0xd8a1e681);
    OP(FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
    OP(FG, A, B, C, D, 9, 5, 0x21e1cde6);
    OP(FG, D, A, B, C, 14, 9, 0xc33707d6);
    OP(FG, C, D, A, B, 3, 14, 0xf4d50d87);
    OP(FG, B, C, D, A, 8, 20, 0x455a14ed);
    OP(FG, A, B, C, D, 13, 5, 0xa9e3e905);
    OP(FG, D, A, B, C, 2, 9, 0xfcefa3f8);
    OP(FG, C, D, A, B, 7, 14, 0x676f02d9);
    OP(FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

    /* Round 3.  */
    OP(FH, A, B, C, D, 5, 4, 0xfffa3942);
    OP(FH, D, A, B, C, 8, 11, 0x8771f681);
    OP(FH, C, D, A, B, 11, 16, 0x6d9d6122);
    OP(FH, B, C, D, A, 14, 23, 0xfde5380c);
    OP(FH, A, B, C, D, 1, 4, 0xa4beea44);
    OP(FH, D, A, B, C, 4, 11, 0x4bdecfa9);
    OP(FH, C, D, A, B, 7, 16, 0xf6bb4b60);
    OP(FH, B, C, D, A, 10, 23, 0xbebfbc70);
    OP(FH, A, B, C, D, 13, 4, 0x289b7ec6);
    OP(FH, D, A, B, C, 0, 11, 0xeaa127fa);
    OP(FH, C, D, A, B, 3, 16, 0xd4ef3085);
    OP(FH, B, C, D, A, 6, 23, 0x04881d05);
    OP(FH, A, B, C, D, 9, 4, 0xd9d4d039);
    OP(FH, D, A, B, C, 12, 11, 0xe6db99e5);
    OP(FH, C, D, A, B, 15, 16, 0x1fa27cf8);
    OP(FH, B, C, D, A, 2, 23, 0xc4ac5665);

    /* Round 4.  */
    OP(FI, A, B, C, D, 0, 6, 0xf4292244);
    OP(FI, D, A, B, C, 7, 10, 0x432aff97);
    OP(FI, C, D, A, B, 14, 15, 0xab9423a7);
    OP(FI, B, C, D, A, 5, 21, 0xfc93a039);
    OP(FI, A, B, C, D, 12, 6, 0x655b59c3);
    OP(FI, D, A, B, C, 3, 10, 0x8f0ccc92);
    OP(FI, C, D, A, B, 10, 15, 0xffeff47d);
    OP(FI, B, C, D, A, 1, 21, 0x85845dd1);
    OP(FI, A, B, C, D, 8, 6, 0x6fa87e4f);
    OP(FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
    OP(FI, C, D, A, B, 6, 15, 0xa3014314);
    OP(FI, B, C, D, A, 13, 21, 0x4e0811a1);
    OP(FI, A, B, C, D, 4, 6, 0xf7537e82);
    OP(FI, D, A, B, C, 11, 10, 0xbd3af235);
    OP(FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
    OP(FI, B, C, D, A, 9, 21, 0xeb86d391);

    /* Put checksum in context given as argument.  */
    ctx->A += A;
    ctx->B += B;
    ctx->C += C;
    ctx->D += D;
}

/* The routine updates the message-digest context to
 * account for the presence of each of the characters inBuf[0..inLen-1]
 * in the message whose digest is being computed.
 */
void MD5Check::md5Write(MD5Context* hd, uint8* inbuf, int inlen) {
    if (hd->count == 64) { /* flush the buffer */
        calculate(hd, hd->buf);
        hd->count = 0;
        hd->nblocks++;
    }
    if (!inbuf) {
        return;
    }
    if (hd->count) {
        for (; inlen && hd->count < 64; inlen--) {
            hd->buf[hd->count++] = *inbuf++;
        }
        md5Write(hd, nullptr, 0);
        if (!inlen) {
            return;
        }
    }

    while (inlen >= 64) {
        calculate(hd, inbuf);
        hd->count = 0;
        hd->nblocks++;
        inlen -= 64;
        inbuf += 64;
    }
    for (; inlen && (hd->count < 64); inlen--) {
        hd->buf[hd->count++] = *inbuf++;
    }
}

/* The routine final terminates the message-digest computation and
 * ends with the desired message digest in mdContext->digest[0...15].
 * The handle is prepared for a new MD5 cycle.
 * Returns 16 bytes representing the digest.
 */
void MD5Check::md5Final(MD5Context* hd) {
    uint32 t, msb, lsb;
    uint8* p;

    md5Write(hd, nullptr, 0); /* flush */

    t = hd->nblocks;
    /* multiply by 64 to make a byte count */
    lsb = t << 6;
    msb = t >> 26;
    /* add the count */
    t = lsb;
    if ((lsb += hd->count) < t) {
        msb++;
    }
    /* multiply by 8 to make a bit count */
    t = lsb;
    lsb <<= 3;
    msb <<= 3;
    msb |= t >> 29;

    if (hd->count < 56) {            /* enough room */
        hd->buf[hd->count++] = 0x80; /* pad */
        while (hd->count < 56) {
            hd->buf[hd->count++] = 0; /* pad */
        }
    } else {                         /* need one extra block */
        hd->buf[hd->count++] = 0x80; /* pad character */
        while (hd->count < 64) {
            hd->buf[hd->count++] = 0;
        }
        md5Write(hd, nullptr, 0); /* flush */
        memset(hd->buf, 0, 56);   /* fill next block with zeroes */
    }
    /* append the 64 bit count */
    hd->buf[56] = lsb;
    hd->buf[57] = lsb >> 8;
    hd->buf[58] = lsb >> 16;
    hd->buf[59] = lsb >> 24;
    hd->buf[60] = msb;
    hd->buf[61] = msb >> 8;
    hd->buf[62] = msb >> 16;
    hd->buf[63] = msb >> 24;
    calculate(hd, hd->buf);

    p = hd->buf;
    if (ComUtil::isBigEndian()) {
        *p++ = hd->A;
        *p++ = hd->A >> 8;
        *p++ = hd->A >> 16;
        *p++ = hd->A >> 24;
        *p++ = hd->B;
        *p++ = hd->B >> 8;
        *p++ = hd->B >> 16;
        *p++ = hd->B >> 24;
        *p++ = hd->C;
        *p++ = hd->C >> 8;
        *p++ = hd->C >> 16;
        *p++ = hd->C >> 24;
        *p++ = hd->D;
        *p++ = hd->D >> 8;
        *p++ = hd->D >> 16;
        *p++ = hd->D >> 24;
    } else {
        *(reinterpret_cast<uint32*>(p)) = hd->A;
        p += 4;
        *(reinterpret_cast<uint32*>(p)) = hd->B;
        p += 4;
        *(reinterpret_cast<uint32*>(p)) = hd->C;
        p += 4;
        *(reinterpret_cast<uint32*>(p)) = hd->D;
        p += 4;
    }
}
}  // namespace zemb
