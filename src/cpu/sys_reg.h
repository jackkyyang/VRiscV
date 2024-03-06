
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

#ifndef __SYS_REG_H__
    #define __SYS_REG_H__

    #include <stdint.h>
    #include "cpu_config.h"
    #include "cpu_glb.h"

    typedef struct csr_feild
    {
        uint16_t id : 8;
        uint16_t pri: 2;
        uint16_t acc: 2;
    }CSRFeild;

    // 执行读CSR操作
    // 如果执行失败，则返回非0值
    // 执行成功，返回0
    int csr_read(uint32_t csr, MXLEN_T *rdptr);

    // 执行写CSR操作
    // 如果执行失败，则返回非0值
    // 执行成功，返回0
    int csr_write(uint32_t csr,MXLEN_T wdata);

    // implicitly inc the counter
    void instreth_inc(uint64_t ins_num);

    void sys_reg_reset();
    void trap2m(MXLEN_T interrupt,MXLEN_T e_code,CPUMode curr_mode);
    void mret_proc();
    void raise_illegal_instruction(CPUMode curr_mode,MXLEN_T inst);
    MXLEN_T int_mask_proc(MXLEN_T int_id,CPUMode curr_mode);

    // Machine Interrupt Registers
    typedef struct mip_t
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
        MXLEN_T scr_int:1; // bit16
        MXLEN_T res17  :1; // bit17
        MXLEN_T kbd_int:1; // bit18
        MXLEN_T usr    :13;// bit31:19
    } MIP;
    void set_mip(MIP);

#endif //__SYS_REG_H__
