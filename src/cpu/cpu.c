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
#include "cpu_glb.h"
#include "../include/color.h"

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

static inline void update_exe_param(){
    exe_param.iid = iid;
    exe_param.pc = pc;
}

static void cpu_init(uint64_t entry_addr,uint8_t self_test)
{
    pc = entry_addr;
    iid = 0;
    fetch_param.max_inst_num = FETCH_NUM;
    fetch_param.inst_set = INST_SET;
    exe_param.inst_set = INST_SET;
    exe_param.fetch_data_buf = fetch_data_buf;
    exe_param.fetch_status = get_fet_st_ptr();
    backend_init();
    ExeStatus *e_st = get_exe_st_ptr();
    e_st->next_mode = M;
    e_st->self_test = self_test;
}

uint64_t cpu_run(uint64_t TIME_OUT,uint64_t entry_addr,uint8_t self_test){
    cpu_init(entry_addr,self_test);
    ExeStatus *e_st = read_exe_st();
    while (1)
    {
        // TimeOut 保护
        if(iid == TIME_OUT){
            printf("**********" L_RED "TIMEOUT" NONE "**********\n");
            printf("TIMEOUT Instruction Number: %lu\n",iid);
            printf("current PC: %lx\n",(uint64_t)(e_st->curr_pc));
            break;
        }

        set_cpu_mode(e_st->next_mode);
        // Front End process
        update_fetch_param();
        instruction_fetch(&fetch_param,fetch_data_buf);
        // Back End process
        update_exe_param();
        instruction_execute(&exe_param);

        if (e_st->exit != 0){
            printf("Virtual Machine Exit!\n");
            iid +=1;
            if (e_st->self_test)
            {
                printf("Self Test Exit! Total Instruction Number: %lu\n",iid);
                printf("current PC: %lx\n",(uint64_t)(e_st->curr_pc));
                printf("*******************************\n");
                if (e_st->exit == 1)
                {
                    printf("**********" L_BLUE " TEST PASS " NONE "**********\n");
                }
                else if(e_st->exit == 2){
                    printf("**********" L_RED " TEST FAIL " NONE "**********\n");
                }
                printf("*******************************\n");
            }

            break;
        }

        // prepare for next instruction
        iid +=1;
        pc = e_st->next_pc;
    }
    return iid;
}
