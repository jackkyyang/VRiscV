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
    U = 0,
    #ifdef S_MODE
    S = 1
    #endif
    M = 3
} CPUMode;

// 用于记录取指过程中发生的错误
typedef struct fetch_status
{
    FetchWidth inst_num;
    uint64_t err_id;  // if no err, this is 0
    uint64_t err_address;
    uint64_t cause;
} FetchStatus;

typedef struct execute_status
{
    uint8_t exit;
    // 0: continue
    // 1: exit virtual machine
    MXLEN_T next_pc;
    MXLEN_T curr_pc;
    CPUMode next_mode;
    // for next process
    uint8_t branch;
    uint8_t exception;

} ExeStatus;

ExeStatus*      get_exe_st_ptr();
ExeStatus*      read_exe_st();
FetchStatus*    get_fet_st_ptr();
void            raise_iinstr_excp(uint64_t cause);
CPUMode         get_cpu_mode();
void            set_cpu_mode(CPUMode next_mode);

#endif // __CPU_GLB_H__