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
#include "front_end.h"
#include "back_end.h"


// 定义通用寄存器
static uint64_t x[32]; // 通用寄存器


// ----------------------------------------------
static void init_register(){
    // 初始化通用寄存器
    for (int i = 0; i < 32; i++)
    {
        x[i] = i;
    }
    // 初始化PC
}

ExeStatus instruction_execute(ExeParam *exe_param)
{
    ExeStatus e_st = {0};
    e_st.curr_pc = exe_param->pc;
    e_st.next_pc = exe_param->pc += 4;
    return e_st;
}
