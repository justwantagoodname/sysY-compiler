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
    ADDR
};


class RVOperand {
public:
    RVOperandTag tag;
    int32_t value;
    RVOperand();
    RVOperand(RVOperandTag tag, int value);
    bool isreg() const;
    bool isimm() const;
    std::string toASM() const;
};

RVOperand make_reg(int reg);
RVOperand make_sreg(int sreg);
RVOperand make_imm(int value);
RVOperand make_simm(float value);

enum RVInstrTag {
    NOP,
    RInstr,
    IInstr
};

class RVInstr {
public:
    RVInstrTag tag;
    RVInstr();
    RVInstr(RVInstrTag tag);
    virtual std::string toASM() = 0;
};

enum RVROp {
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
    // Store
    SW
    
};
class RVRInstr : public RVInstr {
public:
    RVROp opt;
    RVOperand opr1, opr2, dst;
    RVRInstr(RVROp opt, const RVOperand& opr1, const RVOperand& opr2, const RVOperand& dst);
    virtual std::string toASM() override;
};


enum RVIOp {
    // Arithmetic
    ADDI,

    // Logical
    XORI,
    ORI,
    ANDI,

    // Compare
    // set < immediate
    SLTI,
    SLTIU
};
class RVIInstr : public RVInstr {
public:
    RVIOp opt;
    RVOperand dst, opr;
    RVIInstr(RVIOp opt, const RVOperand& opr, const RVOperand& dst);
    virtual std::string toASM() override;
};

#endif