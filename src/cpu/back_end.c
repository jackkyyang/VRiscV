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
#include "sys_reg.h"
#include "cpu_glb.h"


// 定义通用寄存器
static MXLEN_T x[32]; // 通用寄存器
static MXLEN_T pc;
static MXLEN_T next_pc;
// Exceptions
// ----------------------------------------------
void backend_init(){
    // 初始化通用寄存器
    for (int i = 0; i < 32; i++)
    {
        x[i] = i;
    }
    // 初始化系统寄存器
    sys_reg_reset();
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

void instruction_execute(ExeParam *exe_param)
{
    ExeStatus *e_st = get_exe_st_ptr();
    CPUMode curr_mode = get_cpu_mode();

    e_st->curr_pc = exe_param->pc;
    pc           = exe_param->pc;
    // decode
    uint32_t inst  = *(exe_param->fetch_data_buf);
    e_st->inst = (MXLEN_T)inst;
    clear_flags();

    // 处理取指过程中的异常
    if (exe_param->fetch_status->err_id > 0)
    {
        exe_param->fetch_status->err_id = 0;
    }


    decode(inst,e_st);


    if (x[0] != 0) printf("Error! Cannot write value to X0!");

    // 按优先级选择异常
    if (e_st->exception){
        if (e_st->ecause.ifetch_breakpoint)
            trap2m(0,3,curr_mode);
        else if (e_st->ecause.instruction_page_fault)
            trap2m(0,12,curr_mode);
        else if (e_st->ecause.instruction_access_fault)
            trap2m(0,1,curr_mode);
        else if (e_st->ecause.illegal_instruction){
            raise_illegal_instruction(curr_mode,(MXLEN_T)inst);
        }
        else if (e_st->ecause.instruction_address_misaligned)
            trap2m(0,0,curr_mode);
        else if (e_st->ecause.ecall_from_u)
            trap2m(0,8,curr_mode);
        else if (e_st->ecause.ecall_from_s)
            trap2m(0,9,curr_mode);
        else if (e_st->ecause.ecall_from_m)
            trap2m(0,11,curr_mode);
        else if (e_st->ecause.ecall_breakpoint)
            trap2m(0,3,curr_mode);
        else if (e_st->ecause.lsu_breakpoint)
            trap2m(0,3,curr_mode);
        else if (e_st->ecause.load_page_fault)
            trap2m(0,13,curr_mode);
        else if (e_st->ecause.store_amo_page_fault)
            trap2m(0,15,curr_mode);
        else if (e_st->ecause.load_access_fault)
            trap2m(0,5,curr_mode);
        else if (e_st->ecause.store_access_fault)
            trap2m(0,7,curr_mode);
        else
            printf("Error Cannot find the exception cause!");
    }

    if (e_st->exception || e_st->mret) {
        // next_pc是由异常处理函数计算出来的，已经更新到e_st中
        next_pc = e_st->next_pc;
        e_st->exception = 0;
        e_st->branch = 0;
        e_st->mret = 0;
        memset(&(e_st->ecause), 0, sizeof(ECause));
    }
    else if (e_st->branch) { // 处理分支指令
        e_st->next_pc = next_pc;
        e_st->branch = 0;
    }
    else {
        e_st->next_pc = pc + 4;
    }

    instreth_inc(FETCH_NUM);
}
