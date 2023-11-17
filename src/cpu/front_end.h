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


#ifndef __FRONT_END_H__
    #define __FRONT_END_H__

#include <stdint.h>
#include "cpu_config.h"

typedef struct inst_fetch_param
{
    uint64_t iid;           //当前执行到第几条指令
    uint64_t pc;            //当前的PC值
    InstSet inst_set;       //指令集：16，32，64
    FetchWidth max_inst_num; // 最大取指数
} FetchParam;

// 用于记录取指过程中发生的错误
typedef struct fetch_status
{
    FetchWidth inst_num;
    uint64_t err_id;  // if no err, this is 0
    uint64_t err_address;
    uint64_t cause;
} FetchStatus;


// 输入：
//  fetch_param: 必要的参数
//  inst_buf:    指向取指结果
// 输出 （unsigned int）：成功取指的数量
//  0：取指失败
//  other: 取指成功
FetchStatus instruction_fetch(FetchParam* fetch_param, int* inst_buf);





#endif //__FRONT_END_H__