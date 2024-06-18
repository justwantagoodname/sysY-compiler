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

    const std::string getStackPointerName() override {
        return "sp";
    }

    const std::string getFramePointerName() override {
        return "fp";
    }

    const std::string getReturnAddressName() override {
        return "lr";
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

    void pushStack(std::initializer_list<std::string> regs) override {
        asm_file.raw("\tpush {");
        bool first = true;
        for (const auto &reg : regs) {
            if (!first) {
                asm_file.raw(", ");
            }
            asm_file.raw(reg.c_str());
            first = false;
        }
        asm_file.raw("}\n");
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

    void add(const std::string& dst, const std::string& src, int imm) override {
        assert(0 <= imm && imm <= 255);
        asm_file.line("\tadd %s, %s, #%d", dst.c_str(), src.c_str(), imm);
    }
};

#endif