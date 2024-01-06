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


#ifndef __CPU_GLB_H__
    #define __CPU_GLB_H__

#include "cpu_config.h"
#include <stdint.h>

typedef enum cpu_mode{

#ifdef U_MODE
    U = 0,
#endif
#ifdef S_MODE
    S = 1
#endif
    M = 3

} CPUMode;

// 用于记录取指过程中发生的错误
typedef struct fetch_status
{
    FetchWidth inst_num;
    // 见 get_ifu_fault();
    uint32_t err_id;
} FetchStatus;

typedef struct execute_status
{
    // 0: continue
    // 1: exit virtual machine with test pass
    // 2：exit virtual machine with test failure
    uint8_t exit;
    // 运行模式为自测模式，需要在ecall的时候检查寄存器结果
    uint8_t self_test;
    MXLEN_T next_pc;// 下一个指令的PC
    MXLEN_T curr_pc;// 当前指令的PC

    CPUMode next_mode; // 用于切换模式
    uint8_t branch; // 用于分支跳转
    uint8_t exception; //用于处理异常
    MXLEN_T inst; // 指令内容，异常时写入mtval

} ExeStatus;
// 存放执行结果
ExeStatus*      get_exe_st_ptr();
ExeStatus*      read_exe_st();
// 存放取指结果
FetchStatus*    get_fet_st_ptr();
void            raise_iinstr_excp(uint64_t cause);
CPUMode         get_cpu_mode();
void            set_cpu_mode(CPUMode next_mode);
void            raise_illegal_instruction(CPUMode curr_mode,MXLEN_T inst);
#endif // __CPU_GLB_H__