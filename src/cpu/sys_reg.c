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
#include <assert.h>
#include "sys_reg.h"
#include "cpu_glb.h"
#include "../dev/clock.h"
#include "cpu_config.h"
#include "../include/comm.h"

//---------------------------------
// M mode csr registers
//---------------------------------
// Machine ISA Register
static struct misa_t
{
    MXLEN_T a        : 1; // Atomic
    MXLEN_T b        : 1;
    MXLEN_T c        : 1; // Compressed
    MXLEN_T d        : 1; // Double-precision fp
    MXLEN_T e        : 1; // RV32/64 E
    MXLEN_T f        : 1; // Single-precision fp
    MXLEN_T g        : 1;
    MXLEN_T h        : 1; // Hypervisor
    MXLEN_T i        : 1; // Integer
    MXLEN_T j        : 1;
    MXLEN_T k        : 1;
    MXLEN_T l        : 1;
    MXLEN_T m        : 1; // Integer Multiply/Divide
    MXLEN_T n        : 1;
    MXLEN_T o        : 1;
    MXLEN_T p        : 1;
    MXLEN_T q        : 1;
    MXLEN_T r        : 1;
    MXLEN_T s        : 1; // Spuervisor mode
    MXLEN_T t        : 1;
    MXLEN_T u        : 1; // User mode
    MXLEN_T v        : 1; // Vector
    MXLEN_T w        : 1;
    MXLEN_T x        : 1;
    MXLEN_T y        : 1;
    MXLEN_T z        : 1;
    MXLEN_T reserved : (MXLEN - 28);
    MXLEN_T mxl      :2; // Machine XLEN
} misa;

// Machine Vendor ID Register
// the value of 0 can be returned to
// indicate the field is not implemented
// or that this is a non-commercial implementation.
static const uint32_t mvendorid = 0;

// Machine Architecture ID Register
// 微架构 ID， 用于确认某个厂商的具体的微架构
// 这里我们自行设置为 1
static const MXLEN_T marchid = 1;

// Machine Implementation ID Register
// 处理器实现ID，用于标记处理器核的设计版本
// 这里写0
static const MXLEN_T mimpid = 0;

// Hart ID Register
// 硬件线程ID，必须要有一个0
// TODO,后续如果支持硬件多线程，这个寄存器需要根据线程数可配
static const MXLEN_T mhartid = 0;

// Machine Status Registers
static struct mstatus_t
{
    uint32_t wpri0  :1;     // bit 0
    uint32_t sie    :1;     // bit 1, 不实现S模式，read-only 0
    uint32_t wpri2  :1;     // bit 2
    uint32_t mie    :1;     // bit 3, reset to 0
    uint32_t wpri4  :1;     // bit 4
    uint32_t spie   :1;     // bit 5, 不实现S模式，read-only 0
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
    uint32_t mprv   :1;     // bit 17,  reset to 0
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
    MXLEN_T mode   :2;
    MXLEN_T base   :(MXLEN-2);
} mtvec;

#ifdef S_MODE
// Machine Trap Delegation Registers
// 可以实现exception的委派，从而减少调用开销
// 可以将低于M mode的中断委派给Smode
// 每bit都对应着mcause中的synchronous exceptions的位置
// 没有实现S模式时，该寄存器不存在
static MXLEN_T medeleg;
// mideleg holds trap delegation bits for individual interrupts
// 每一bit对应着mip中的中断内容
static MXLEN_T mideleg;
#endif

// Machine Interrupt Registers
static struct mip_t
{
// 每bit为1的时候，表示mcause中对应中断号的中断正在pending
    MXLEN_T res0   :1; // bit0
    MXLEN_T ssip   :1; // bit1, supervisor-level software interrupts
    MXLEN_T res2   :1; // bit2
    // msip is written by accesses to memory-mapped control registers,
    // which are used by remote harts to provide machine-level interprocessor interrupts
    // 如果系统里只有一个核, msip 和 msie 都是read-only 0
    MXLEN_T msip   :1; // bit3, Read-Only, Machine-level software interrupts
    MXLEN_T res4   :1; // bit4
    MXLEN_T stip   :1; // bit5, supervisor-level timer interrupts
    MXLEN_T res6   :1; // bit6
    MXLEN_T mtip   :1; // bit7, Read-Only, Machine timer interrupts
    MXLEN_T res8   :1; // bit8
    MXLEN_T seip   :1; // bit9, supervisor-level external interrupts
    MXLEN_T res9   :1; // bit10
    MXLEN_T meip   :1; // bit11, Read-Only, Machine-level external interrupts
    MXLEN_T res12  :4; // bit15:12
    MXLEN_T usr    :16;// bit31:16
} mip;
static struct mie_t
{
// 每bit为1的时候，表示mcause中对应中断号的中断使能
    MXLEN_T res0   :1; // bit0
    MXLEN_T ssie   :1; // bit1, supervisor-level software interrupts
    MXLEN_T res2   :1; // bit2
    // 如果系统里只有一个核, msip 和 msie 都是read-only 0
    MXLEN_T msie   :1; // bit3, Machine-level software interrupts
    MXLEN_T res4   :1; // bit4
    MXLEN_T stie   :1; // bit5, supervisor-level timer interrupts
    MXLEN_T res6   :1; // bit6
    MXLEN_T mtie   :1; // bit7, Machine timer interrupts
    MXLEN_T res8   :1; // bit8
    MXLEN_T seie   :1; // bit9, supervisor-level external interrupts
    MXLEN_T res9   :1; // bit10
    MXLEN_T meie   :1; // bit11, Machine-level external interrupts
    MXLEN_T res12  :4; // bit15:12
    MXLEN_T usr    :16;// bit31:16
} mie;
// 当所有条件满足时，中断i被路由到M模式
// 1. 当前特权模式为 M 并且设置了 mstatus 寄存器中的 MIE 位，或者当前特权模式的特权低于 M 模式
// 2. bit i在mip和mie中都为1
// 3. mideleg存在的同时，mideleg的第i比特不为1
// 以上条件必须在执行xRET或者改变和中断有关的CRS后立刻评估
// 路由到M模式的中断比到低权限中断的优先机高
// 当有多个中断同时发生时，按以下的优先级take： MEI, MSI, MTI, SEI, SSI, STI.
// MEIP,MTIP,MSIP都是read-only的寄存器，意味着清除pending的责任在外部设备

//--------------------------------------------
// Hardware Performance Monitor
// privileged Counter/Timers
// cycle,instret 和 hpmcountern 实际上是 mcycle，minstret和mhpmcountern的别名
// time是memory-mapped寄存器mtime的别名
// 因此读取time的csr指令可以被硬件翻译成load指令，也可以被trap到M模式处理


// mcycle 在reset的时候可以是任何值
// 可以被软件写入一个值，并继续计数
// 可以被mcountinhibit控制暂停计数
// 可以在多个core之间share，此时通过软件写入或者暂停都会同时影响多个core
// TODO，暂时为只读信号，不能暂停，后面加入多线程时，再开发写入和暂停功能

// 返回cycle的低32bit
static MXLEN_T mcycle(){
    return (MXLEN_T)(get_clock());
}

// Instruction-retired Counter
static uint64_t ins_retired_cnt = 0;
void instreth_inc(uint64_t ins_num)
{
    ins_retired_cnt += ins_num;
}

// hardware performance monitor
static uint64_t mhpmcounter [29]; // 硬件性能计数器

#ifndef RV64
// 返回cycle的高32bit，只在RV32有效
static uint32_t mcycleh(){
    return (MXLEN_T)(get_clock() >> 32);
}
#endif

// event 选择器，为对应的counter选择要监听的事件
// event基本是自定义的，但是要求0号event必须是no event，意味着counter不需要计数
static MXLEN_T mhpmevent   [29];

// Machine Counter-Enable Register
// 控制对应的counter是否能在S/U模式下被访问
static struct mcounteren_t
{
    uint32_t cy     :   1; // enable for cycle in S/U mode
    uint32_t tm     :   1; // enable for timer in S/U mode
    uint32_t ir     :   1; // enable for instret in S/U mode
    uint32_t hpm3   :   1; // enable for hpmcounter3 in S/U mode
    uint32_t hpm4   :   1;
    uint32_t hpm5   :   1;
    uint32_t hpm6   :   1;
    uint32_t hpm7   :   1;
    uint32_t hpm8   :   1;
    uint32_t hpm9   :   1;
    uint32_t hpm10  :   1;
    uint32_t hpm11  :   1;
    uint32_t hpm12  :   1;
    uint32_t hpm13  :   1;
    uint32_t hpm14  :   1;
    uint32_t hpm15  :   1;
    uint32_t hpm16  :   1;
    uint32_t hpm17  :   1;
    uint32_t hpm18  :   1;
    uint32_t hpm19  :   1;
    uint32_t hpm20  :   1;
    uint32_t hpm21  :   1;
    uint32_t hpm22  :   1;
    uint32_t hpm23  :   1;
    uint32_t hpm24  :   1;
    uint32_t hpm25  :   1;
    uint32_t hpm26  :   1;
    uint32_t hpm27  :   1;
    uint32_t hpm28  :   1;
    uint32_t hpm29  :   1;
    uint32_t hpm30  :   1;
    uint32_t hpm31  :   1;
} mcounteren;

// Machine Counter-Enable Register
// 控制对应的counter是否继续递增
static struct mcountinhibit_t
{
    uint32_t cy     :   1; // counter-inhibit for cycle in S/U mode
    uint32_t tm     :   1; // counter-inhibit for timer in S/U mode
    uint32_t ir     :   1; // counter-inhibit for instret in S/U mode
    uint32_t hpm3   :   1; // counter-inhibit for hpmcounter3 in S/U mode
    uint32_t hpm4   :   1;
    uint32_t hpm5   :   1;
    uint32_t hpm6   :   1;
    uint32_t hpm7   :   1;
    uint32_t hpm8   :   1;
    uint32_t hpm9   :   1;
    uint32_t hpm10  :   1;
    uint32_t hpm11  :   1;
    uint32_t hpm12  :   1;
    uint32_t hpm13  :   1;
    uint32_t hpm14  :   1;
    uint32_t hpm15  :   1;
    uint32_t hpm16  :   1;
    uint32_t hpm17  :   1;
    uint32_t hpm18  :   1;
    uint32_t hpm19  :   1;
    uint32_t hpm20  :   1;
    uint32_t hpm21  :   1;
    uint32_t hpm22  :   1;
    uint32_t hpm23  :   1;
    uint32_t hpm24  :   1;
    uint32_t hpm25  :   1;
    uint32_t hpm26  :   1;
    uint32_t hpm27  :   1;
    uint32_t hpm28  :   1;
    uint32_t hpm29  :   1;
    uint32_t hpm30  :   1;
    uint32_t hpm31  :   1;
} mcountinhibit;

// Machine Scratch Register
// 可以用来保存M模式下的栈指针
// 可以在进程切换的时候，保存和恢复机器模式的上下文
static MXLEN_T mscratch;

// Machine Exception Program Counter
// 在陷入M模式的时候，将当前的PC保存到该寄存器中
static MXLEN_T mepc;

// Machine Cause Register
// 进入M模式时，记录事件号
// M模式下软件可以读写
static struct mcause_t
{
    MXLEN_T exception_code : (MXLEN-1); // reset to a reset indication
    MXLEN_T interrupt : 1;
} mcause = {0};

// Machine Trap Value Register
// 用于trap 进M模式时，写入一些和exception相关的信息，从而帮助软件定位问题
// 1. 在发生misaligned load或store导致的access fault或者page-fault时
//    保存了触发错误的虚拟地址
// 2. 在取指过程中触发了access fault或者page-fault时
//    保存了触发错误的指令的地址，mepc则是取指地址的首地址，两者不一定相同
// 3. 在发生illegal instruction exception时，保存第一个非法指令的内容
static MXLEN_T mtval;

// Machine Configuration Pointer Register
// 指向configuration data的指针，在RV32下必须是4byte对齐的
// 软件可以通过这个指针来遍历硬件配置
// 如果为0，说明没有这个配置结构体，软件需要依赖其它方式
static MXLEN_T mconfigptr;

// Machine Environment Configuration Register
// 用来向低级别的模式提供环境配置信息
// 暂时不需要
static struct menvcfg_t
{
    uint64_t fiom   :1; // read-only 0
    uint64_t res1   :3;
    uint64_t cbie   :2;
    uint64_t cbcfe  :1;
    uint64_t cbze   :1;
    uint64_t res8   :54;
    uint64_t pbmte  :1;  // read-only 0
    uint64_t stce   :1;  //
} menvcfg;

// Machine Security Configuration Register
// 和安全特性有关的配置信息
// 暂时不需要
// static struct mseccfg_t
// {
//     uint64_t mml    :1;
//     uint64_t mmwp   :1;
//     uint64_t rlb    :1;
//     uint64_t res3   :5;
//     uint64_t useed  :1;
//     uint64_t sseed  :1;
//     uint64_t res10  :54;
// } mseccfg;

//---------------------------------
// U mode csr registers
//---------------------------------
// Unprivileged Floating-Point CSRs
static uint32_t fcrs = 0;
static uint32_t fflags(){
    return (fcrs & 0x1F);
}
static uint32_t frm(){
    return (fcrs & 0xE0) >> 5;
}

// Unprivileged Counter/Timers
// cycle
static MXLEN_T cycle(){
    return mcycle();
}

// time
static MXLEN_T time(){
    return (MXLEN_T)(get_time());
}

static MXLEN_T instret() {
    return (MXLEN_T)ins_retired_cnt;
}

#ifndef RV64
static uint32_t cycleh(){
    return mcycleh();
}
static uint32_t timeh(){
    return (uint32_t)(get_time() >> 32);
}
static uint32_t instreth() {
    return (uint32_t)(ins_retired_cnt >> 32);
}
#endif

static uint64_t hpmcounter(int no){
    uint32_t counten = STRUCT2INT(uint32_t,mcounteren);
    if (!PICK_BIT(no,counten))
    {
        // TODO, raise_iinstr_excp(XXX);
        return 0;
    }

    return mhpmcounter[no - 3];
}

// Reset
void sys_reg_reset()
{
    mstatus.mie = 0;
    mstatus.mprv = 0;
    mstatush.mbe = 0;
    misa.a       =1;     //a
    misa.b       =0;     //b
    misa.c       =0;     //c
    misa.d       =0;     //d
    misa.e       =0;     //e
    misa.f       =0;     //TODO, f
    misa.g       =0;     //g
    misa.h       =0;     //h
    misa.i       =1;     //i
    misa.j       =0;     //j
    misa.k       =0;     //k
    misa.l       =0;     //l
    misa.m       =0;     //m
    misa.n       =0;     //n
    misa.o       =0;     //o
    misa.p       =0;     //p
    misa.q       =0;     //q
    misa.r       =0;     //r
    misa.s       =0;     //s
    misa.t       =0;     //t
    misa.u       =1;     //u, User mode
    misa.v       =0;     //v
    misa.w       =0;     //w
    misa.x       =0;     //x
    misa.y       =0;     //y
    misa.z       =0;     //z
    misa.reserved=0;     //reserved
    misa.mxl     =1;    //xlen = 32
}

//-------------------------------
// CRS 读写操作
//-------------------------------
int csr_read(uint32_t csr, MXLEN_T *rdptr){

    uint16_t csr_addr = ((uint16_t)csr) & 0xfff;
    //-----------------------
    // U mode
    //-----------------------
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
        *rdptr = cycle();
        break;
    case 0xC01:
        *rdptr = time();
        break;
    case 0xC02:
        *rdptr = instret();
        break;
    case 0xC03: // hpmcounter3, Performance-monitoring counter.
    case 0xC04: // hpmcounter4, Performance-monitoring counter
    case 0xC05:
    case 0xC06:
    case 0xC07:
    case 0xC08:
    case 0xC09:
    case 0xC0a:
    case 0xC0b:
    case 0xC0c:
    case 0xC0d:
    case 0xC0e:
    case 0xC0f:
    case 0xC10:
    case 0xC11:
    case 0xC12:
    case 0xC13:
    case 0xC14:
    case 0xC15:
    case 0xC16:
    case 0xC17:
    case 0xC18:
    case 0xC19:
    case 0xC1a:
    case 0xC1b:
    case 0xC1c:
    case 0xC1d:
    case 0xC1e:
    case 0xC1f:
        *rdptr = 0;
        break;
    case 0xC80:
        *rdptr = cycleh();
        break;
    case 0xC81:
        *rdptr = timeh();
        break;
    case 0xC82:
        *rdptr = instreth();
        break;
    case 0xC83: // Upper 32 bits of hpmcounter3, RV32 only.
    case 0xC84: // Upper 32 bits of hpmcounter4, RV32 only.
    case 0xC85:
    case 0xC86:
    case 0xC87:
    case 0xC88:
    case 0xC89:
    case 0xC8a:
    case 0xC8b:
    case 0xC8c:
    case 0xC8d:
    case 0xC8e:
    case 0xC8f:
    case 0xC90:
    case 0xC91:
    case 0xC92:
    case 0xC93:
    case 0xC94:
    case 0xC95:
    case 0xC96:
    case 0xC97:
    case 0xC98:
    case 0xC99:
    case 0xC9a:
    case 0xC9b:
    case 0xC9c:
    case 0xC9d:
    case 0xC9e:
    case 0xC9f:
        *rdptr = 0;
        break;
    //-----------------------
    // M mode
    //-----------------------
    case 0xF11:
        *rdptr = (MXLEN_T)mvendorid;
        break;
    case 0xF12:
        *rdptr = marchid;
        break;
    case 0xF13:
        *rdptr = mimpid;
        break;
    case 0xF14:
        *rdptr = mhartid;
        break;
    case 0xF15:
        *rdptr = mconfigptr;
        break;

    // Machine Trap Setup
    case 0x300:
        *rdptr = STRUCT2INT(MXLEN_T,mstatus);
        break;
    case 0x301:
        *rdptr = STRUCT2INT(MXLEN_T,misa);
        break;
    case 0x302:
        *rdptr = 0; // medeleg
        break;
    case 0x303:
        *rdptr = 0; // mideleg
        break;
    case 0x304:
        *rdptr = STRUCT2INT(MXLEN_T,mie);
        break;
    case 0x305:
        *rdptr = STRUCT2INT(MXLEN_T,mtvec);
        break;
    case 0x306:
        *rdptr = STRUCT2INT(MXLEN_T,mcounteren);
        break;
    case 0x310:
        *rdptr = STRUCT2INT(MXLEN_T,mstatush);
        break;
    // Machine Trap Handling
    case 0x340:
        *rdptr = (MXLEN_T)mscratch;
        break;
    case 0x341:
        *rdptr = (MXLEN_T)mepc;
        break;
    case 0x342:
        *rdptr = STRUCT2INT(MXLEN_T,mcause);
        break;
    case 0x343:
        *rdptr = (MXLEN_T)mtval;
        break;
    case 0x344:
        *rdptr = STRUCT2INT(MXLEN_T,mip);
        break;
    case 0x34a:
        *rdptr = 0; // minst
        break;
    case 0x34b:
        *rdptr = 0; // mtval2
        break;
    // Machine Configuration
    case 0x30a:
        *rdptr = STRUCT2INT(MXLEN_T,menvcfg);
        break;
    case 0x31a:
        *rdptr = STRUCT2INT(uint64_t,menvcfg)>>32;
        break;
    case 0x747:
        *rdptr = 0; //mseccfg
        break;
    case 0x757:
        *rdptr = 0; //mseccfgh
        break;

    default:
        // 访问非法寄存器地址，返回1
        return 1;
    }

    return 0;
}

static void write_fflags(MXLEN_T wdata){
    uint32_t tmp_data = fcrs & 0xFFFFFFE0;
    fcrs = tmp_data | ((uint32_t)(wdata&0x1F));
}

static void write_frm(MXLEN_T wdata){
    uint32_t tmp_data = fcrs & 0xFFFFFF1F;
    fcrs = tmp_data | ((uint32_t)(wdata&0xE0));
}

static void write_mstatus(MXLEN_T wdata){
    // 只有部分字段可写，剩下的都是read-only 0
    struct mstatus_t tmp_reg = INT2STRUCT(struct mstatus_t,wdata);
    mstatus.sie     = tmp_reg.sie;
    mstatus.mie     = tmp_reg.mie;
    mstatus.spie    = tmp_reg.spie;
    mstatus.mpie    = tmp_reg.mpie;
    #ifndef S_MODE
        //没实现S模式，因此不能向MPP中写入1或2
        if (tmp_reg.mpp == 1 || tmp_reg.mpp == 2)
            mstatus.mpp     = 0;
        else
            mstatus.mpp     = tmp_reg.mpp;
    #endif
    // mstatus.fs      = tmp_reg.fs;
    mstatus.mprv    = tmp_reg.mprv;
    mstatus.tw      = tmp_reg.tw;
    mstatus.sd      = tmp_reg.sd;
}


int csr_write(uint32_t csr, MXLEN_T wdata)
{
    uint16_t csr_addr = ((uint16_t)csr) & 0xfff;
    switch (csr_addr)
    {
    case 0x001:
        write_fflags(wdata);
        break;
    case 0x002:
        write_frm(wdata);
        break;
    case 0x003:
        fcrs = wdata;
        break;

    // M mode
    // Machine Trap Setup
    case 0x300:
        write_mstatus(wdata);
        break;
    case 0x301:
        // 架构上允许修改misa，软件可以禁用或开启某些特性
        // 但是在这里我们不允许软件修改这个寄存器，所有写操作会当作nop
        break;
    case 0x302:
        // medeleg
        break;
    case 0x303:
        // mideleg
        break;
    case 0x304:
        mie = INT2STRUCT(struct mie_t,wdata);
        break;
    case 0x305:
        mtvec = INT2STRUCT(struct mtvec_t,wdata);
        break;
    case 0x306:
        mcounteren = INT2STRUCT(struct mcounteren_t,wdata);
        break;
    case 0x310:
        // 当前配置只支持小端mstatush 是 read-only 0
        break;
    // Machine Trap Handling
    case 0x340:
        mscratch = wdata;
        break;
    case 0x341:
        mepc = wdata;
        break;
    case 0x342:
        mcause = INT2STRUCT(struct mcause_t,wdata);
        break;
    case 0x343:
        mtval = wdata;
        break;
    case 0x344:
        mip =INT2STRUCT(struct mip_t,wdata);
        break;

    // Machine Configuration
    // TODO, 需要把menvcfg拆开
    // case 0x30a:
    //     menvcfg = INT2STRUCT(struct menvcfg_t,wdata);
    //     break;
    // case 0x31a:
    //     menvcfg = INT2STRUCT(struct menvcfg_t);
    //     break;
    // case 0x747:
    //     //mseccfg
    //     break;
    // case 0x757:
    //     //mseccfgh
    //     break;
    default:
        // 访问非法寄存器地址，返回1
        return 1;
    }
    return 0;
}

//---------------------------------------------
// Trap 进M状态时的系统寄存器处理
// 硬件自动处理的部分
void trap2m(MXLEN_T interrupt,MXLEN_T e_code,CPUMode curr_mode){
    mcause.interrupt = interrupt;
    mcause.exception_code = e_code;

    ExeStatus *e_st = get_exe_st_ptr();
    e_st->exception = 1;
    // 保存异常现场
    // 异常PC
    mepc = e_st->curr_pc;
    // 异常信息，供软件使用
    assert(mtvec.mode == 0 || mtvec.mode == 1); // 其它值都是非法值
    // 异常路由
    if (mtvec.mode == 1)
    {
        if (interrupt == 1)
        {
            e_st->next_pc = (mtvec.base << 2) + (4 * e_code);
        }
        else
        {
            e_st->next_pc = (mtvec.base << 2);
        }
    } else if (mtvec.mode == 0)
    {
        e_st->next_pc = (mtvec.base << 2);
    }
    // 设置状态寄存器
    mstatus.mpie = mstatus.mie; // 保存之前的中断使能配置
    mstatus.mie = 0; // 关闭中断
    if (curr_mode == M)
    {
        mstatus.mpp  = 3;
    } else if (curr_mode == U)
    {
        mstatus.mpp  = 0;
    }
    // 进入M模式
    e_st->next_mode = M;
}

void ebreak_trap()
{
    CPUMode curr_mode = get_cpu_mode();
    trap2m(0,3,curr_mode);
}

void ecall_trap()
{
    CPUMode curr_mode = get_cpu_mode();
    if (curr_mode == U)
    {
        trap2m(0,8,curr_mode);
    }
    #ifdef S_MODE
    else if (curr_mode == M)
    {
        trap2m(0,9,curr_mode);
    }
    #endif
    else if (curr_mode == M)
    {
        trap2m(0,11,curr_mode);
    }
}

//
void raise_illegal_instruction(CPUMode curr_mode,MXLEN_T inst){
    trap2m(0,2,curr_mode);
    mtval = inst;
};


// Mret 的处理
void mret_proc(){
    ExeStatus *e_st = get_exe_st_ptr();
    mstatus.mie = mstatus.mpie;
    mstatus.mpie = 1;
    e_st->exception = 1;
    if (mstatus.mpp == 0)
    {
        e_st->next_mode = U;
        mstatus.mprv = 0;
    }
    #ifdef S_MODE
    else if (mstatus.mpp == 1)
    {
        e_st->next_mode = S;
        mstatus.mprv = 0;
    }
    #endif
    else if (mstatus.mpp == 3)
    {
        e_st->next_mode = M;
    }

    mstatus.mpp = 0; // 每次mret时都设置成最小的privilege level
    e_st->next_pc = mepc;
}