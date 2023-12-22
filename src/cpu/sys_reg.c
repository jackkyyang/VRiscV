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

// Machine Status Registers
static struct mstatus_t
{
    uint32_t wpri0  :1;     // bit 0
    uint32_t sie    :1;     // bit 1
    uint32_t wpri2  :1;     // bit 2
    uint32_t mie    :1;     // bit 3
    uint32_t wpri4  :1;     // bit 4
    uint32_t spie   :1;     // bit 5
    uint32_t ube    :1;     // bit 6, 只支持小端，read-only 0
    uint32_t mpie   :1;     // bit 7
    uint32_t spp    :1;     // bit 8, 不实现S模式，read-only 0
    uint32_t vs     :2;     // bit 10:9, WARL fields, VPU 状态，暂时不实现，tie0
    uint32_t mpp    :2;     // bit 12:11, WARL字段，因此读取的值只能是合法值，不能出现S模式
    // TODO, FPU 状态，暂时不实现，tie0
    uint32_t fs     :2;     // bit 14:13, WARL fields
    uint32_t xs     :2;     // bit 16:15, 用户扩展状态，没有实现，因此read-only 0
    // MPRV用来控制M模式的页表翻译过程
    // 如果为1：则使用mpp中模式的页表翻译
    // 如果为0：则使用M模式的页表
    // 在MRET的时候，MPRV要清零
    uint32_t mprv   :1;     // bit 17
    // SUM (permit Supervisor User Memory access) bit
    // 用来控制S模式下的软件是否可以访问U模式下的地址空间
    // 没实现S模式时，SUM是read-only 0
    uint32_t sum    :1;     // bit 18, read-only 0
    // MXR (Make eXecutable Readable) 用来控制可执行地址是否可以通过load指令读取
    // 如果为1，load指令可以读取executable但是unreadable的地址
    // 没实现S模式时，MXR是read-only 0
    uint32_t mxr    :1;     // bit 19, read-only 0
    // 没实现S模式时，TVM是read-only 0
    uint32_t tvm    :1;     // bit 20, read-only 0
    // 当TW为0的时候，允许在低权限模式下执行WFI指令
    // 当TW为1的时候，不允许在M以下的模式中执行WFI，要报 illegal-instruction exception
    uint32_t tw     :1;     // bit 21, TW (Timeout Wait)
    //(Trap SRET) bit, 没实现S模式时，TSR 是read-only 0
    uint32_t tsr    :1;     // bit 22, read-only 0
    uint32_t wpri23 :8;     // bit 30:23
    uint32_t sd     :1;     // bit 31,
} mstatus = {0};

static struct mstatush_t
{
    uint32_t wpri0  :4;     // bit 4:0
    // If S-mode is not supported, SBE is read-only 0
    uint32_t sbe    :1;     // bit 4
    // 对于只支持小端的机器，mbe，sbe和ube都是 read-only 0
    uint32_t mbe    :1;     // bit 5 read-only 0
    uint32_t wpri6  :26;    // bit 31:6
} mstatush = {0};

// Machine Trap-Vector Base-Address Register
static struct mtvec_t
{
    // 0: 所有异常都使用同一个入口
    // 1：异常向量，所有异步中断使用Base + 4 x cause（每个中断只能有一条指令的空间）
    uint32_t mode   :2;
    uint32_t base   :30;
} mtvec;

// Machine Trap Delegation Registers
// 可以实现exception的委派，从而减少调用开销
// 可以将低于M mode的中断委派给Smode
// 每bit都对应着mcause中的synchronous exceptions的位置
// 没有实现S模式时，该寄存器不存在
static uint32_t medeleg;
// mideleg holds trap delegation bits for individual interrupts
// 每一bit对应着mip中的中断内容
static uint32_t mideleg;

// Machine Interrupt Registers
static struct mip_t
{
// 每bit为1的时候，表示mcause中对应中断号的中断正在pending
    uint32_t res0   :1; // bit0
    uint32_t ssip   :1; // bit1, supervisor-level software interrupts
    uint32_t res2   :1; // bit2
    // msip is written by accesses to memory-mapped control registers,
    // which are used by remote harts to provide machine-level interprocessor interrupts
    // If a system has only one hart, msip 和 msie 都是read-only 0
    uint32_t msip   :1; // bit3, read-only, Machine-level software interrupts
    uint32_t res4   :0; // bit4
    uint32_t stip   :0; // bit5, supervisor-level timer interrupts
    uint32_t res6   :0; // bit6
    uint32_t mtip   :0; // bit7, read-only, Machine timer interrupts
    uint32_t res8   :0; // bit8
    uint32_t seip   :0; // bit9, supervisor-level external interrupts
    uint32_t res9   :0; // bit10
    uint32_t meip   :0; // bit11, read-only, Machine-level external interrupts
    uint32_t res12  :4; // bit15:12
    uint32_t usr    :16;// bit31:16
} mip;
static struct mie_t
{
// 每bit为1的时候，表示mcause中对应中断号的中断使能
    uint32_t res0   :1; // bit0
    uint32_t ssie   :1; // bit1, supervisor-level software interrupts
    uint32_t res2   :1; // bit2
    // If a system has only one hart, msip 和 msie 都是read-only 0
    uint32_t msie   :1; // bit3, Machine-level software interrupts
    uint32_t res4   :0; // bit4
    uint32_t stie   :0; // bit5, supervisor-level timer interrupts
    uint32_t res6   :0; // bit6
    uint32_t mtie   :0; // bit7, Machine timer interrupts
    uint32_t res8   :0; // bit8
    uint32_t seie   :0; // bit9, supervisor-level external interrupts
    uint32_t res9   :0; // bit10
    uint32_t meie   :0; // bit11, Machine-level external interrupts
    uint32_t res12  :4; // bit15:12
    uint32_t usr    :16;// bit31:16
} mie;
// 当所有条件满足时，中断i被路由到M模式
// 1. 当前特权模式为 M 并且设置了 mstatus 寄存器中的 MIE 位，或者当前特权模式的特权低于 M 模式
// 2. bit i在mip和mie中都为1
// 3. mideleg存在的同时，mideleg的第i比特不为1
// 以上条件必须在执行xRET或者改变和中断有关的CRS后立刻评估
// 路由到M模式的中断比到低权限中断的优先机高
// 当有多个中断同时发生时，按以下的优先级take： MEI, MSI, MTI, SEI, SSI, STI.






// 读写操作

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

