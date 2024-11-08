#include "codegen/riscvinstr.h"
#include <map>

std::string RVOperand::getRegName() const {
    assert(isreg() || tag == STACK);
    if (reg == RVRegs::zero) return "zero";
    if (reg == RVRegs::ra) return "ra";
    if (reg == RVRegs::sp) return "sp";
    if (reg == RVRegs::s0) return "s0";
    if (reg == RVRegs::s1) return "s1";
    if (RVRegs(10) <= reg && reg <= RVRegs(17)) return "a" + std::to_string(int(reg) - 10);
    if (RVRegs(18) <= reg && reg <= RVRegs(27)) return "s" + std::to_string(int(reg) - 16);
    if (RVRegs(0) <= reg && reg <= RVRegs(27)) return "x" + std::to_string(int(reg));
    if (RVRegs(28) <= reg && reg <= RVRegs(35)) return "fa" + std::to_string(int(reg) - 28);

    printf("reg id: %d\n", reg);
    panic("Error: getRegName(): bad reg id");
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
    if (tag == SIMM)return ".LC" + std::to_string(value);
    if (tag == REG) return getRegName();
    if (tag == SREG) return getRegName();
    if (tag == ADDR) {
        if (nhl == 0) return addr;
        if (nhl == 1) return "%hi(" + addr + ")";
        if (nhl == 2) return "%lo(" + addr + ")";
    }
    if (tag == STACK) return std::to_string(offset) + "(" + getRegName() + ")";
    return "ERROR!";
}


RVOperand make_reg(RVRegs reg) {
    RVOperand opr;
    opr.tag = REG;
    opr.reg = reg;
    return opr;
}
RVOperand make_areg(int offset) {
    assert(offset < 8 && offset >= 0);
    RVRegs reg = RVRegs(10 + offset);
    return make_reg(reg);
}
RVOperand make_sreg(int offset) {
    assert(offset < 8);
    RVRegs reg = RVRegs(28 + offset);
    RVOperand res = make_reg(reg);
    res.tag = SREG;
    return res;
}
RVOperand make_sreg(RVRegs reg) {
    assert(int(reg) - 28 < 8 && int(reg) - 28 > 0);
    RVOperand res = make_reg(reg);
    res.tag = SREG;
    return res;
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
RVOperand make_stack(RVRegs reg, int offset) {
    RVOperand opr;
    opr.tag = STACK;
    opr.reg = reg;
    opr.offset = offset;
    return opr;
}
RVOperand make_addr(const std::string& label, uint8_t normal_hi_lo) {
    RVOperand opr;
    opr.tag = ADDR;
    opr.addr = label;
    opr.nhl = normal_hi_lo;
    return opr;
}

RVInstr::RVInstr() : opt(RVOp::NOP) {
    return;
}
RVInstr::RVInstr(RVOp opt) : opt(opt) {
    return;
}

RVTag::RVTag(const std::string& cont) : RVInstr(RVOp::NOP), cont(cont) {
    return;
}
std::string RVTag::toASM() {
    return cont + ":\n";
}
RVword::RVword(uint32_t value) : value(value) {
    return;
}
std::string RVword::toASM() {
    return "    .word " + std::to_string(value) + "\n";
}
RVstring::RVstring(const std::string& str) : str(str) {
    return;
}
std::string RVstring::toASM() {
    return "    .string \"" + str + "\"\n";
}
RVzero::RVzero(uint32_t size) :size(size) {
    return;
}

std::string RVzero::toASM() {
    return "    .zero " + std::to_string(size * 4) + "\n";
}


RVArith::RVArith(RVOp opt, const RVOperand& dst, const RVOperand& opr1, const RVOperand& opr2)
    : RVInstr(opt), dst(dst), opr1(opr1), opr2(opr2) {
    //assert(opt == RVOp::ADD || opt == RVOp::SUB || opt == RVOp::MUL
    //	|| opt == RVOp::DIV || opt == RVOp::MOD);
    assert(dst.isreg());
    assert(opr1.isreg());
    assert(opr2.isreg() || opr2.isimm() || opr2.tag == ADDR);

    if (opr1.isfloat() || opr2.isfloat()) {
        is_float = true;
    } else {
        is_float = false;
    }
    return;
}
std::string RVArith::toASM() {
    std::string result = "    ";
    switch (opt) {
    case RVOp::ADD:
        result += "add";
        break;
    case RVOp::ADDI:
        result += "addi";
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
    case RVOp::FADD:
        result += "fadd.s";
        break;
    case RVOp::FSUB:
        result += "fsub.s";
        break;
    case RVOp::FMUL:
        result += "fmul.s";
        break;
    case RVOp::FDIV:
        result += "fdiv.s";
        break;
    case RVOp::FMOD:
        result += "frem.s";
        break;
    default:
        panic("Error: RVArith::toASM(): default");
    }

    if (opr2.tag == ADDR)
        result += " " + dst.toASM() + ", " + opr1.toASM() + ", " + "%lo(" + opr2.toASM() + ")" + "\n";
    else
        result += " " + dst.toASM() + ", " + opr1.toASM() + ", " + opr2.toASM() + "\n";
    return result;
}

RVMem::RVMem(RVOp opt, const RVOperand& opr, uint16_t offset, bool base_on_sp)
    : RVInstr(opt), opr(opr) {
    assert(opt == RVOp::LD || opt == RVOp::LW || opt == RVOp::SW || opt == RVOp::SD);
    if (base_on_sp) dst = make_stack(RVRegs::sp, offset);
    else dst = make_stack(RVRegs::s0, offset);
}
RVMem::RVMem(RVOp opt, const RVOperand& dst, const RVOperand& addr)
    : RVInstr(opt), dst(dst), opr(addr) {
    //assert(opt == RVOp::FLW || opt == RVOp::FLD || opt == RVOp::FSW || opt == RVOp::LSTR || opt == RVOp::LI || opt == RVOp::LD || opt == RVOp::LW);
}
// RVMem::RVMem(RVOp opt, const RVOperand& dst, const RVOperand& value) 
//     : RVInstr(opt), dst(dst), opr(value) {
//     assert(opt == RVOp::LI);
//     assert(dst.tag == IMM);
// }
std::string RVMem::toASM() {
    string result;
    switch (opt) {
    case RVOp::LI:
        result = "    li " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::LW:
        result = "    lw " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::LD:
        result = "    ld " + opr.toASM() + ", " + dst.toASM() + "\n";
        break;
    case RVOp::LUI:
        result = "    lui " + dst.toASM() + ", " + "%hi(" + opr.toASM() + ")" + "\n";
        break;
    case RVOp::SW:
        result = "    sw " + opr.toASM() + ", " + dst.toASM() + "\n";
        break;
    case RVOp::SD:
        result = "    sd " + opr.toASM() + ", " + dst.toASM() + "\n";
        break;
    case RVOp::FLW:
        if (opr.tag == ADDR) {
            result = "    lui " + dst.toASM().substr(1) + ", %" + "hi(" + opr.toASM() + ")\n";
            result += "    flw " + dst.toASM() + ", %" + "lo(" + opr.toASM() + ")(" + dst.toASM().substr(1) + ")\n";
        } else {
            result += "    flw " + dst.toASM() + ", " + opr.toASM() + "\n";
        }
        break;
    case RVOp::FLD:
        if (opr.tag == ADDR) {
            result = "    lui " + dst.toASM().substr(1) + ", %" + "hi(" + opr.toASM() + ")\n";
            result += "    fld " + dst.toASM() + ", %" + "lo(" + opr.toASM() + ")(" + dst.toASM().substr(1) + ")\n";
        } else {
            result += "    fld " + dst.toASM() + ", " + opr.toASM() + "\n";
        }
        break;
    case RVOp::FSW:
        result = "    fsw " + opr.toASM() + ", " + dst.toASM() + "\n";
        break;
    case RVOp::LSTR:
        result = "    lui " + make_areg(5).toASM() + ", %hi(" + opr.toASM() + ")\n";
        result += "    addi " + dst.toASM() + ", " + make_areg(5).toASM() + ", %lo(" + opr.toASM() + ")\n";
        break;
    default:
        panic("TODO!");
        break;
    }
    return result;
}

RVConvert::RVConvert(RVOp opt, const RVOperand& dst, const RVOperand& opr)
    : RVInstr(opt), dst(dst), opr(opr) {
    //assert(opt == RVOp::FCVTDS || opt == RVOp::FCVTF);
}
std::string RVConvert::toASM() {
    string result;
    switch (opt) {
    case RVOp::FCVTDS:
        result = "    fcvt.d.s " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::FCVTF:
        result = "    fcvt.s.w " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::FCVTT:
        result = "    fcvt.w.s " + dst.toASM() + ", " + opr.toASM() + ", rtz\n";
        break;
    default:
        panic("Error on RVConvert");
        break;
    }
    return result;
}

RVMov::RVMov(RVOp opt, const RVOperand& dst, const RVOperand& opr)
    : RVInstr(opt), dst(dst), opr(opr) {
    assert(opt == RVOp::MV || opt == RVOp::FMVXD || opt == RVOp::FMVS);
}
std::string RVMov::toASM() {
    string result;
    switch (opt) {
    case RVOp::FMVXD:
        result = "    fmv.x.d " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::FMVS:
        result = "    fmv.s " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    case RVOp::MV:
        result = "    mv " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;
    default:
        panic("Error on RVMov");
        break;
    }
    return result;
}

RVCall::RVCall(const std::string& func_name)
    : func_name(func_name) {
    return;
}
std::string RVCall::toASM() {
    return "    call " + func_name + "\n";
}

RVJump::RVJump(RVOp opt, const RVOperand& dst)
    : RVInstr(opt), dst(dst) {
    assert(dst.tag == REG || dst.tag == ADDR);
    return;
}
std::string RVJump::toASM() {
    std::string result;
    switch (opt) {
    case RVOp::J:
        result = "    j " + dst.toASM() + "\n";
        break;
    case RVOp::JR:
        result = "    jr " + dst.toASM() + "\n";
        break;
    case RVOp::JMP:
        result = "    j " + dst.toASM() + "\n";
        break;
    case RVOp::JALR:
    default:
        panic("RVJump error");
        break;
    }
    return result;
}

RVSext::RVSext(RVOp opt, const RVOperand& dst, const RVOperand& opr)
    : RVInstr(opt), dst(dst), opr(opr) {
    return;
}
std::string RVSext::toASM() {
    std::string result = "";
    switch (opt)
    {
    case RVOp::SEXTW:
        result = "    sext.w " + dst.toASM() + ", " + opr.toASM() + "\n";
        break;

    default:
        panic("RVSext error");
        break;
    }
}
RVCompare::RVCompare(RVOp opt, const RVOperand& dst, const RVOperand& op1, const RVOperand& op2)
    : RVInstr(opt), dst(dst), op1(op1), op2(op2)
{
}

std::string RVCompare::toASM() {
    std::string result;
    switch (opt) {
    case RVOp::BGT:
        result = "    bgt " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BGE:
        result = "    bge " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BLT:
        result = "    blt " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BLE:
        result = "    ble " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BEQ:
        result = "    beq " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BNE:
        result = "    bne " + op1.toASM() + "," + op2.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::BNEZ:
        result = "    bne " "zero" "," + op1.toASM() + "," + dst.toASM() + "\n";
        break;
    case RVOp::FGT:
        result = "    fgt.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FGE:
        result = "    fge.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FLT:
        result = "    flt.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FLE:
        result = "    fle.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FEQ:
        result = "    feq.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FNE:
        result = "    fne.s " "a5" "," + op1.toASM() + "," + op2.toASM() + "\n";
        result += "    bne "  "a5"  "," "zero" "," + dst.toASM() + "\n";
        break;
    case RVOp::FNEZ:
    default:
        panic("RVCompare error");
        break;
    }
    return result;
}

RVSLLi::RVSLLi(RVOp opt, const RVOperand& dst, int imm)
    :dst(dst), imm(imm), RVInstr(opt)
{
}

std::string RVSLLi::toASM()
{
    std::string result;
    switch (opt) {
    case RVOp::SLL:
        result = "    slli " + dst.toASM() + "," + dst.toASM() + "," + std::to_string(imm) + "\n";
        break;
    default:
        panic("RVSLLI error");
        break;
    }
    return result;
}

