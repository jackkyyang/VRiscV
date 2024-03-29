/*
MIT License

Copyright (c) 2023 jackkyyang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef __CPU_CONFIG_H__
    #define __CPU_CONFIG_H__

#include <stdint.h>

#define U_MODE

typedef enum {
    I16 = 1,
    I32 = 2,
    I64 = 3
} InstSet;

typedef uint8_t FetchWidth;

#define IALIGN 32 //不支持压缩指令集，PC必须是4-byte对齐

#ifdef RV64
    #define MXLEN 64;
    #define INST_SET I64
    typedef uint64_t   MXLEN_T;
    typedef int64_t    MXLEN_ST;
    typedef __uint128_t  DMXLEN_T; // double length of the MXLEN
    typedef __int128_t   DMXLEN_ST; // double length of the MXLEN
    #define DMXLEN_HMSK 0xffffffffffffffff0000000000000000
#else
    #define MXLEN 32
    #define INST_SET I32
    typedef uint32_t  MXLEN_T;
    typedef int32_t   MXLEN_ST;
    typedef uint64_t  DMXLEN_T; // double length of the MXLEN
    typedef int64_t   DMXLEN_ST; // double length of the MXLEN
    #define DMXLEN_HMSK 0xffffffff00000000
#endif


#define FETCH_NUM 1

#endif //__CPU_CONFIG_H__

