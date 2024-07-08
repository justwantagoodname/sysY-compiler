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
    std::string toASM();
};

RVOperand make_reg(int reg);
RVOperand make_sreg(int sreg);
RVOperand make_imm(int value);
RVOperand make_simm(float value);

enum RVInstrTag {
    NOP,
    RInstr,
};

class RVInstr {
public:
    RVInstrTag tag;
    RVInstr();
    RVInstr(RVInstrTag tag);
    virtual std::string toASM() = 0;
};

enum RVBinaryOp {
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

    // Logical
    XOR,
    OR,
    AND,

    // Shifts
    SLL,
    SRL,
    SRA

    
};
class RVBinaryInstr : public RVInstr {
public:
    RVBinaryOp opt;
    RVOperand opr1, opr2, des;
    RVBinaryInstr(RVBinaryOp opt, RVOperand opr1, RVOperand opr2, RVOperand des);
    virtual std::string toASM() override;
};

#endif