
#include <stdint.h>
#include "back_end.h"
#include "execution.h"
#include "cpu_glb.h"

void decode(uint32_t inst,ExeStatus *e_st){

    uint8_t opcode = (inst & 0x7F);
    uint8_t func7 = (inst & 0b11111110000000000000000000000000) >> 25;
    uint8_t func3 = (inst & 0b00000000000000000111000000000000) >> 12;
    uint8_t rd    = (inst & 0b00000000000000000000111110000000) >> 7;
    uint8_t rs1   = (inst & 0b00000000000011111000000000000000) >> 15;
    uint8_t rs2   = (inst & 0b00000001111100000000000000000000) >> 20;
    int32_t imm   = ((int32_t)(inst & 0b11111111111100000000000000000000)) >> 20;

    if (opcode == OP_32){
            if (func7 == MULDIV)//
            {
                flags.is_alu = 1;
                if (func3 < 0b100)
                {
                    flags.is_mul = 1;
                } else {
                    flags.is_div = 1;
                }

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
                flags.is_alu = 1;
                if (func3 == 0b001 || func3 == 0b101)
                    flags.is_shift = 1;
                else if (func3 == 0b010 || func3 == 0b011)
                    flags.is_cmp = 1;

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
                flags.is_alu = 1;
                if (func3 == 0b101)
                    flags.is_shift = 1;

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

            flags.is_alu = 1;
            if (func3 == 0b001 || func3 == 0b101)
                flags.is_shift = 1;
            else if (func3 == 0b010 || func3 == 0b011)
                flags.is_cmp = 1;

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

        flags.is_branch=1;
        e_st->branch = 1;
        br_cnt +=1;

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

        flags.is_jump = 1;
        jmp_cnt +=1;
        e_st->branch = 1;
        jal(rd,jal_imm);
    }
    else if (opcode == JALR && func3 == 0b000)
    {
        flags.is_jump = 1;
        jmp_cnt +=1;
        e_st->branch = 1;
        jalr(rd,rs1,imm);
    }
    else if (opcode == LOAD)
    {
        flags.is_load = 1;
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

        flags.is_store = 1;

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

        flags.is_li = 1;
        lui(rd,li_imm);
    }
    else if (opcode == AUIPC)
    {
        int32_t li_imm = (inst & 0b11111111111111111111000000000000);
        flags.is_li = 1;
        auipc(rd,li_imm);
    }
    else if (opcode == SYSTEM)
    {
        flags.is_sys = 1;
        if (func3 >0b000)
            flags.is_csr = 1;

        if (func3 == 0b000)
        {
            if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b000000000000)
                ecall();
            else if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b000000000001)
                ebreak();
            else if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b001100000010)
                mret();
            else if (rs1 == 0b00000 && rd == 0b00000 && imm == 0b000100000101)
                wfi();
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
        else if (func3 == 1 && rs1 == 0 && rd == 0 && imm == 0)
            fence_i();
        else if (func3 == 0b000)
            fence(rd,rs1,succ,pred,fm);

        else undef();
    }
    else undef();

}