// RV64GC - RV64IMAFDC

#pragma once

#ifndef RISCVINSTR_H_
#define RISCVINSTR_H_

#include "sysY.h"

enum RVOperandTag {
    UNDEF,
    ERROR,
    REG,
    SREG,
    IMM,
    SIMM,
    ADDR,

    // base on sp
    STACK,
    // base on s0
    BASE
};

enum RVRegs {
    zero = 0,
    ra = 1,
    sp = 2,
    s0 = 8,
    s1 = 9,
    a0 = 10, a1, a2, a3, a4, a5, a6, a7 = 17,
    s2 = 18, s3, s4, s5, s6, s7, s8, s9, s10, s11 = 27,
    fa0 = 28, fa1, fa2, fa3, fa4, fa5, fa6, fa7 = 35
};

class RVOperand {
    std::string getRegName() const;
public:
    RVOperandTag tag;
    int32_t value;
    // normal hi lo
    uint8_t nhl;
    std::string addr;
    RVRegs reg;
    int offset;
    RVOperand();
    RVOperand(RVOperandTag tag, int value);
    bool isreg() const;
    bool isimm() const;
    bool isfloat() const;
    std::string toASM() const;
};

RVOperand make_reg(RVRegs reg);
RVOperand make_areg(int offset);
RVOperand make_sreg(int sreg);
RVOperand make_sreg(RVRegs sreg);
RVOperand make_imm(int value);
RVOperand make_simm(int value);
RVOperand make_stack(RVRegs reg, int offset);
RVOperand make_addr(const std::string& label, uint8_t normal_hi_lo = 0);

enum class RVOp {
    // Arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    // Float arithmetic
    FADD,
    FSUB,
    FMUL,
    FDIV,
    FMOD,

    // Compare
    BGT,
    BGE,
    BLT,
    BLE,
    BEQ,
    BNE,
    BNEZ,

    // Float Compare
    FGT,
    FGE,
    FLT,
    FLE,
    FEQ,
    FNE,
    FNEZ,       

    MV,
    FMVXD,
    FMVS,
    // Float move
    // from integer
    FMVF,
    // to integer
    FMVT,

    // Float Convert
    // from integer
    FCVTF,
    // from integer unsigned
    FCVTFU,
    // to integer
    FCVTT,
    // to integer unsigned
    FCVTTU,
    // from 32bit to 64bit
    FCVTDS,
    FCVTWS,

    // Logical
    XOR,
    OR,
    AND,

    // Shifts
    SLL,
    SRL,
    SRA,

    // Compare
    // set <
    SLT,
    SLTU,

    // Load
    LW,
    LI,
    LD,
    FLW,
    FLD,
    // Load string
    LSTR,
    // Store
    SW,
    SD,
    FSW,

    // I
    // Arithmetic
    ADDI,

    // Logical
    XORI,
    ORI,
    ANDI,

    // Compare
    // set < immediate
    SLTI,
    SLTIU,

    // Jump
    J,
    JR,
    JALR,
    JMP,

    SEXTW,

    NOP
};

class RVInstr {
public:
    RVOp opt;
    RVInstr();
    RVInstr(RVOp opt);
    virtual std::string toASM() = 0;
};

class RVTag : public RVInstr {
public:
    std::string cont;
    RVTag(const std::string& cont);
    virtual std::string toASM() override;
};
class RVword : public RVInstr {
public:
    uint32_t value;
    RVword(uint32_t value);
    virtual std::string toASM() override;
};
class RVstring : public RVInstr {
public:
    std::string str;
    RVstring(const std::string& str);
    virtual std::string toASM() override;
};


class RVArith : public RVInstr {
public:
    bool is_float;
    RVOperand dst, opr1, opr2;
    RVArith(RVOp opt, const RVOperand& dst, const RVOperand& opr1, const RVOperand& opr2);
    virtual std::string toASM() override;
};

// Load & Store
class RVMem : public RVInstr {
public:
    bool is_float;
    RVOperand dst, opr;
    RVMem(RVOp opt, const RVOperand& opr, uint16_t dst_offset, bool base_on_sp = true);
    RVMem(RVOp opt, const RVOperand& dst, const RVOperand& value);
    virtual std::string toASM() override;
};

class RVMov: public RVInstr {
public:
    RVOperand dst, opr;
    RVMov(RVOp opt, const RVOperand& dst, const RVOperand& opr);
    virtual std::string toASM() override;
};

class RVConvert : public RVInstr {
public:
    RVOperand dst, opr;
    RVConvert(RVOp opt, const RVOperand& dst, const RVOperand& opr);
    virtual std::string toASM() override;
};

// Call
class RVCall : public RVInstr {
public:
    std::string func_name;
    RVCall(const std::string& func_name);
    virtual std::string toASM() override;
};

class RVJump : public RVInstr {
public:
    RVOperand dst;
    RVJump(RVOp opt, const RVOperand& dst);
    virtual std::string toASM() override;
};
class RVSext : public RVInstr {
public:
    RVOperand dst, opr;
    RVSext(RVOp opt, const RVOperand& dst, const RVOperand& opr);
    virtual std::string toASM() override;
};

class RVCompare : public RVInstr {
public:
    RVOperand dst;
    RVOperand op1, op2;
    RVCompare(RVOp opt, const RVOperand& dst, const RVOperand& op1, const RVOperand& op2);
    virtual std::string toASM() override;
};

#endif