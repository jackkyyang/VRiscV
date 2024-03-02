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
// -------------------------------------------------------------------------------
// 功能
// 1. 管理内存地址空间，向CPU提供设备信息
// 2. 管理物理内存
// -------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "memory.h"
#include "mem_pool.h"
#include "dev_config.h"
#include "dev_bus.h"
#include "../include/comm.h"


static uint32_t ifu_fault;
static uint32_t lsu_fault;
static uint32_t mmu_fault;

uint32_t get_ifu_fault(){
    return ifu_fault;
}
uint32_t get_lsu_fault(){
    return lsu_fault;
}
uint32_t get_mmu_fault(){
    return mmu_fault;
}

typedef enum e_dev_acc {
    DRAM = 0,
    KBD = 1,
    SCR = 2,
    INTCTRL = 3
} MemDevID;

typedef struct mem_check
{
    uint8_t fault; // 是否存在memory 访问错误
    uint8_t across_page; // 是否跨页
    uint8_t byte_num; // 第一页中的访问的字节数
    uint64_t across_offset; // 跨页后的offset
    uint64_t next_page_addr; // 跨页后的地址
    MemDevID dev_target;
} MCheck;

void memory_init()
{
    mem_pool_init();
}

void memory_free()
{
    mem_pool_free();
}

// TODO, 非对齐访问检查
// 设备地址必须对齐到4byte
static inline int misalign_access_check(uint64_t addr, uint8_t byte_num){
    return (((int)addr & 0b11) > 0);
}

static MCheck addr_check(uint64_t addr, uint8_t byte_num, MemOpSrc op_src){
    // 检查地址范围是否越界
    // 对于某些设备，不允许不对齐的访问
    // 不允许跨设备进行访问
    uint64_t addr_tmp  = 0;
    MCheck addr_check = {0};

    // if (addr < DRAM_BASE || addr > DRAM_END || (addr + byte_num) > DRAM_END)
    // {
    //     addr_check.fault = 1;
    //     if (op_src == CPU_FE)
    //         ifu_fault = 1;
    //     else if (op_src == CPU_BE)
    //         lsu_fault = 1;
    //     else if (op_src == CPU_MMU)
    //         mmu_fault = 1;
    // }
    if (addr >= DRAM_BASE && addr <= DRAM_END) {
        addr_check.dev_target = DRAM;
        if ((addr + byte_num) > DRAM_END)
        {
            addr_check.fault = 1;
        }
    }
    else if (addr >= KBD_BASE && addr <= KBD_END)
    {
        addr_check.dev_target = KBD;
        if ((addr + byte_num) > KBD_END) {
            addr_check.fault = 1;
        }
        if (misalign_access_check(addr,byte_num))
        {
            addr_check.fault = 1;
        }

    }
    else if (addr >= SCR_BASE && addr <= SCR_END)
    {
        addr_check.dev_target = SCR;
        if ((addr + byte_num) > SCR_END) {
            addr_check.fault = 1;
        }
        if (misalign_access_check(addr,byte_num))
        {
            addr_check.fault = 1;
        }
    }
    else if (addr >= INTCTRL_BASE && addr <= INTCTRL_END)
    {
        addr_check.dev_target = INTCTRL;
        if ((addr + byte_num) > INTCTRL_END) {
            addr_check.fault = 1;
        }
        if (misalign_access_check(addr,byte_num))
        {
            addr_check.fault = 1;
        }
    }
    else {
        // 未知的设备
        addr_check.fault = 1;
    }


    addr_tmp = (addr & (uint64_t)(ENTRY_SIZE - 1)) + ((uint64_t)byte_num);
    if (addr_tmp > ENTRY_SIZE) {
        addr_check.across_page = 1;
        // 将地位mask，只看高位
        addr_check.next_page_addr = (addr & (~(uint64_t)(ENTRY_SIZE - 1))) + ENTRY_SIZE;
        addr_check.across_offset = addr_tmp - ENTRY_SIZE;
        addr_check.byte_num = ENTRY_SIZE - (addr & (uint64_t)(ENTRY_SIZE - 1));
    } else {
        addr_check.byte_num = byte_num;
    }
    return addr_check;
}


static uint8_t *get_mem_ptr(uint64_t addr){
    uint64_t page_addr = (uint64_t)ROUND(addr,ENTRY_SIZE);
    uint8_t *mem_addr = mem_pool_lkup(page_addr);
    if (mem_addr == NULL)
        return NULL;
    uint64_t offset = MOD(addr,ENTRY_SIZE);
    return (mem_addr + offset);
}

// DRAM 读取
static inline int read_dram(uint64_t addr,uint8_t *data_buf,MCheck mem_check){
    uint8_t *rd_ptr = get_mem_ptr(addr);
    assert(rd_ptr!=NULL);
    for (uint8_t i = 0; i < mem_check.byte_num; i++)
    {
        data_buf[i] = rd_ptr[i];
    }

    if (mem_check.across_page)
    {
        rd_ptr = get_mem_ptr(mem_check.next_page_addr);
        for (uint8_t i = 0; i < mem_check.across_offset; i++)
        {
            data_buf[mem_check.byte_num + i] = rd_ptr[i];
        }
    }
    return 0;
}

// DRAM 写
static inline int write_dram(uint64_t addr,uint8_t *data_buf,MCheck mem_check){
    uint8_t *wr_ptr = get_mem_ptr(addr);
    assert(wr_ptr!=NULL);
    for (uint8_t i = 0; i < mem_check.byte_num; i++)
    {
        wr_ptr[i] = data_buf[i] ;
    }

    if (mem_check.across_page)
    {
        wr_ptr = get_mem_ptr(mem_check.next_page_addr);
        for (uint8_t i = 0; i < mem_check.across_offset; i++)
        {
            wr_ptr[i] = data_buf[mem_check.byte_num + i];
        }
    }
    return 0;
}

int read_data(uint64_t addr, uint8_t byte_num, MemOpSrc op_src, uint8_t *data_buf)
{
    // 对于RV32中的load指令，有三种长度：1，2，4
    // 对于instruction fetch，长度是4的倍数
    // 对于跨页面边界的地址，需要拆分成多个read操作

    MCheck mem_check = addr_check(addr,byte_num,op_src);
    if (mem_check.across_page)
        assert((mem_check.byte_num + mem_check.across_offset) == byte_num);
    if (byte_num == 0 || mem_check.fault==1)
        return 1;

    // 总线 DeMux
    if (mem_check.dev_target == DRAM) {
        return read_dram(addr,data_buf,mem_check);
    }
    else if (mem_check.dev_target == KBD) {
        return read_kbd(addr,byte_num,data_buf);
    }
    else if (mem_check.dev_target == SCR) {
        return read_screen(addr,byte_num,data_buf);
    }
    else if (mem_check.dev_target == INTCTRL) {
        return read_int(addr,byte_num,data_buf);
    }
    else {
        return 1;
    }

}

int write_data(uint64_t addr, uint8_t byte_num, MemOpSrc op_src, uint8_t *data_buf)
{
    // 对于RV32中的store指令，有三种长度：1，2，4
    // 对于跨页面边界的地址，需要拆分成多个write操作

    MCheck mem_check = addr_check(addr,byte_num,op_src);
    if (mem_check.across_page)
        assert((mem_check.byte_num + mem_check.across_offset) == byte_num);
    if (byte_num == 0 || mem_check.fault==1)
        return 1;

    // 总线 DeMux
    if (mem_check.dev_target == DRAM) {
        return write_dram(addr,data_buf,mem_check);
    }
    else if (mem_check.dev_target == KBD) {
        return write_kbd(addr,byte_num,data_buf);
    }
    else if (mem_check.dev_target == SCR) {
        return write_screen(addr,byte_num,data_buf);
    }
    else if (mem_check.dev_target == INTCTRL) {
        return write_int(addr,byte_num,data_buf);
    }
    else {
        return 1;
    }
}
