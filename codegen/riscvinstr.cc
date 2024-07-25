#include "codegen/riscvinstr.h"
#include <map>

std::string RVOperand::getRegName() const {
    assert(isreg());
    if (reg == zero) return "zero";
    if (reg == ra) return "ra";
    if (reg == sp) return "sp";
    if (reg == s1) return "s1";
    if (10 <= reg && reg <= 17) return "a" + std::to_string(reg - 10);
    if (18 <= reg && reg <= 27) return "s" + std::to_string(reg - 16);
    if (0 <= reg && reg <= 27) return "x" + std::to_string(reg);

    panic("Error: getRegName()");
    return "";
}
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
    if (tag == IMM) return std::to_string(value);
    if (tag == SIMM) panic("SIMM");
    if (tag == REG) return getRegName();
    if (tag == SREG) panic("SREG");
    if (tag == ADDR) return addr;
    if (tag == STACK) return std::to_string(offset) + "(" + getRegName() + ")";
    return "ERROR!";
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
RVOperand make_addr(const std::string& label) {
    RVOperand opr;
    opr.tag = ADDR;
    opr.addr = label;
    return opr;
}

RVInstr::RVInstr() : opt(RVOp::NOP) {
    return;
}
RVInstr::RVInstr(RVOp opt) : opt(opt) {
    return;
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
    
    result += ", " + dst.toASM() + ", " + opr1.toASM() + ", " + opr2.toASM() + "\n";
    return result;
}

RVMem::RVMem(RVOp opt, const RVOperand& opr, uint16_t offset) 
    : RVInstr(opt), opr(opr), dst(make_stack(RVRegs::sp, offset)) {
    assert(opt == RVOp::LW || opt == RVOp::FLW 
        || opt == RVOp::SW || opt == RVOp::FSW);
}
RVMem::RVMem(RVOp opt, const RVOperand& opr, const RVOperand& value) 
    : RVInstr(opt), opr(opr), dst(value) {
    assert(opt == RVOp::LI);
    assert(dst.tag == IMM);
}
std::string RVMem::toASM() {
    switch (opt) {
        case RVOp::LI:
            return "    li " + opr.toASM() + ", " + dst.toASM() + "\n";
            break;
        case RVOp::LW:
        case RVOp::SW:
        case RVOp::FLW:
        case RVOp::FSW:
        default:
            panic("TODO!");
            break;
    }
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