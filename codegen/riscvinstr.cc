#include "codegen/riscvinstr.h"
#include <map>

RVOperand::RVOperand() : tag(UNDEF), value(0) {
    return;
}
RVOperand::RVOperand(RVOperandTag tag, int value)
    : tag(tag), value(value) {
        return;
}
std::string RVOperand::toASM() {
    assert(false);
    return "NULL";
}


RVOperand make_reg(int reg) {
    return RVOperand(REG, reg);
}
RVOperand make_sreg(int sreg) {
    return RVOperand(SREG, sreg);
}
RVOperand make_imm(int value) {
    return RVOperand(IMM, value);
}
RVOperand make_simm(float value) {
    return RVOperand(SIMM, value);
}

RVInstr::RVInstr() : tag(NOP) {
    return;
}
RVInstr::RVInstr(RVInstrTag op) : tag(op) {
    return;
}
RVBinaryInstr::RVBinaryInstr(RVBinaryOp opt, RVOperand opr1, RVOperand opr2, RVOperand des)
    : RVInstr(RInstr), opt(opt), opr1(opr1), opr2(opr2), des(des) {
    return;
}
std::string RVBinaryInstr::toASM() {
    static const std::map<RVBinaryOp, std::string> asm_op_tag = {
        {ADD, "add"},
        {SUB, "sub"},
        {MUL, "mul"},
        {DIV, "div"},
        {MOD, "rem"},
        {FADD, "fadd.s"},
        {FSUB, "fsub.s"},
        {FMUL, "fmul.s"},
        {FMOD, "frem.s"},
        {XOR, "xor"},
        {OR, "or"},
        {AND, "and"},
        {SLL, "sll"},
        {SRL, "srl"},
        {SRA, "sra"}

    };
    std::string result = "    " + asm_op_tag.find(opt)->second;
    result += des.toASM() + ", " + opr1.toASM() + ", " + opr2.toASM();
    return result;
}