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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cpu_config.h"
#include "front_end.h"
#include "back_end.h"


// 定义通用寄存器
static uint64_t x[32]; // 通用寄存器
static uint64_t pc;
static uint64_t next_pc;
// Exceptions
static ExcptInfo exception_info;
// ----------------------------------------------
void backend_init(){
    // 初始化通用寄存器
    for (int i = 0; i < 32; i++)
    {
        x[i] = i;
    }
    exception_info.exception_vaild = 0;
    exception_info.exception_cause = 0;
}

// flags
static ExeFlags flags;
uint64_t jmp_cnt;
uint64_t br_cnt;
uint64_t br_taken_cnt;
static void clear_flags(){
    memset(&flags, 0, sizeof(ExeFlags));
}

#include "decode.h"

ExeStatus instruction_execute(ExeParam *exe_param)
{
    ExeStatus e_st = {0};
    e_st.curr_pc = exe_param->pc;
    pc           = exe_param->pc;

    // decode
    uint32_t inst  = *exe_param->fetch_data_buf;
    clear_flags();

    decode(inst);

    if (x[0] != 0) printf("Error! Cannot write value to X0!");

    if (flags.is_branch || flags.is_jump) {
        e_st.next_pc = next_pc;
    } else {
        e_st.next_pc = pc + 4;
    }

    return e_st;
}
