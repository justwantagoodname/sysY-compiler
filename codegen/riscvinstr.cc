#include "codegen/riscvinstr.h"
#include <map>

RVOperand::RVOperand() : tag(UNDEF), value(0) {
    return;
}
RVOperand::RVOperand(RVOperandTag tag, int value)
    : tag(tag), value(value) {
        return;
}
bool RVOperand::isreg() const {
    return tag == REG || tag == SREG;
}
bool RVOperand::isimm() const {
    return tag == IMM || tag == SIMM;
}
bool RVOperand::isfloat() const {
    return tag == SREG || tag == SIMM;
}
std::string RVOperand::toASM() const {
    assert(false);
    return "NULL";
}


RVOperand make_reg(RVRegs reg) {
    RVOperand opr;
    opr.tag = REG;
    opr.reg = reg;
    return opr;
}
RVOperand make_sreg(RVRegs sreg) {
    RVOperand opr;
    opr.tag = SREG;
    opr.reg = sreg;
    return opr;
}
RVOperand make_imm(int value) {
    RVOperand opr;
    opr.tag = IMM;
    opr.value = value;
    return opr;
}
RVOperand make_simm(int value) {
    RVOperand opr;
    opr.tag = SIMM;
    opr.value = value;
    return opr;
}
RVOperand make_stack(RVRegs reg, uint16_t offset) {
    RVOperand opr;
    opr.tag = STACK;
    opr.reg = reg;
    opr.offset = offset;
    return opr;
}

RVInstr::RVInstr() : tag(NOP) {
    return;
}
RVInstr::RVInstr(RVOp opt) : opt(opt) {
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

        {SLT, "slt"},
        {SLTU, "sltu"},

        {LW, "lw"},
        {SW, "sw"}

    };
    std::string result = "    " + asm_op_tag.find(opt)->second;
    result += dst.toASM() + ", " + opr1.toASM();
    if (opr2.tag != UNDEF) result += ", " + opr2.toASM();
    return result;
}

RVArith::RVArith(RVOp opt, const RVOperand& dst, const RVOperand& opr1, const RVOperand& opr2)
    : RVInstr(opt), dst(dst), opr1(opr1), opr2(opr2) {
        assert(opt == RVOp::ADD || opt == RVOp::SUB || opt == RVOp::MUL
            || opt == RVOp::DIV || opt == RVOp::MOD);
        assert(dst.isreg());
        assert(opr1.isreg());
        assert(opr2.isreg() || opr2.isimm());

        if (opr1.isfloat() || opr2.isfloat()) {
            is_float = true;
        }
        return;
}
std::string RVArith::toASM() {
    if (is_float) {
        panic("TODO: RVArith::toASM()");
    }
    std::string result = "    ";
    switch (opt) {
        case RVOp::ADD:
            result += "add";
            break;
        case RVOp::SUB:
            result += "sub";
            break;
        case RVOp::MUL:
            result += "mul";
            break;
        case RVOp::DIV:
            result += "div";
            break;
        case RVOp::MOD:
            result += "rem";
            break;
        default:
            panic("Error: RVArith::toASM(): default");
    }
    if (opr2.isimm()) {
        result.push_back('i');
    }
    
    result += " " + dst.toASM() + " " + opr1.toASM() + " " + opr2.toASM();
    return result;
}

RVMem::RVMem(RVOp opt, const RVOperand& opr, uint16_t offset) 
    : RVInstr(opt), opr(opr), dst(make_stack(RVRegs::sp, offset)) {
    assert(opt == RVOp::LW || opt == RVOp::FLW || opt == RVOp::SW || opt == RVOp::FSW);
}
std::string RVMem::toASM() {
    panic("TODO!");
    return "";
}

std::string RVCall::intPutOnReg(const RVOperand& opr, uint8_t reg) {
    if (opr.isimm()) {
        return "    li " + std::to_string(reg) + ", " + opr.toASM() + "\n";
    } else if (opr.isreg()) {
        return "    mv " + std::to_string(reg) + ", " + opr.toASM() + "\n";
    }
}
std::string RVCall::toASM() {
    std::string result = "";
    std::string put_stack = "", put_regs = "";

    // deal with args
    uint8_t int_count = 0, float_count = 0;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].isfloat()) {
            panic("TODO: isfloat");
            if (float_count >= 8) {
                panic("TODO: put on stack");
            } else {
                panic("TODO: put in reg");
            }
            ++float_count;
        } else {
            if (int_count >= 8) {
                panic("TODO: put on stack");
            } else {
                put_regs += intPutOnReg(args[i], int_count);
            }
        }
    }

    panic("TODO: call");
    return result;
}

std::string RVPutf::toASM() {
    panic("TODO!");
    return "";
}