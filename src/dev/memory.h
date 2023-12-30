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


#ifndef __MEMORY_H__
    #define __MEMORY_H__

#include <stdint.h>

// Memory Map
    #define DRAM1MB    (1<<20)
    #define DRAM128MB  DRAM1MB * 128

    #define DRAM_BASE 0x80000000
    #define DRAM_END  DRAM_BASE + DRAM128MB - 1



// End Memory Map

typedef enum {
    CPU_FE  = 0, // Front End in CPU
    CPU_BE  = 1, // Back End in CPU
    CPU_MMU = 2, // MMU in CPU
    DEV     = 3  // Device
} MemOpSrc;

typedef struct mem_port
{
    MemOpSrc op_src;     // 操作源
    uint8_t  port_width; // Port的宽度（byte数）
} MemPort;

void memory_init(uint64_t mem_size);
void memory_free();

// 主存读操作
// addr: 读地址，无符号数，位宽为64 bits
// byte_num：读取的（byte）数量
// data_buf: 数据buf，由master指定
// op_src: 操作来源
// Return：0 失败，other：成功，且读数据的数量
int read_data(uint64_t addr, uint8_t byte_num, MemOpSrc op_src, uint8_t* data_buf);

// 主存写操作
// addr: 写地址，无符号数，位宽为64 bits
// byte_num：写取的（byte）数量
// data_buf: 数据buf，由master指定
// op_src: 操作来源
// Return：0 失败，other：成功，且写入数据的数量
int write_data(uint64_t addr, uint8_t byte_num, MemOpSrc op_src, uint8_t* data_buf);

// 0：无错误
// 1：访问了不存在的地址
// 2：对于某些设备，发生了地址不对齐的访问
// 3：跨设备访问
uint32_t get_ifu_fault();
// 0：无错误
// 1：访问了不存在的地址
// 2：对于某些设备，发生了地址不对齐的访问
// 3：跨设备访问
// 4：写只读地址
// 5：对只执行的地址进行读取
uint32_t get_lsu_fault();
// 0：无错误
// 1：访问了不存在的地址
// 2：对于某些设备，发生了地址不对齐的访问
// 3：跨设备访问
// 5：对只执行的地址进行读取
uint32_t get_mmu_fault();


#endif //__MEMORY_H__