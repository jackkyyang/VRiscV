#ifndef __EXECUTION_H__
    #define __EXECUTION_H__

#include <stdint.h>
#include <assert.h>
#include "../dev/memory.h"
#include "sys_reg.h"
#include "../include/comm.h"
#include "cpu_glb.h"

static inline MXLEN_T addr_calc(MXLEN_T source, int32_t imm){
        // 相当于AGU
        MXLEN_ST abs_imm = (MXLEN_ST)(-imm);
        if (imm > 0)
            return source + (MXLEN_T)imm;
        else
            return source - (MXLEN_T)(abs_imm);
}

// reg op
static inline void mul(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)(r1 * r2);
}

static inline void mulh(uint8_t rd, uint8_t rs1, uint8_t rs2){
    DMXLEN_ST r1 = (DMXLEN_ST)(x[rs1]);
    DMXLEN_ST r2 = (DMXLEN_ST)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)((r1 * r2) & 0xffffffff00000000 >> 32);
}

static inline void mulhsu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    DMXLEN_ST r1 = (DMXLEN_ST)(x[rs1]);
    DMXLEN_T  r2 = (DMXLEN_T)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)((r1 * r2) & 0xffffffff00000000 >> 32);
}

static inline void mulhu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    DMXLEN_T r1 = (DMXLEN_T)(x[rs1]);
    DMXLEN_T r2 = (DMXLEN_T)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)((r1 * r2) & 0xffffffff00000000 >> 32);
}

static inline void div(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r2 == 0)
        x[rd] = (MXLEN_T)(0xffffffffffffffff);
    else if (r1 == 0x80000000 && r2 == -1)
        x[rd] = (MXLEN_T)(0x80000000);
    else if (rd != 0)
        x[rd] = (MXLEN_T)(r1/r2);
}

static inline void divu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);

    if (r2 == 0)
        x[rd] = (MXLEN_T)0xffffffffffffffff;
    else if (rd != 0)
        x[rd] = (MXLEN_T)(r1/r2);
}

static inline void rem(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r2 == 0)
        x[rd] = (MXLEN_T)(0xffffffffffffffff);
    else if (r1 == 0x80000000 && r2 == -1)
        x[rd] = 0;
    else if (rd != 0)
        x[rd] = (MXLEN_T)(r1%r2);
}

static inline void remu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);

    if (r2 == 0)
        x[rd] = (MXLEN_T)(0xffffffffffffffff);
    else if (rd != 0)
        x[rd] = (MXLEN_T)(r1%r2);
}

static inline void add(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)(r1 + r2);
}

static inline void sll(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // logical left shift
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = (MXLEN_T)(r1 << sh);
}

static inline void slt(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);
    if (rd != 0)
        if (r1 < r2) x[rd] = 1;
        else x[rd] = 0;
}

static inline void sltu(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    if (rd != 0)
        if (r1 < r2) x[rd] = 1;
        else x[rd] = 0;
}

static inline void xor(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    if (rd != 0)
        x[rd] = (MXLEN_T)(r1 ^ r2);
}

static inline void srl(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // logical right shift
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = r1 >> sh;
}

static inline void or(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 | r2;
}

static inline void and(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    if (rd != 0)
        x[rd] = r1 & r2;
}

static inline void sub(uint8_t rd, uint8_t rs1, uint8_t rs2){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);
    if (rd != 0)
        x[rd] = (uint64_t)(r1 + r2);
}

static inline void sra(uint8_t rd, uint8_t rs1, uint8_t rs2){
    // arithmetic right shift
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    uint8_t sh = (x[rs2] & 0b11111);
    if (rd != 0)
        x[rd] = r1 >> sh;
}
// imme
static inline void addi(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 + imm;
}
static inline void slti(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        if (r1 < imm)
            x[rd] = 1;
        else x[rd] = 0;
}
static inline void sltiu(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    if (rd != 0)
        if (r1 < (MXLEN_T)(imm))
            x[rd] = 1;
        else x[rd] = 0;
}
static inline void xori(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 ^ imm;
}
static inline void ori(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 | imm;
}
static inline void andi(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 & imm;
}
static inline void slli(uint8_t rd, uint8_t rs1, uint8_t shamt){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 << shamt;
}
static inline void srli(uint8_t rd, uint8_t rs1, uint8_t shamt){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 >> shamt;
}
static inline void srai(uint8_t rd, uint8_t rs1, uint8_t shamt){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    if (rd != 0)
        x[rd] = r1 >> shamt;
}
static inline void uop(){
    ;
}
// conditional branch
static inline void beq(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r1 == r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_T)(pc + 4);
    }

}
static inline void bne(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r1 != r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_ST)(pc + 4);
    }
}
static inline void blt(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r1 < r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_T)(pc + 4);
    }
}
static inline void bgt(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_ST r1 = (MXLEN_ST)(x[rs1]);
    MXLEN_ST r2 = (MXLEN_ST)(x[rs2]);

    if (r1 >= r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_T)(pc + 4);
    }
}
static inline void bltu(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    if (r1 < r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_T)(pc + 4);
    }
}
static inline void bgeu(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);

    if (r1 >= r2)
    {
        br_taken_cnt +=1;
        next_pc = addr_calc(pc,imm);
    } else {
        next_pc = (MXLEN_T)(pc + 4);
    }
}
// unconditional branch
static inline void jal(uint8_t rd, int32_t imm){

    MXLEN_ST abs_imm = - imm;
    next_pc = addr_calc(pc,imm);

    if (rd != 0)
        x[rd] = (MXLEN_T)(pc + 4);

}
static inline void jalr(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    next_pc = addr_calc(r1,imm);

    if (rd != 0)
        x[rd] = (MXLEN_T)(pc + 4);
}
// load
static inline void lb(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    uint8_t rd_data;
    MXLEN_T addr = addr_calc(r1,imm);
    int read_num = read_data(addr,1,CPU_BE,&rd_data);
    assert(read_num == 1); // load指令必须有数据返回
    if (rd != 0)
        x[rd] = (MXLEN_ST)(rd_data);

}
static inline void lh(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    uint16_t rd_data;
    MXLEN_T addr = addr_calc(r1,imm);
    int read_num = read_data(addr,2,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 2); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = (MXLEN_ST)(rd_data);
    }
}
static inline void lw(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    int32_t rd_data;
    MXLEN_T addr = addr_calc(r1,imm);
    int read_num = read_data(addr,4,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 4); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = rd_data;
    }
}
static inline void lbu(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    uint8_t rd_data;
    MXLEN_T addr = addr_calc(r1,imm);
    int read_num = read_data(addr,1,CPU_BE,&rd_data);
    assert(read_num == 1); // load指令必须有数据返回
    if (rd != 0)
        x[rd] = (MXLEN_T)(rd_data);
}
static inline void lhu(uint8_t rd, uint8_t rs1, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    uint16_t rd_data;
    MXLEN_T addr = addr_calc(r1,imm);
    int read_num = read_data(addr,2,CPU_BE,(uint8_t*)(&rd_data));
    assert(read_num == 2); // load指令必须有数据返回
    if (rd != 0) {
        x[rd] = (MXLEN_T)(rd_data);
    }
}
//store
static inline void sb(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    uint8_t wr_data = (uint8_t)r2;
    MXLEN_T addr = addr_calc(r1,imm);
    int write_num = write_data(addr,1,CPU_BE,&wr_data);
    assert(write_num == 1); // write指令必须有数据返回
}
static inline void sh(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    uint8_t wr_data[2] = {(uint8_t)r2,(uint8_t)(r2 >> 8)};
    MXLEN_T addr = addr_calc(r1,imm);
    int write_num = write_data(addr,2,CPU_BE,wr_data);
    assert(write_num == 2); // write指令必须有数据返回
}
static inline void sw(uint8_t rs1, uint8_t rs2, int32_t imm){
    MXLEN_T r1 = (MXLEN_T)(x[rs1]);
    MXLEN_T r2 = (MXLEN_T)(x[rs2]);
    MXLEN_T addr = addr_calc(r1,imm);
    int write_num = write_data(addr,4,CPU_BE,(uint8_t*)(&r2));
    assert(write_num == 4); // write指令必须有数据返回
}
// load imme
static inline void lui(uint8_t rd, int32_t imm){
    if (rd != 0)
        x[rd] = (MXLEN_T)imm;
}
static inline void auipc(uint8_t rd, int32_t imm){
    if (rd != 0)
        x[rd] = addr_calc(pc,imm);
}
// system
static inline void ecall(){
    ecall_trap(); // 执行trap操作，在其中设置好了next_pc
}
static inline void ebreak(){
    uop();
}
static inline void mret(){
    ExeStatus *e_st = read_exe_st();
    CPUMode curr_mode = get_cpu_mode();
    if (curr_mode != M)
    {
        //如果在非M模式下执行，直接报异常
        raise_illegal_instruction(curr_mode,e_st->inst);
        return;
    }
    // 执行MRET指令
    mret_proc();
}
static inline void wfi(){
    uop();
}

static inline int csr_check(CSRFeild csr_id,int write){
    ExeStatus *e_st = read_exe_st();
    CPUMode curr_mode = get_cpu_mode();
    int err_flag = 0;
    // 检查是否访问了高权限寄存器
    if (curr_mode == U && csr_id.pri > 0)
    {
        err_flag = 1;
    }
    #ifdef S_MODE
    else if (curr_mode == S && csr_id.pri > 1)
    {
        err_flag = 1;
    }
    #endif

    // 检查是否对只读寄存器进行写操作
    if (write == 1 && csr_id.acc == 3)
    {
        err_flag = 1;
    }

    if (err_flag == 1)
    {
        raise_illegal_instruction(curr_mode,e_st->inst);
    }

    return err_flag;
}

static inline void csrrw(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_data;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);
    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,1)) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    // rw一定会写入
    write_data = x[rs1];
    csr_write(csr,write_data);
}
static inline void csrrs(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_msk,write_data;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);

    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,(rs1!=0))) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    if (rs1 != 0) // 如果rs==0，不写CSR
    {
        write_msk = x[rs1];
        write_data = read_data | write_msk; // 将mask中为1的bit置1
        csr_write(csr,write_data);
    }

}
static inline void csrrc(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_msk,write_data;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);

    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,(rs1!=0))) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    if (rs1 != 0) // 如果rs==0，不写CSR
    {
        write_msk = x[rs1];
        write_data = read_data & (~write_msk); // 将mask中为1的bit清零
        csr_write(csr,write_data);
    }
}
static inline void csrrwi(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_data;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);
    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,1)) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    // rw一定会写入
    write_data = (MXLEN_T)rs1;
    csr_write(csr,write_data);
}
static inline void csrrsi(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_data,write_msk;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);
    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,(rs1!=0))) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    if (rs1 != 0) // 如果rs==0，不写CSR
    {
        write_msk = (MXLEN_T)rs1;
        write_data = read_data | write_msk; // 将mask中为1的bit置1
        csr_write(csr,write_data);
    }
}
static inline void csrrci(uint8_t rd, uint8_t rs1, uint32_t csr){
    MXLEN_T read_data,write_data,write_msk;
    CSRFeild csr_id = INT2STRUCT(CSRFeild,csr);
    // 检查指令是否合法，包括
    // 1. 检查读写权限
    // 2. 检查是否访问了高级别的寄存器
    if (csr_check(csr_id,(rs1!=0))) // 非法指令，跳过执行阶段
        return;

    csr_read(csr,&read_data);
    if (rd != 0) // 如果rd==0，不写寄存器
    {
        x[rd] = read_data;
    }
    if (rs1 != 0) // 如果rs==0，不写CSR
    {
        write_msk = (MXLEN_T)rs1;
        write_data = read_data & (~write_msk); // 将mask中为1的bit清零
        csr_write(csr,write_data);
    }
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
    ExeStatus *e_st = read_exe_st();
    raise_illegal_instruction(get_cpu_mode(),e_st->inst);
}
#endif //__EXECUTION_H__
