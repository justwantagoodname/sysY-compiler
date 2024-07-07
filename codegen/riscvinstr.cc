#include "codegen/riscvinstr.h"

RVOperand::RVOperand() : tag(UNDEF), value(0) {
    return;
}
RVOperand::RVOperand(RVOperandTag tag, int value)
    : tag(tag), value(value) {
        return;
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

RVInstr::RVInstr() : op(NOP) {
    return;
}
RVInstr::RVInstr(RVOp op) : op(op) {
    return;
}
RVBinaryInstr::RVBinaryInstr(RVOperand opr1, RVOperand opr2, RVOperand result)
    : RVInstr(BINARY), opr1(opr1), opr2(opr2), result(result) {
    return;
}
std::string RVBinaryInstr::toASM() override {
    std::string result;
    switch (opt) {
        case ADD:
            result += "    add ";
            break;
        case SUB:
            result += "    sub ";
            break;
        default:
            assert(false);
            break;
    }
    result += result.toASM() + ", " + opr1.toASM() + ", " + opr2.toASM();
    return result;
}