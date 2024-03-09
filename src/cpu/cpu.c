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

static CPUParam cpu_params;
void* cpu_run(void* param){
    cpu_params = *((CPUParam*)param);
    cpu_init(cpu_params.entry_addr,cpu_params.self_test);
    ExeStatus *e_st = read_exe_st();
    *(cpu_params.start_time) = clock();
    while (1)
    {
        // TimeOut 保护
        if(iid == cpu_params.TIME_OUT){
            *(cpu_params.end_time) = clock();
            printf("**********" L_RED "TIMEOUT" NONE "**********\n");
            printf("TIMEOUT Instruction Number: %lu\n",iid);
            printf("current PC: %lx\n",(uint64_t)(e_st->curr_pc));
            break;
        }

        volatile uint8_t stop_value = *(cpu_params.cpu_exit);
        if (stop_value)
        {
            *(cpu_params.end_time) = clock();
            printf("CPU exit!\n");
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
            *(cpu_params.end_time) = clock();
            printf("Virtual Machine Exit!\n");
            iid +=1;
            if (e_st->self_test)
            {
                printf("Self Test Exit! Total Instruction Number: %lu\n",iid);
                printf("current PC: %lx\n",(uint64_t)(e_st->curr_pc));
                printf("*******************************\n");
                FILE* st_fd = fopen("./self_test_result.log","w");
                if (e_st->exit == 1)
                {
                    printf("**********" L_BLUE " TEST PASS " NONE "**********\n");
                    fprintf(st_fd,"1");
                }
                else if(e_st->exit == 2){
                    printf("**********" L_RED " TEST FAIL " NONE "**********\n");
                    fprintf(st_fd,"0");
                }
                printf("*******************************\n");
                fclose(st_fd);
            }
            break;
        }

        if (cpu_params.tpc_fd != NULL) // 表明有打开的trace log文件，使能了trace pc的功能
        {
            if (fprintf(cpu_params.tpc_fd,"0x%x\n",(MXLEN_T)pc) < 0)
            {
                printf("Warning! found error during writing log file of Trace PC");
            }
        }


        // prepare for next instruction
        iid +=1;
        pc = e_st->next_pc;
    }
}

uint64_t get_iid()
{
    return iid;
}
