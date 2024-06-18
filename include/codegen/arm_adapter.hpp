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

    const void emitSeparator() override {
        asm_file.line();
    }

    void emitFunctionLabel(const std::string &funcName) override {
        asm_file.line(".global %s", funcName.c_str());
        asm_file.line(".text");
        asm_file.line(".type %s, %%function", funcName.c_str());
        asm_file.line("%s:", funcName.c_str());
    }

    void emitLabel(const std::string &reg) override {
        asm_file.line("%s:", reg.c_str());
    }

    const std::string createRegList(std::initializer_list<std::string> regs) {
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

    void popStack(std::initializer_list<std::string> regs) override {
        asm_file.line("\tpop %s", createRegList(regs).c_str());
    }

    /**
     * 存储立即数，目前只能处理立即数小于等于 255 的情况
     * @param reg
     * @param x
     */
    void loadImmediate(const std::string& reg, int x) override {
        assert(0 <= x && x <= 255);
        asm_file.line("\tmov %s, #%d", reg.c_str(), x);
    }

    void loadImmediate(const std::string& reg, float x) override {
        // TODO: implement this
    }

    void uniOp(const std::string& op, const std::string& dst, const std::string& src, int imm) {
        assert(0 <= imm && imm <= 255);
        asm_file.line("\t%s %s, %s, #%d", op.c_str(), dst.c_str(), src.c_str(), imm);
    }

    void add(const std::string& dst, const std::string& src, int imm) override {
        uniOp("add", dst, src, imm);
    }

    void sub(const std::string& dst, const std::string& src, int imm) override {
        uniOp("sub", dst, src, imm);
    }

    void mov(const std::string& dst, const std::string& src) override {
        asm_file.line("\tmov %s, %s", dst.c_str(), src.c_str());
    }
};

#endif