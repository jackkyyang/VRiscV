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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/comm.h"
#include "mem_pool.h"

#define ALIGN32_L1(addr) ((addr & (0xf0000000)) >> 28)
#define ALIGN32_L2(addr) ((addr & (0x0ff80000)) >> 19)
#define ALIGN32_L3(addr) ((addr & (0x0007f000)) >> 12)

#define L1_IDX_LEN  4
#define L2_IDX_LEN  9
#define L3_IDX_LEN  (20- L1_IDX_LEN - L2_IDX_LEN)
#define L1_LEN      (1 << L1_IDX_LEN)
#define L2_LEN      (1 << L2_IDX_LEN)
#define L3_LEN      (1 << L3_IDX_LEN)

static uint64_t mem_pool_size; // 内存池大小，单位是ENTRY_SIZE
static uint64_t l2_table_size; // 内存池大小，单位是ENTRY_SIZE
static uint64_t l3_table_size; // 内存池大小，单位是ENTRY_SIZE

// 这里使用类似MMU管理页表的手段来管理内存池的地址
// 因为是32位的机器，且地址一定是4KB对齐
// 第一级表有16项，用addr[31:28] 来索引，共8 * 16 = 128B，只有第一级表是静态表
// 第二级表有512项，用addr[27:19] 来索引，共512*8 = 4KB
// 第三级表有128项，用addr[18:12] 来索引，共128*8 = 1KB
// 第2、3级表为动态分配内存

typedef struct l3_entry_t
{
    uint8_t valid; // 本级表的offset 为[27:19]
    uint8_t *addr; //下一级表，offset为[18:12]
}L3Entry;

typedef struct l2_entry_t
{
    uint8_t valid; // 本级表的offset 为[27:19]
    L3Entry *l3_table; //下一级表，offset为[18:12]
}L2Entry;

typedef struct l1_entry_t
{
    uint8_t valid; // 本级表的offset 为[31:28]
    L2Entry *l2_table; //下一级表，offset为[27:19]
} L1Entry;

// 创建第一级表，常驻内存
static L1Entry l1_table [L1_LEN];

static L2Entry* create_l2_table(L1Entry* l1_table){
    // 创建并初始化L2 table
    L2Entry* l2_table = (L2Entry*)malloc(sizeof(L2Entry)*L2_LEN);
    for (uint32_t i = 0; i < L2_LEN; i++)
    {
        l2_table[i].valid = 0;
        l2_table[i].l3_table = NULL;
    }

    l1_table->l2_table = l2_table;
    l1_table->valid = 1;

    if (l2_table != NULL)
    {
        l2_table_size +=1;
    }

    return l2_table;
}

static L3Entry* create_l3_table(L2Entry* l2_table){
    // 创建并初始化L3 table
    L3Entry* l3_table = (L3Entry*)malloc(sizeof(L3Entry)*L3_LEN);
    for (uint32_t i = 0; i < L3_LEN; i++)
    {
        l3_table[i].valid = 0;
        l3_table[i].addr = NULL;
    }

    l2_table->l3_table = l3_table;
    l2_table->valid = 1;

    if (l3_table != NULL)
    {
        l3_table_size +=1;
    }

    return l3_table;
}

static uint8_t* add_new_mem(){
    uint8_t* new_mem = (uint8_t*)malloc(sizeof(uint8_t)*ENTRY_SIZE);
    mem_pool_size += 1;
    return new_mem;
}

uint8_t *mem_pool_lkup(uint64_t addr)
{
    uint32_t l1_addr = ALIGN32_L1(addr);
    uint32_t l2_addr = ALIGN32_L2(addr);
    uint32_t l3_addr = ALIGN32_L3(addr);
    L2Entry *l2_table;
    L3Entry *l3_table;

    if (l1_table[l1_addr].valid)                            // l1 table hit
    {
        l2_table= l1_table[l1_addr].l2_table;               // 拿到L2 table
        if (l2_table[l2_addr].valid)                        // L2 table hit
        {
            l3_table = l2_table[l2_addr].l3_table;          // 拿到L3 TABLE
            if (l3_table[l3_addr].valid == 0)                // L3 table miss
            {
                l3_table[l3_addr].addr = add_new_mem();
                l3_table[l3_addr].valid = 1;
            }
            return l3_table[l3_addr].addr;                  // 返回最终地址

        } else {                                            // L2 table miss
            l3_table = create_l3_table(&l2_table[l2_addr]);  // 创建新的L3 table
            l3_table[l3_addr].addr = add_new_mem();         // 申请新的内存空间
            l3_table[l3_addr].valid = 1;
            return l3_table[l3_addr].addr;
        }
    } else {                                            // l1 table miss
        l2_table = create_l2_table(&l1_table[l1_addr]);  // 创建新的L2 table
        l3_table = create_l3_table(&l2_table[l2_addr]);  // 创建新的L3 table
        l3_table[l3_addr].addr = add_new_mem();         // 申请新的内存空间
        l3_table[l3_addr].valid = 1;
        return l3_table[l3_addr].addr;
    }

}

void mem_pool_init()
{
    // 初始化第一级页表，内容为空
    for (uint32_t i = 0; i < L1_LEN; i++)
    {
        l1_table[i].valid = 0;
        l1_table[i].l2_table = NULL;
    }
    mem_pool_size = 0;
    l2_table_size = 0;
    l3_table_size = 0;
}

void l3_table_free(L3Entry *l3_table){
    for (int i = 0; i < L3_LEN; i++)
    {
        if (l3_table[i].valid)
        {
            free(l3_table[i].addr);
        }

    }
}

void l2_table_free(L2Entry *l2_table){
    for (int i = 0; i < L2_LEN; i++)
    {
        if (l2_table[i].valid)
        {
            l3_table_free(l2_table[i].l3_table);
            free(l2_table[i].l3_table);
        }
    }
}

void l1_table_free(){
    for (int i = 0; i < L1_LEN; i++)
    {
        if (l1_table[i].valid)
        {
            l2_table_free(l1_table[i].l2_table);
            free(l1_table[i].l2_table);
        }
    }
}

void mem_pool_free()
{
    // 遍历内存池，销毁所有内存对象，并将所有表项清空
    l1_table_free();
}

uint64_t get_mem_pool_size()
{
    return mem_pool_size * 4;
}

uint64_t get_l2_table_size()
{
    return l2_table_size * sizeof(L2Entry)*L2_LEN;
}

uint64_t get_l3_table_size()
{
    return l3_table_size * sizeof(L3Entry)*L2_LEN;
}

#undef ALIGN32_L1
#undef ALIGN32_L2
#undef ALIGN32_L3
#undef L1_IDX_LEN
#undef L2_IDX_LEN
#undef L3_IDX_LEN
#undef L1_LEN
#undef L2_LEN
#undef L3_LEN
