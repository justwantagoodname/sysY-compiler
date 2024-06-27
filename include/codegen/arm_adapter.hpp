#pragma once

#ifndef ARM_ADAPTER_H
#define ARM_ADAPTER_H

#include "sysY.h"
#include "asm_helper.hpp"
#include "codegen/adapter.hpp"

class ARMAdapter : public Adapter {
private:
    AssemblyBuilder &asm_file;
public:
    explicit ARMAdapter(AssemblyBuilder &asm_file) : asm_file(asm_file) {}

    ~ARMAdapter() override = default;

    std::string platformName() const override {
        return "ARM32";
    }

    void postGenerate() override {
        asm_file.line()
                .line(".section	.note.GNU-stack,\"\",%%progbits")
                .line();
    }

    const int getWordSize() override {
        return 4;
    }

    const std::string getRegName(int reg) override {
        return "r" + std::to_string(reg);
    }

    const std::string getStackPointerName() override {
        return "sp";
    }

    const std::string getFramePointerName() override {
        return "fp";
    }

    const std::string getReturnAddressName() override {
        return "lr";
    }

    const std::string getPCName() override {
        return "pc";
    }

    const void emitComment() override {
        asm_file.line();
    }

    const void emitComment(const std::string& content) override {
        asm_file.line("# %s", content.c_str());
    }

    void emitFunctionLabel(const std::string &funcName) override {
        asm_file.line(".global %s", funcName.c_str());
        asm_file.line(".text");
        asm_file.line(".align 2");
        asm_file.line(".type %s, %%function", funcName.c_str());
        asm_file.line("%s:", funcName.c_str());
    }

    void emitLabel(const std::string &reg) override {
        asm_file.line("%s:", reg.c_str());
    }

    const std::string createRegList(const std::vector<std::string> &regs) {
        // TODO: replace to stringstream
        std::string reg_list = "{";
        bool first = true;
        for (const auto &reg : regs) {
            if (!first) {
                reg_list += ", ";
            }
            reg_list += reg;
            first = false;
        }
        reg_list += "}";
        return reg_list;
    }

    void pushStack(std::initializer_list<std::string> regs) override {
        asm_file.line("\tpush %s", createRegList(regs).c_str());
    }

    void pushStack(const std::vector<std::string> &regs) override {
        asm_file.line("\tpush %s", createRegList(regs).c_str());
    }

    void popStack(std::initializer_list<std::string> regs) override {
        asm_file.line("\tpop %s", createRegList(regs).c_str());
    }

    void popStack(const std::vector<std::string> &regs) override {
        asm_file.line("\tpop %s", createRegList(regs).c_str());
    }

    /**
     * 存储立即数到寄存器
     * @param reg
     * @param x
     */
    void loadImmediate(const std::string& reg, int x) override {
        // TODO：也许得考虑编译机器大小端
        auto ux = static_cast<unsigned int>(x);
        if ((ux & 0xFFFF0000) == 0) {
            asm_file.line("\tmov %s, #%u", reg.c_str(), ux);
        } else {
            unsigned int lo = ux & 0xFFFF, hi = ux >> 16;
            asm_file.line("\tmovw %s, #%d", reg.c_str(), lo);
            asm_file.line("\tmovt %s, #%d", reg.c_str(), hi);
        }
    }

    void loadImmediate(const std::string& reg, float x) override {
        // TODO: implement this
    }

    void loadLabelAddress(const std::string& reg, const std::string& labelName) override {
        asm_file.line("\tldr %s, =%s", reg.c_str(), labelName.c_str());
    }

    void loadRegister(const std::string& dst, const std::string& src, int offset) override {
        if (offset == 0) asm_file.line("\tldr %s, [%s]", dst.c_str(), src.c_str());
        else asm_file.line("\tldr %s, [%s, #%d]", dst.c_str(), src.c_str(), offset);
    }

    void storeRegister(const std::string& src, const std::string& dst, int offset) override {
        if (offset == 0) asm_file.line("\tstr %s, [%s]", src.c_str(), dst.c_str());
        else asm_file.line("\tstr %s, [%s, #%d]", src.c_str(), dst.c_str(), offset);
    }

    void uniOpWithImm(const std::string& op, const std::string& dst, const std::string& src, int imm) {
        asm_file.line("\t%s %s, %s, #%d", op.c_str(), dst.c_str(), src.c_str(), imm);
    }

    void uniOp(const std::string& op, const std::string& dst, const std::string& src, const std::string& src2) {
        asm_file.line("\t%s %s, %s, %s", op.c_str(), dst.c_str(), src.c_str(), src2.c_str());
    }

    void neg(const std::string& dst, const std::string& src) override {
        asm_file.line("\tnegs %s, %s", dst.c_str(), src.c_str());
    }

    void add(const std::string& dst, const std::string& src, int imm) override {
        uniOpWithImm("add", dst, src, imm);
    }

    void add(const std::string& dst, const std::string& src1, const std::string& src2) override {
        uniOp("add", dst, src1, src2);
    }

    void sub(const std::string& dst, const std::string& src, int imm) override {
        uniOpWithImm("sub", dst, src, imm);
    }

    void sub(const std::string& dst, const std::string& src1, const std::string& src2) override {
        uniOp("sub", dst, src1, src2);
    }

    void mul(const std::string& dst, const std::string& src1, const int imm) override {
        uniOpWithImm("mul", dst, src1, imm);
    }

    void mul(const std::string& dst, const std::string& src1, const std::string& src2) override {
        uniOp("mul", dst, src1, src2);
    }

    /*
        @note 会覆盖 r0 和 r1 的值需要先保存或者直接使用 r0 和 r1
    */
    void div(const std::string& dst, const std::string& src1, const std::string& src2) override {
        // TODO: 除2的幂次方可以用移位操作
        
        // 先移动 src1 到 r0

        if (src1 != "r0") asm_file.line("\tmov r0, %s", src1.c_str());
        // src2 移动到 r1
        if (src2 != "r1") asm_file.line("\tmov r1, %s", src2.c_str());

        // 调用函数
        asm_file.line("\tbl __aeabi_idiv");

        // 结果移动到 dst
        if (dst != "r0") {
            asm_file.line("\tmov %s, r0", dst.c_str());
        }
    }

    /*
        @note 会覆盖 r0 和 r1 的值需要先保存或者直接使用 r0 和 r1
    */
    void mod(const std::string& dst, const std::string& src1, const std::string& src2) override {
        // TODO: 对 2 的幂次方取模可以用位运算
        // 先移动 src1 到 r0
        if (src1 != "r0") asm_file.line("\tmov r0, %s", src1.c_str());
        // src2 移动到 r1
        if (src2 != "r1") asm_file.line("\tmov r1, %s", src2.c_str());
        // 调用函数
        asm_file.line("\tbl __aeabi_idivmod");
        // 结果移动到 dst
        if (dst != "r1") asm_file.line("\tmov %s, r1", dst.c_str());
    }

    void mov(const std::string& dst, const std::string& src) override {
        asm_file.line("\tmov %s, %s", dst.c_str(), src.c_str());
    }

    void call(const std::string& labelName) override {
        asm_file.line("\tbl %s", labelName.c_str());
    }

    void jump(const std::string& labelName) override {
        asm_file.line("\tb %s", labelName.c_str());
    }

    void nop() override {
        asm_file.line("\tnop");
    }

    void jumpEqual(const std::string& src1, const std::string& src2, const std::string& labelName) override {
        asm_file.line("\tcmp %s, %s", src1.c_str(), src2.c_str());
        asm_file.line("\tbeq %s", labelName.c_str());
    }

    void jumpEqual(const std::string& src1, int imm, const std::string& labelName) override {
        asm_file.line("\tcmp %s, #%d", src1.c_str(), imm);
        asm_file.line("\tbeq %s", labelName.c_str());
    }

    void jumpEqual(const std::string& src1, int imm, const std::string& eqLabel, const std::string& neLabel) override {
        asm_file.line("\tcmp %s, #%d", src1.c_str(), imm);
        asm_file.line("\tbeq %s", eqLabel.c_str());
        asm_file.line("\tbne %s", neLabel.c_str());
    }

    void jumpNotEqual(const std::string& src1, const std::string& src2, const std::string& labelName) override {
        asm_file.line("\tcmp %s, %s", src1.c_str(), src2.c_str());
        asm_file.line("\tbne %s", labelName.c_str());
    }

    void jumpNotEqual(const std::string& src1, int imm, const std::string& labelName) override {
        asm_file.line("\tcmp %s, #%d", src1.c_str(), imm);
        asm_file.line("\tbne %s", labelName.c_str());
    }

    void notReg(const std::string& dst, const std::string& src) override {
        /**
         * 这段代码确有妙处，乃从 GCC 中学来，以计算前导零之法巧取逻辑反转，可谓匠心独运。
         * 用 clz 计算前导零后，右移五位，逻辑反转之效立现。
         * 诚然，若以条件传送，需三指令之功，而此法则一石二鸟，殊为简洁。
         * 技法如此，真妙至极。
         */
        asm_file.line("\tclz %s, %s", src.c_str(), src.c_str());
        asm_file.line("\tlsrs %s, %s, #5", dst.c_str(), src.c_str());
    }

    void cmpGeneral(const std::string& src1, const std::string& src2) {
        asm_file.line("\tcmp %s, %s", src1.c_str(), src2.c_str());
    }

    void cmpEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmoveq %s, #1", dst.c_str());
        asm_file.line("\tmovne %s, #0", dst.c_str());
    }

    void cmpNotEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmovne %s, #1", dst.c_str());
        asm_file.line("\tmoveq %s, #0", dst.c_str());
    }

    void cmpLess(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmovlt %s, #1", dst.c_str());
        asm_file.line("\tmovge %s, #0", dst.c_str());
    }

    void cmpLessEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmovle %s, #1", dst.c_str());
        asm_file.line("\tmovgt %s, #0", dst.c_str());
    }

    void cmpGreater(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmovgt %s, #1", dst.c_str());
        asm_file.line("\tmovle %s, #0", dst.c_str());
    }

    void cmpGreaterEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        cmpGeneral(src1, src2);
        asm_file.line("\tmovge %s, #1", dst.c_str());
        asm_file.line("\tmovlt %s, #0", dst.c_str());
    }
};

#endif