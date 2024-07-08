#include "codegen/riscvinstr.h"
#include <map>

RVOperand::RVOperand() : tag(UNDEF), value(0) {
    return;
}
RVOperand::RVOperand(RVOperandTag tag, int value)
    : tag(tag), value(value) {
        return;
}
std::string RVOperand::toASM() const {
    assert(false);
    return "NULL";
}
bool RVOperand::isreg() const {
    return tag == REG || tag == SREG;
}
bool RVOperand::isimm() const {
    return tag == IMM || tag == SIMM;
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
RVRInstr::RVRInstr(RVROp opt, RVOperand opr1, RVOperand opr2, RVOperand dst)
    : RVInstr(RInstr), opt(opt), opr1(opr1), opr2(opr2), dst(dst) {
    return;
}
std::string RVRInstr::toASM() {
    static const std::map<RVROp, std::string> asm_op_tag = {
        {ADD, "add"},
        {SUB, "sub"},
        {MUL, "mul"},
        {DIV, "div"},
        {MOD, "rem"},

        {FADD, "fadd.s"},
        {FSUB, "fsub.s"},
        {FMUL, "fmul.s"},
        {FMOD, "frem.s"},

        {FMVF, "fmv.w.x"},
        {FMVT, "fmv.x.w"},
        {FCVTF, "fcvt.s.w"},
        {FCVTFU, "fcvt.s.wu"},
        {FCVTT, "fcvt.w.s"},
        {FCVTTU, "fcvt.wu.s"},

        {XOR, "xor"},
        {OR, "or"},
        {AND, "and"},
        {SLL, "sll"},
        {SRL, "srl"},
        {SRA, "sra"},

        {LW, "lw"},
        {SW, "sw"}

    };
    std::string result = "    " + asm_op_tag.find(opt)->second;
    result += dst.toASM() + ", " + opr1.toASM();
    if (opr2.tag != UNDEF) result += ", " + opr2.toASM();
    return result;
}