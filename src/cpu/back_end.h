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

#ifndef __BACKEND_H_
    #define __BACKEND_H_

#include <stdint.h>
#include "cpu_config.h"
#include "cpu_glb.h"
#include "front_end.h"

typedef struct exe_param
{
    uint64_t iid;           //当前执行到第几条指令
    uint64_t pc;            //当前的PC值
    InstSet  inst_set;       //指令集：16，32，64
    FetchStatus *fetch_status;
    uint32_t *fetch_data_buf;
}ExeParam;


void instruction_execute(ExeParam* exe_param);

// op
#define OP_32     0b00110011
#define LOAD      0b00000011
#define STORE     0b00100011
#define OP_IMM    0b00010011
#define SYSTEM    0b01110011
#define MISC_MEM  0b00001111
#define BRANCH    0b01100011
#define JAL       0b01101111
#define JALR      0b01100111
#define LUI       0b00110111
#define AUIPC     0b00010111
// func7
#define MULDIV    0b00000001

typedef struct exe_flags
{
    uint8_t is_branch;
    uint8_t is_jump;
    uint8_t is_alu;
    uint8_t is_mul;
    uint8_t is_div;
    uint8_t is_shift;
    uint8_t is_cmp;
    uint8_t is_csr;
    uint8_t is_store;
    uint8_t is_load;
    uint8_t is_li;
    uint8_t is_sys;
}ExeFlags;

typedef struct excpt_t
{
    uint8_t exception_vaild;
    uint64_t exception_cause;
} ExcptInfo;

void backend_init();

#endif //__BACKEND_H_