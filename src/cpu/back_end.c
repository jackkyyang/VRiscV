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
#include <stdio.h>
#include <string.h>

#include "cpu_config.h"
#include "front_end.h"
#include "back_end.h"


// 定义通用寄存器
static uint64_t x[32]; // 通用寄存器
static uint64_t pc;
// ----------------------------------------------
static void init_register(){
    // 初始化通用寄存器
    for (int i = 0; i < 32; i++)
    {
        x[i] = i;
    }
}

// flags
static ExeFlags flags;
static void clear_flags(){
    memset(&flags, 0, sizeof(ExeFlags));
}

#include "execution.h"

ExeStatus instruction_execute(ExeParam *exe_param)
{
    ExeStatus e_st = {0};
    e_st.curr_pc = exe_param->pc;
    pc           = exe_param->pc;

    // decode
    uint32_t inst  = *exe_param->fetch_data_buf;
    uint8_t opcode = (inst & 0x7F);
    uint8_t func7 = (inst & 0b11111110000000000000000000000000) >> 25;
    uint8_t func3 = (inst & 0b00000000000000000111000000000000) >> 12;
    uint8_t rd    = (inst & 0b00000000000000000000111110000000) >> 7;
    uint8_t rs1   = (inst & 0b00000000000011111000000000000000) >> 15;
    uint8_t rs2   = (inst & 0b00000001111100000000000000000000) >> 20;
    int32_t imm   = ((int32_t)(inst & 0b11111111111100000000000000000000)) >> 20;

    clear_flags();
    if (opcode == OP_32){
            if (func7 == MULDIV)//
            {
                if (func3 == 0b000)
                    mul(rd,rs1,rs2);
                else if (func3 == 0b001)
                    mulh(rd,rs1,rs2);
                else if (func3 == 0b010)
                    mulhsu(rd,rs1,rs2);
                else if (func3 == 0b011)
                    mulhu(rd,rs1,rs2);
                else if (func3 == 0b100)
                    div(rd,rs1,rs2);
                else if (func3 == 0b101)
                    divu(rd,rs1,rs2);
                else if (func3 == 0b110)
                    rem(rd,rs1,rs2);
                else if (func3 == 0b111)
                    remu(rd,rs1,rs2);
                else undef();

            }else if (func7 == 0b0000000)
            {
                if (func3 == 0b000)
                    add(rd,rs1,rs2);
                else if (func3 == 0b001)
                    sll(rd,rs1,rs2);
                else if (func3 == 0b010)
                    slt(rd,rs1,rs2);
                else if (func3 == 0b011)
                    sltu(rd,rs1,rs2);
                else if (func3 == 0b100)
                    xor(rd,rs1,rs2);
                else if (func3 == 0b101)
                    srl(rd,rs1,rs2);
                else if (func3 == 0b110)
                    or(rd,rs1,rs2);
                else if (func3 == 0b111)
                    and(rd,rs1,rs2);
                else undef();
            }
            else if (func7 == 0b0100000)
            {
                if (func3 == 0b000)
                    sub(rd,rs1,rs2);
                else if (func3 == 0b101)
                    sra(rd,rs1,rs2);
                else undef();
            }
            else undef();
        }
    else if (opcode == OP_IMM)
        {
            uint8_t shamt = (inst & 0b00000001111100000000000000000000) >> 20;
            uint8_t shift_imm = (inst & 0b11111110000000000000000000000000) >> 25;
            if (func3 == 0b000)
                addi(rd,rs1,imm);
            else if (func3 == 0b010)
                slti(rd,rs1,imm);
            else if (func3 == 0b011)
                sltiu(rd,rs1,imm);
            else if (func3 == 0b100)
                xori(rd,rs1,imm);
            else if (func3 == 0b110)
                ori(rd,rs1,imm);
            else if (func3 == 0b111)
                andi(rd,rs1,imm);
            // shift imm
            else if (func3 == 0b001 && shift_imm == 0b0000000)
                slli(rd,rs1,shamt);
            else if (func3 == 0b101 && shift_imm == 0b0000000)
                srli(rd,rs1,shamt);
            else if (func3 == 0b101 && shift_imm == 0b0100000)
                srai(rd,rs1,shamt);
            else if (func3 == 0b000 && rs1 == 0b00000,rd == 0b00000 &&imm == 0b000000000000)
                uop();
            else undef();
        }
    else if (opcode == BRANCH)
    {
        int32_t branch_imm = ((inst & 0b00000000000000000000111100000000) >> 7) |
                              ((inst & 0b01111110000000000000000000000000) >> 20) |
                              ((inst & 0b00000000000000000000000010000000) << 4) |
                              ((int32_t)(inst & 0b10000000000000000000000000000000) >> 19);
        if (func3 == 0b000)
            beq(rs1,rs2,branch_imm);
        else if (func3 == 0b001)
            bne(rs1,rs2,branch_imm);
        else if (func3 == 0b100)
            blt(rs1,rs2,branch_imm);
        else if (func3 == 0b101)
            bgt(rs1,rs2,branch_imm);
        else if (func3 == 0b110)
            bltu(rs1,rs2,branch_imm);
        else if (func3 == 0b111)
            bgeu(rs1,rs2,branch_imm);
        else undef();
    }
    else if (opcode == JAL)
    {
        int32_t jal_imm =((inst & 0b01111111111000000000000000000000) >> 20) |
                          ((inst & 0b00000000000100000000000000000000) >> 9) |
                          ((inst & 0b00000000000011111111000000000000) >> 0) |
                          ((int32_t)(inst & 0b10000000000000000000000000000000) >> 11);
        jal(rd,jal_imm);
    }
    else if (opcode == JALR && func3 == 0b000)
    {
        jalr(rd,rs1,imm);
    }
    else if (opcode == LOAD)
    {
        if (func3 == 0b000)
            lb(rd,rs1,imm);
        else if (func3 == 0b001)
            lh(rd,rs1,imm);
        else if (func3 == 0b010)
            lw(rd,rs1,imm);
        else if (func3 == 0b100)
            lbu(rd,rs1,imm);
        else if (func3 == 0b101)
            lhu(rd,rs1,imm);
        else undef();
    }
    else if (opcode == STORE)
    {
        int32_t str_imm = ((inst & 0b00000000000000000000111110000000) >> 7) |
                           ((int32_t)(inst & 0b11111110000000000000000000000000) >> 20);
        if (func3 == 0b000)
            sb(rs1,rs2,str_imm);
        else if (func3 == 0b001)
            sh(rs1,rs2,str_imm);
        else if (func3 == 0b010)
            sw(rs1,rs2,str_imm);
        else undef();
    }
    else if (opcode == LUI)
    {
        int32_t li_imm = (inst & 0b11111111111111111111000000000000);
        lui(rd,li_imm);
    }
    else if (opcode == LUI)
    {
        int32_t li_imm = (inst & 0b11111111111111111111000000000000);
        auipc(rd,li_imm);
    }
    else if (opcode == SYSTEM)
    {
        if (func3 == 0b000)
        {
            if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b000000000000)
                ecall();
            else if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b000000000001)
                ebreak();
        }
        else if(func3 == 0b001)
            csrrw(rd,rs1,imm);
        else if(func3 == 0b010)
            csrrs(rd,rs1,imm);
        else if(func3 == 0b011)
            csrrc(rd,rs1,imm);
        else if(func3 == 0b101)
            csrrwi(rd,rs1,imm);
        else if(func3 == 0b110)
            csrrsi(rd,rs1,imm);
        else if(func3 == 0b111)
            csrrci(rd,rs1,imm);
        else undef();
    }
    else if (opcode == MISC_MEM)
    {
        uint8_t succ= (inst & 0b00000000111100000000000000000000) >> 20;
        uint8_t pred = (inst & 0b00001111000000000000000000000000) >> 24;
        uint8_t fm = (inst & 0b11110000000000000000000000000000) >> 28;

        if (func3 == 0 && rs1 == 0 && rd == 0 && imm == 0b100000110011)
            fence_tso();
        else if (func3 == 0 && rs1 == 0 && rd == 0 && imm == 0b000000010000)
            pause();
        else if (func3 == 0b000)
            fence(rd,rs1,succ,pred,fm);
        else undef();
    }
    else undef();

    if (x[0] != 0) printf("Error! Cannot write value to X0!");

    e_st.next_pc = exe_param->pc += 4;
    return e_st;
}
