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

#include "cpu_config.h"
#include "cpu.h"
#include "front_end.h"
#include "back_end.h"

// ----------------------------------------------
// 定义PC
static uint64_t pc;
// ----------------------------------------------
// 指令计数器
static uint64_t iid;
// 基本状态
static FetchParam fetch_param;
static ExeParam exe_param;
// ----------------------------------------------
static uint32_t fetch_data_buf [FETCH_NUM]; // 指令缓存

static inline void update_fetch_param(){
    fetch_param.iid = iid;
    fetch_param.pc = pc;
}

static inline void update_exe_param(FetchStatus f_st){
    exe_param.iid = iid;
    exe_param.pc = pc;
    exe_param.fetch_status = f_st;
}

static void cpu_init()
{
    pc = RESET_ADDR;
    iid = 0;
    fetch_param.max_inst_num = FETCH_NUM;
    fetch_param.inst_set = INST_SET;
    exe_param.inst_set = INST_SET;
    exe_param.fetch_data_buf = fetch_data_buf;
}

uint64_t cpu_start(uint64_t TIME_OUT){
    cpu_init();
    FetchStatus f_st;
    ExeStatus e_st;
    while (1)
    {
        // TimeOut 保护
        if(iid == TIME_OUT){
            printf("CPU timeout! Total Instruction Number: %lu\n",iid);
            printf("current PC: %lx\n",e_st.curr_pc);
            break;
        }

        // Front End process
        update_fetch_param();
        f_st = instruction_fetch(&fetch_param,fetch_data_buf);
        // Back End process
        update_exe_param(f_st);
        e_st = instruction_execute(&exe_param);
        // 处理 各种error
        if (e_st.redirect == 2){ // normal exit
            //临时处理，后面都在 exception report中处理
            printf("Execution End!");
            iid +=1;
            break;
        }

        // prepare for next instruction
        iid +=1;
        pc = e_st.next_pc;
    }
    return iid;
}
