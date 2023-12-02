// MIT License
//
// Copyright (c) 2023 jackkyyang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-------------------------------------------------------
// 描述:
//      1. 动态申请和管理虚拟机需要的内存
//      2. 为虚拟机内存提供底层抽象


#ifndef __MEM_POOL_H__
    #define __MEM_POOL_H__

#include <stdint.h>

#define ENTRY_SIZE 4096
#define ALIGN2PGSZ(addr,T) (addr & (~(T)(ENTRY_SIZE -1)))
#define MOD2PGSZ(addr,T) (addr & (T)(ENTRY_SIZE -1))

// 初始化内存池
void mem_pool_init();

// 释放所有内存池
void mem_pool_free();

// 查询该页是否存在， 地址必须是 ENTRY_SIZE
// 该地址对应的页面的首地址，地址一定为 ENTRY_SIZE
uint8_t* mem_pool_lkup(uint64_t addr);

// 返回当前内存池中有效的内存容量，单位KB
uint64_t get_mem_pool_size();
// 返回当前内存池中L2 table占用的内存数量，单位Byte
uint64_t get_l2_table_size();
// 返回当前内存池中L3 table占用的内存数量，单位Byte
uint64_t get_l3_table_size();

#endif //__MEM_POOL_H__