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
#include "sys_reg.h"
#include "../dev/clock.h"

//---------------------------------
// U mode csr registers
//---------------------------------
// Unprivileged Floating-Point CSRs
static const uint32_t fcrs = 0;
static uint32_t fflags(){
    return (fcrs & 0b11111);
}
static uint32_t frm(){
    return (fcrs & 0b11100000) >> 5;
}

// Unprivileged Counter/Timers
// cycle
static uint32_t cycle32l(){
    return (uint32_t)(get_clock());
}
static uint32_t cycle32h(){
    return (uint32_t)(get_clock() >> 32);
}
// time
static uint32_t time32l(){
    return (uint32_t)(get_time());
}
static uint32_t time32h(){
    return (uint32_t)(get_time() >> 32);
}
// Instruction-retired Counter
static uint64_t ins_retired_cnt = 0;
void instreth_inc(uint64_t ins_num)
{
    ins_retired_cnt += ins_num;
}

//---------------------------------
// M mode csr registers
//---------------------------------
// Machine ISA Register
// 架构上允许修改，软件可以禁用或开启某些特性
// 但是在这里我们不允许软件修改这个寄存器，所有写操作会当作nop
static const struct misa_t
{
    uint32_t a        : 1; // Atomic
    uint32_t b        : 1;
    uint32_t c        : 1; // Compressed
    uint32_t d        : 1; // Double-precision fp
    uint32_t e        : 1; // RV32/64 E
    uint32_t f        : 1; // Single-precision fp
    uint32_t g        : 1;
    uint32_t h        : 1; // Hypervisor
    uint32_t i        : 1; // Integer
    uint32_t j        : 1;
    uint32_t k        : 1;
    uint32_t l        : 1;
    uint32_t m        : 1; // Integer Multiply/Divide
    uint32_t n        : 1;
    uint32_t o        : 1;
    uint32_t p        : 1;
    uint32_t q        : 1;
    uint32_t r        : 1;
    uint32_t s        : 1; // Spuervisor mode
    uint32_t t        : 1;
    uint32_t u        : 1; // User mode
    uint32_t v        : 1; // Vector
    uint32_t w        : 1;
    uint32_t x        : 1;
    uint32_t y        : 1;
    uint32_t z        : 1;
    uint32_t reserved : 4;
    uint32_t mxl      :2; // Machine XLEN
} misa = {   1,     //a
             0,     //b
             0,     //c
             0,     //d
             0,     //e
             0,     //f
             0,     //g
             0,     //h
             1,     //i
             0,     //j
             0,     //k
             0,     //l
             0,     //m
             0,     //n
             0,     //o
             0,     //p
             0,     //q
             0,     //r
             0,     //s
             0,     //t
             1,     //u
             0,     //v
             0,     //w
             0,     //x
             0,     //y
             0,     //z
             0,     //reserved
             1};    // mxl

// Machine Vendor ID Register
// the value of 0 can be returned to
// indicate the field is not implemented
// or that this is a non-commercial implementation.
static const struct mvendorid_t
{
    uint32_t offset :7;
    uint32_t bank :25;
} mvendorid = {0,0};

// Machine Architecture ID Register
// 微架构 ID， 用于确认某个厂商的具体的微架构
// 这里我们自行设置为 1
static const uint32_t marchid = 1;

// Machine Implementation ID Register
// 处理器实现ID，用于标记处理器核的设计版本
// 这里写0
static const uint32_t mimpid = 0;

// Hart ID Register
// 硬件线程ID，必须要有一个0
// TODO,后续如果支持硬件多线程，这个寄存器需要根据线程数可配
static const uint32_t mhartid = 0;

int csr32_read(CSRFeild csr, uint32_t *rdptr){

    uint16_t csr_addr = (uint16_t)(csr);

    switch (csr_addr)
    {
    case 0x001:
        *rdptr = fflags();
        break;
    case 0x002:
        *rdptr = frm();
        break;
    case 0x003:
        *rdptr = fcrs;
        break;
    case 0xC00:
        *rdptr = cycle32l();
        break;
    case 0xC01:
        *rdptr = time32l();
        break;
    case 0xC02:
        *rdptr = (uint32_t)ins_retired_cnt;
        break;
    case 0xC03,
         0xC04,
         0xC05,
         0xC06,
         0xC07,
         0xC08,
         0xC09,
         0xC0a,
         0xC0b,
         0xC0c,
         0xC0d,
         0xC0e,
         0xC0f,
         0xC10,
         0xC11,
         0xC12,
         0xC13,
         0xC14,
         0xC15,
         0xC16,
         0xC17,
         0xC18,
         0xC19,
         0xC1a,
         0xC1b,
         0xC1c,
         0xC1d,
         0xC1e,
         0xC1f:
        *rdptr = 0;
        break;
    case 0xC80:
        *rdptr = cycle32h();
        break;
    case 0xC81:
        *rdptr = time32h();
        break;
    case 0xC82:
        *rdptr = (uint32_t)(ins_retired_cnt >> 32);
        break;
    case 0xC83,
         0xC84,
         0xC85,
         0xC86,
         0xC87,
         0xC88,
         0xC89,
         0xC8a,
         0xC8b,
         0xC8c,
         0xC8d,
         0xC8e,
         0xC8f,
         0xC90,
         0xC91,
         0xC92,
         0xC93,
         0xC94,
         0xC95,
         0xC96,
         0xC97,
         0xC98,
         0xC99,
         0xC9a,
         0xC9b,
         0xC9c,
         0xC9d,
         0xC9e,
         0xC9f:
        *rdptr = 0;
        break;


    default:
        return 1;
    }

    return 0;
}

int csr32_write(CSRFeild csr, uint32_t wdata)
{
    return 0;
}

