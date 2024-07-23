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

enum class RVRegs {
    ra = 1,
    sp = 2
};

class RVOperand {
public:
    RVOperandTag tag;
    int32_t value;
    int32_t addr;
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

enum RVInstrTag {
    NOP,
    RInstr,
    IInstr
};

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
    SLTIU
};

class RVInstr {
public:
    RVInstrTag tag;
    RVOp opt;
    RVInstr();
    RVInstr(RVOp opt);
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