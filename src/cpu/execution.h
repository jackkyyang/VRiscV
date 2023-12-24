#ifndef __EXECUTION_H__
    #define __EXECUTION_H__

#include <stdint.h>
#include <assert.h>
#include "../dev/memory.h"
#include "sys_reg.h"
#include "../include/comm.h"

// reg op
static inline void mul(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = (r1 * r2);
}

static inline void mulh(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int64_t r1 = (int64_t)(x[rs1]);
    int64_t r2 = (int64_t)(x[rs2]);
    if (rd != 0)
        x[rd] = ((r1 * r2) & 0xffffffff00000000) >> 32;
}

static inline void mulhsu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int64_t r1 = (int64_t)(x[rs1]);
    uint64_t r2 = (uint64_t)(x[rs2]);
    if (rd != 0)
        x[rd] = ((r1 * r2) & 0xffffffff00000000) >> 32;
}

static inline void mulhu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint64_t r1 = (uint64_t)(x[rs1]);
    uint64_t r2 = (uint64_t)(x[rs2]);
    if (rd != 0)
        x[rd] = ((r1 * r2) & 0xffffffff00000000) >> 32;
}

static inline void div(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r2 == 0)
        x[rd] = 0xffffffffffffffff;
    else if (r1 == 0x80000000 && r2 == -1)
        x[rd] = 0x80000000;
    else if (rd != 0)
        x[rd] = r1/r2;
}

static inline void divu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);

    if (r2 == 0)
        x[rd] = 0xffffffffffffffff;
    else if (rd != 0)
        x[rd] = r1/r2;
}

static inline void rem(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r2 == 0)
        x[rd] = 0xffffffffffffffff;
    else if (r1 == 0x80000000 && r2 == -1)
        x[rd] = 0;
    else if (rd != 0)
        x[rd] = r1%r2;
}

static inline void remu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);

    if (r2 == 0)
        x[rd] = 0xffffffffffffffff;
    else if (rd != 0)
        x[rd] = r1%r2;
}

static inline void add(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 + r2;
}

static inline void sll(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // logical left shift
    int32_t r1 = (int32_t)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = r1 << sh;
}

static inline void slt(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);
    if (rd != 0)
        if (r1 < r2) x[rd] = 1;
        else x[rd] = 0;
}

static inline void sltu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    if (rd != 0)
        if (r1 < r2) x[rd] = 1;
        else x[rd] = 0;
}

static inline void xor(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 ^ r2;
}

static inline void srl(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // logical right shift
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = r1 >> sh;
}

static inline void or(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 | r2;
}

static inline void and(uint8_t rd, uint8_t rs1, uint8_t rs2){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 & r2;
}

static inline void sub(uint8_t rd, uint8_t rs1, uint8_t rs2){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);
    if (rd != 0)
        x[rd] = (uint64_t)(r1 + r2);
}

static inline void sra(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // arithmetic right shift
    int32_t r1 = (int32_t)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = r1 >> sh;
}
// imme
static inline void addi(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 + imm;
}
static inline void slti(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        if (r1 < imm)
            x[rd] = 1;
        else x[rd] = 0;
}
static inline void sltiu(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    if (rd != 0)
        if (r1 < (uint32_t)(imm))
            x[rd] = 1;
        else x[rd] = 0;
}
static inline void xori(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 ^ imm;
}
static inline void ori(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 | imm;
}
static inline void andi(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 & imm;
}
static inline void slli(uint8_t rd, uint8_t rs1, uint8_t shamt){
    uint32_t r1 = (uint32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 << shamt;
}
static inline void srli(uint8_t rd, uint8_t rs1, uint8_t shamt){
    uint32_t r1 = (uint32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 >> shamt;
}
static inline void srai(uint8_t rd, uint8_t rs1, uint8_t shamt){
    int32_t r1 = (int32_t)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 >> shamt;
}
static inline void uop(){
    ;
}
// conditional branch
static inline void beq(uint8_t rs1, uint8_t rs2, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r1 == r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }

}
static inline void bne(uint8_t rs1, uint8_t rs2, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r1 != r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }
}
static inline void blt(uint8_t rs1, uint8_t rs2, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r1 < r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }
}
static inline void bgt(uint8_t rs1, uint8_t rs2, int32_t imm){
    int32_t r1 = (int32_t)(x[rs1]);
    int32_t r2 = (int32_t)(x[rs2]);

    if (r1 >= r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }
}
static inline void bltu(uint8_t rs1, uint8_t rs2, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    if (r1 < r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }
}
static inline void bgeu(uint8_t rs1, uint8_t rs2, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);

    if (r1 >= r2)
    {
        int32_t abs_imm = - imm;
        br_taken_cnt +=1;
        if (imm > 0)
            next_pc = (uint32_t)(pc + (uint64_t)(imm));
        else
            next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));
    } else {
        next_pc = (uint32_t)(pc + 4);
    }
}
// unconditional branch
static inline void jal(uint8_t rd, int32_t imm){

    int32_t abs_imm = - imm;
    if (imm > 0)
        next_pc = (uint32_t)(pc + (uint64_t)(imm));
    else
        next_pc = (uint32_t)(pc - (uint64_t)(abs_imm));

    if (rd != 0)
        x[rd] = (uint32_t)(pc + 4);

}
static inline void jalr(uint8_t rd, uint8_t rs1, int32_t imm){
    int32_t abs_imm = - imm;
    uint32_t r1 = (uint32_t)(x[rs1]);
    if (imm > 0)
        next_pc = ((uint32_t)(r1 + (uint32_t)(imm))) & 0xfffffffe;
    else
        next_pc = ((uint32_t)(r1 - (uint32_t)(abs_imm))) & 0xfffffffe;

    if (rd != 0)
        x[rd] = (uint32_t)(pc + 4);
}
// load
static inline void lb(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    int32_t abs_imm = (-imm);
    uint8_t rd_data;
    uint64_t addr;
    if (imm>0)
    {
        addr =  (uint64_t)(r1 + imm);
    } else {
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);
    }

    int read_num = read_data(addr,1,CPU_BE,&rd_data);
    assert(read_num == 1); // load指令必须有数据返回
    if (rd != 0)
        x[rd] = (int32_t)(rd_data);

}
static inline void lh(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    int32_t abs_imm = (-imm);
    uint16_t rd_data;
    uint64_t addr;
    if (imm>0)
        addr =  (uint64_t)(r1 + imm);
    else
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);

    int read_num = read_data(addr,2,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 2); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = (int32_t)(rd_data);
    }
}
static inline void lw(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    int32_t abs_imm = (-imm);
    int32_t rd_data;
    uint64_t addr;
    if (imm>0)
        addr =  (uint64_t)(r1 + imm);
    else
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);

    int read_num = read_data(addr,4,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 4); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = rd_data;
    }
}
static inline void lbu(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    int32_t abs_imm = (-imm);
    uint8_t rd_data;
    uint64_t addr;
    if (imm>0)
    {
        addr =  (uint64_t)(r1 + imm);
    } else {
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);
    }

    int read_num = read_data(addr,1,CPU_BE,&rd_data);
    assert(read_num == 1); // load指令必须有数据返回
    if (rd != 0)
        x[rd] = (uint32_t)(rd_data);
}
static inline void lhu(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    int32_t abs_imm = (-imm);
    uint16_t rd_data;
    uint64_t addr;
    if (imm>0)
        addr =  (uint64_t)(r1 + imm);
    else
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);

    int read_num = read_data(addr,2,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 2); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = (uint32_t)(rd_data);
    }
}
//store
static inline void sb(uint8_t rs1, uint8_t rs2, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    uint8_t wr_data = (uint8_t)r2;
    int32_t abs_imm = (-imm);
    uint64_t addr;
    if (imm>0)
    {
        addr =  (uint64_t)(r1 + imm);
    } else {
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);
    }

    int write_num = write_data(addr,1,CPU_BE,&wr_data);
    assert(write_num == 1); // write指令必须有数据返回
}
static inline void sh(uint8_t rs1, uint8_t rs2, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    uint8_t wr_data[2] = {(uint8_t)r2,(uint8_t)(r2 >> 8)};
    int32_t abs_imm = (-imm);
    uint64_t addr;
    if (imm>0)
    {
        addr =  (uint64_t)(r1 + imm);
    } else {
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);
    }

    int write_num = write_data(addr,2,CPU_BE,wr_data);
    assert(write_num == 1); // write指令必须有数据返回
}
static inline void sw(uint8_t rs1, uint8_t rs2, int32_t imm){
    uint32_t r1 = (uint32_t)(x[rs1]);
    uint32_t r2 = (uint32_t)(x[rs2]);
    int32_t abs_imm = (-imm);
    uint64_t addr;
    if (imm>0)
    {
        addr =  (uint64_t)(r1 + imm);
    } else {
        addr =  (uint64_t)(r1 - (uint32_t)abs_imm);
    }

    int write_num = write_data(addr,4,CPU_BE,(uint8_t*)(&r2));
    assert(write_num == 1); // write指令必须有数据返回
}
// load imme
static inline void lui(uint8_t rd, int32_t imm){
    if (rd != 0)
        x[rd] = (uint32_t)imm;
}
static inline void auipc(uint8_t rd, int32_t imm){
    int32_t abs_imm = - imm;
    if (rd != 0)
        if (imm > 0)
            x[rd] = (uint32_t)(pc + (uint64_t)(imm));
        else
            x[rd] = (uint32_t)(pc - (uint64_t)(abs_imm));
}
// system
static inline void ecall(){
    exception_info.exception_vaild = 1;
}
static inline void ebreak(){
    uop();
}
static inline void mret(){
    exception_info.exception_vaild = 1;
}
static inline void wfi(){
    uop();
}
static inline void csrrw(uint8_t rd, uint8_t rs1, uint32_t csr){
    uint32_t read_data;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);
    if (rd == 0) // 只读取不写入
    {
        csr_read(csr,&read_data);
    }

}
static inline void csrrs(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
}
static inline void csrrc(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
}
static inline void csrrwi(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
}
static inline void csrrsi(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
}
static inline void csrrci(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
}
// misc mem
static inline void fence_tso(){
    uop();
}
static inline void pause(){
    uop();
}
static inline void fence(uint8_t rd, uint8_t rs1, uint8_t succ,uint8_t pred,uint8_t fm){
    uop();
}
// Undefined
static inline void undef(){
    exception_info.exception_vaild = 1;

}
#endif //__EXECUTION_H__
