#ifndef __EXECUTION_H__
    #define __EXECUTION_H__

#include <stdint.h>

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
    ;
}
static inline void bne(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void blt(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void bgt(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void bltu(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void bgeu(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
// unconditional branch
static inline void jal(uint8_t rd, int32_t imm){
    ;
}
static inline void jalr(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
// load
static inline void lb(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
static inline void lh(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
static inline void lw(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
static inline void lbu(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
static inline void lhu(uint8_t rd, uint8_t rs1, int32_t imm){
    ;
}
//store
static inline void sb(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void sh(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
}
static inline void sw(uint8_t rs1, uint8_t rs2, int32_t imm){
    ;
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
            x[rd] = (uint32_t)((int32_t)(pc) - abs_imm);
}
// system
static inline void ecall(){
    ;
}
static inline void ebreak(){
    ;
}
static inline void csrrw(uint8_t rd, uint8_t rs1, uint32_t csr){
    ;
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
    ;
}
static inline void pause(){
    ;
}
static inline void fence(uint8_t rd, uint8_t rs1, uint8_t succ,uint8_t pred,uint8_t fm){
    ;
}
// Undefined
static inline void undef(){
    ;
}
#endif //__EXECUTION_H__
