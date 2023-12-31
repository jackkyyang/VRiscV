
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

#endif //__SYS_REG_H__
