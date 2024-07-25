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
    STACK
};

enum RVRegs {
    zero = 0,
    ra = 1,
    sp = 2,
    s1 = 9,
    a0 = 10, a1, a2, a3, a4, a5, a6, a7 = 17,
    s2 = 18, s3, s4, s5, s6, s7, s8, s9, s10, s11 = 27
};

class RVOperand {
    std::string getRegName() const;
public:
    RVOperandTag tag;
    int32_t value;
    std::string addr;
    RVRegs reg;
    uint16_t offset;
    RVOperand();
    RVOperand(RVOperandTag tag, int value);
    bool isreg() const;
    bool isimm() const;
    bool isfloat() const;
    std::string toASM() const;
};

RVOperand make_reg(int reg);
RVOperand make_sreg(int sreg);
RVOperand make_imm(int value);
RVOperand make_simm(float value);
RVOperand make_stack(RVRegs reg, uint16_t offset);
RVOperand make_addr(const std::string& label);

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
    FLW,
    // Store
    SW,
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

    NOP
};

class RVInstr {
public:
    RVOp opt;
    RVInstr();
    RVInstr(RVOp opt);
    virtual std::string toASM() = 0;
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
    RVMem(RVOp opt, const RVOperand& opr, uint16_t offset);
    RVMem(RVOp opt, const RVOperand& opr, const RVOperand& value);
    virtual std::string toASM() override;
};

// Call
class RVCall : public RVInstr {
    std::string intPutOnReg(const RVOperand& opr, uint8_t reg);
public:
    std::vector<RVOperand> args;
    virtual std::string toASM() override;
};

// Putf
class RVPutf : public RVInstr {
public:
    std::vector<RVOperand> args;
    virtual std::string toASM() override;
};

#endif