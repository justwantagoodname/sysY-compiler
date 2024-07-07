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

enum RVOp {
    NOP,
    BINARY,
};

class RVInstr {
public:
    RVOp op;
    RVInstr();
    RVInstr(RVOp op);
    virtual std::string toASM() = 0;
};

enum RVBinary {
    ADD,
    SUB
};
class RVBinaryInstr : public RVInstr {
public:
    RVBinary opt;
    RVOperand opr1, opr2, result;
    RVBinaryInstr(RVOperand opr1, RVOperand opr2, RVOperand result);
    virtual std::string toASM() override;
};

#endif