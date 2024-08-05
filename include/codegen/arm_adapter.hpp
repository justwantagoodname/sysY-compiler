#pragma once

#ifndef ARM_ADAPTER_H
#define ARM_ADAPTER_H

#include "sysY.h"
#include "asm_helper.hpp"
#include "codegen/adapter.hpp"

struct ExternFunctionDeclare {
    std::string name; // sysy 中的函数名
    std::string asm_name; // 汇编中实际转跳地址
    std::vector<std::pair<std::string, std::string> > args; // 参数列表 pair.first 是类型，第二个是参数名
    std::string ret_type; // 返回类型
    std::function<void(const ExternFunctionDeclare& declare, ASTNode* callContext, StackTranslator* translator)> call_handle; // 翻译调用函数

    void call(ASTNode* callContext, StackTranslator* translator) const {
        // 首先设置返回参数
        ASTNode_add_attr_str(callContext, "type", ret_type.c_str());

        call_handle(*this, callContext, translator);
    }
};

class ARMAdapter : public Adapter {
private:
    AssemblyBuilder &asm_file;
    std::map<std::string, ExternFunctionDeclare> extern_functions;

public:
    explicit ARMAdapter(AssemblyBuilder &asm_file) : asm_file(asm_file) {
        // "starttime","stoptime","getarray","getch","getfarray","getfloat","getint","putarray","putch","putf","putfarray","putfloat","putint"
        auto arm_std_call_hf_handle = [this](const ExternFunctionDeclare& declare, ASTNode* callContext, StackTranslator* translator) {
            // 这个其实只能用于我们的StackTranslator，因为这里先把参数 push 到栈上

            // 开始计算参数
            auto real_param_size = (int) ASTNode_children_size(callContext); // 在调用上下文中获取实参数个数
            assert(real_param_size == declare.args.size());

            std::vector<std::string> arg_type;
            if (real_param_size) {
                // 有实参需要先计算
                QueryResult *real_params = ASTNode_querySelector(callContext, "Param"),
                            *cur = real_params->prev;
                int idx = real_param_size - 1;
                do {
                    // 反向遍历参数
                    const char *type;
                    ASTNode *inner = ASTNode_querySelectorOne(cur->node, "*");
                    translator->translateExpInner(inner);

                    bool hasType = ASTNode_get_attr_str(inner, "type", &type);
                    assert(hasType);

                    arg_type.push_back(type);

                    if (idx != 0) pushStack({translator->accumulatorReg}); // 倒着入栈，所以第一个参数不需要 push
                    cur = cur->prev;
                    idx--;
                } while (cur != real_params->prev);
            }
            std::reverse(arg_type.begin(), arg_type.end());
            int integer_reg = 0, float_reg = 0;

            // 注意这里没有判断什么时候停止（参数全部放到寄存器上），考虑库函数没有多余4个的情况，所以没有问题
            for (int i = 0;i < arg_type.size(); i++) {
                if (arg_type[i] == declare.args[i].first) {
                    // 如果类型相同，直接放到寄存器上
                    if (arg_type[i] == SyFloat) {
                        if (i != 0) {
                            floadRegister(getFRegisterName(float_reg++), getStackPointerName(), 0);
                            add(getStackPointerName(), getStackPointerName(), 4);
                        } else {
                            // fmov(getFRegisterName(float_reg++), getRegName(0));
                            // 注意所有的浮点数返回被放在 s0
                            float_reg++;
                        }
                    } else {
                        // 其他所有情况都直接放到整数寄存器上
                        if (i != 0) popStack({getRegName(integer_reg++)});
                        else integer_reg++; // 第一个参数不需要 pop
                    }
                } else {
                    // 如果类型不同，需要转换
                    if (arg_type[i] == SyInt && declare.args[i].first == SyFloat) {
                        if (i != 0) {
                            floadRegister(getFRegisterName(float_reg), getStackPointerName(), 0);
                            add(getStackPointerName(), getStackPointerName(), 4);
                        } else {
                            fmov(getFRegisterName(float_reg), getRegName(0));
                        }
                        i2f(getFRegisterName(float_reg), getFRegisterName(float_reg));
                        float_reg++;
                    } else if (arg_type[i] == SyFloat && declare.args[i].first == SyInt) {
                        if (i != 0) {
                            floadRegister(getFRegisterName(float_reg), getStackPointerName(), 0);
                            add(getStackPointerName(), getStackPointerName(), 4);
                        } else {
                            fmov(getFRegisterName(float_reg), getRegName(0));
                        }
                        f2i(getFRegisterName(float_reg), getFRegisterName(float_reg));
                        fmov(getRegName(integer_reg), getFRegisterName(float_reg));
                        integer_reg++;
                    } else {
                        assert(false);
                    }
                }
            }

            call(declare.asm_name);
        };

        // 需要行号的函数
        auto lib_require_line = [this](const ExternFunctionDeclare& declare, ASTNode* callContext, StackTranslator* translator) {
            int lineno;
            ASTNode_get_attr_int(callContext, "line", &lineno);


            loadImmediate(getRegName(0), lineno);
            call(declare.asm_name);
        };

        auto arm_gnu_varargs_call_handle = [this, &asm_file](const ExternFunctionDeclare& declare, ASTNode* callContext, StackTranslator* translator) {
            // 这个其实只能用于我们的StackTranslator，因为这里先把参数 push 到栈上

            // 翻译 GNU 可变参数函数调用
            // 主要是对 float 参数特殊处理
            // 这个其实只能用于我们的StackTranslator，因为这里先把参数 push 到栈上

            // 开始计算参数
            auto real_param_size = (int) ASTNode_children_size(callContext); // 在调用上下文中获取实参数个数
            assert(real_param_size >= 1); // 可变参数至少大于1

            int param_stack_size = 0; // 参数在栈上的大小

            asm_file.mark(); // 插入对齐的位置

            std::vector<std::string> arg_type;
            if (real_param_size) {
                // 有实参需要先计算
                QueryResult *real_params = ASTNode_querySelector(callContext, "Param"),
                        *cur = real_params->prev;
                int idx = real_param_size - 1;
                do {
                    const char *type;
                    bool hasType = ASTNode_get_attr_str(cur->node, "type", &type);
                    if (hasType && strcmp(type, "StringConst") == 0){
                        arg_type.push_back("StringConst");
                        const char* label;
                        bool hasLabel = ASTNode_get_attr_str(cur->node, "label", &label);
                        assert(hasLabel);
                        loadLabelAddress(getRegName(0), label); // string 肯定是第一个直接加载到 r0
                    } else {
                        // 反向遍历参数
                        ASTNode *inner = ASTNode_querySelectorOne(cur->node, "*");
                        translator->translateExpInner(inner);
                        translator->passType(cur->node, inner);

                        hasType = ASTNode_get_attr_str(inner, "type", &type);

                        if (hasType && strcmp(type, SyFloat) == 0) {
                            arg_type.push_back(SyFloat);
                            // 如果是 float 参数，需要转成 double
                            f2d("d0", "s0");
                            fpushStack({"d0"});
                            param_stack_size += 8;
                        } else {
                            assert(hasType && strcmp(type, SyInt) == 0); // 只有 Int 和 Float 两种类型
                            arg_type.push_back(SyInt);
                            if (idx != 0) pushStack({translator->accumulatorReg}); // 倒着入栈，所以第一个参数不需要 push
                            param_stack_size += getWordSize();
                        }
                    }

                    cur = cur->prev;
                    idx--;
                } while (cur != real_params->prev);
            }
            int reg_param_size = std::min(4, real_param_size); // 这个 4 是 ARM32 函数调用时放在寄存器上的函数参数个数
            std::reverse(arg_type.begin(), arg_type.end());
            if (reg_param_size > 1) { // 一个参数不需要 pop 因为最后一个参数没有 push
                std::vector<std::string> regs;
                for (int i = 1; i < reg_param_size; i++) {
                    if (arg_type[i] == SyInt) {
                        param_stack_size -= getWordSize();
                        regs.push_back(getRegName(i));
                    }

                    if (arg_type[i] == SyFloat && i != 3) {
                        param_stack_size -= 8;
                        // 以double储存为两个字节
                        // 这里非常奇怪，浮点参数被固定放到 r2 r3 中
                        regs.push_back(getRegName(2));
                        regs.push_back(getRegName(3));
                        break; // 碰到浮点参数就结束，后面到一律压入栈
                    }
                }
                popStack(regs);
            }
            if (param_stack_size % 8 != 0) {
                param_stack_size += 4;
                // 插入到之前的位置
                asm_file.beginBack();
                sub(getStackPointerName(), getStackPointerName(), 4);
                asm_file.endBack();
            }
            call(declare.asm_name);
            if (param_stack_size > 0) 
                add(getStackPointerName(), getStackPointerName(), param_stack_size); // 还原栈
        };
        // 常规函数
        extern_functions["getch"] = ExternFunctionDeclare {
            .name = "getch",
            .asm_name = "getch",
            .args = {},
            .ret_type = "Int",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["getint"] = ExternFunctionDeclare {
            .name = "getint",
            .asm_name = "getint",
            .args = {},
            .ret_type = "Int",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["getarray"] = ExternFunctionDeclare {
            .name = "getarray",
            .asm_name = "getarray",
            .args = {{"[Int", "dest_base"}},
            .ret_type = "Int",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["getfarray"] = ExternFunctionDeclare {
            .name = "getfarray",
            .asm_name = "getfarray",
            .args = {{"[Float", "dest_base"}},
            .ret_type = "Int",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["putint"] = ExternFunctionDeclare {
            .name = "putint",
            .asm_name = "putint",
            .args = {{"Int", "x"}},
            .ret_type = "Void",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["putch"] = ExternFunctionDeclare {
            .name = "putch",
            .asm_name = "putch",
            .args = {{"Int", "x"}},
            .ret_type = "Void",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["putarray"] = ExternFunctionDeclare {
            .name = "putarray",
            .asm_name = "putarray",
            .args = {{"Int", "size"}, {"[Int", "src_base"}},
            .ret_type = "Void",
            .call_handle = arm_std_call_hf_handle
        };
        extern_functions["putfarray"] = ExternFunctionDeclare {
            .name = "putfarray",
            .asm_name = "putfarray",
            .args = {{"Int", "size"}, {"[Float", "src_base"}},
            .ret_type = "Void",
            .call_handle = arm_std_call_hf_handle
        };

        extern_functions["putfloat"] = ExternFunctionDeclare {
            .name = "putfloat",
            .asm_name = "putfloat",
            .args = {{"Float", "x"}},
            .ret_type = "Void",
            .call_handle = arm_std_call_hf_handle
        };

        extern_functions["getfloat"] = ExternFunctionDeclare {
            .name = "getfloat",
            .asm_name = "getfloat",
            .args = {},
            .ret_type = "Float",
            .call_handle = arm_std_call_hf_handle
        };

        // 计时函数
        extern_functions["starttime"] = ExternFunctionDeclare {
            .name = "starttime",
            .asm_name = "_sysy_starttime",
            .args = {},
            .ret_type = "Void",
            .call_handle = lib_require_line
        };
        extern_functions["stoptime"] = ExternFunctionDeclare {
            .name = "stoptime",
            .asm_name = "_sysy_stoptime",
            .args = {},
            .ret_type = "Void",
            .call_handle = lib_require_line
        };

        // 可变参数函数
        extern_functions["putf"] = ExternFunctionDeclare {
            .name = "putf",
            .asm_name = "putf",
            .args = {},
            .ret_type = "Void",
            .call_handle = arm_gnu_varargs_call_handle
        };
    }

    ~ARMAdapter() override = default;

    std::string platformName() const override {
        return "ARM32";
    }

    void preGenerate() override {
        asm_file.line("\t.syntax unified")
            .line("\t.arch armv7-a")
            .line("\t.fpu vfpv4")
            .line("\t.eabi_attribute 27, 3")
            .line("\t.eabi_attribute 28, 1")
            .line("\t.eabi_attribute 23, 1")
            .line("\t.eabi_attribute 24, 1")
            .line("\t.eabi_attribute 25, 1")
            .line("\t.eabi_attribute 26, 2")
            .line("\t.eabi_attribute 30, 6")
            .line("\t.eabi_attribute 34, 0")
            .line("\t.eabi_attribute 18, 4")
            .line();
    }

    void postGenerate() override {
        asm_file.line()
                .line(".section	.note.GNU-stack,\"\",%%progbits")
                .line(".ident	\"SysY-Compiler\"")
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

    bool isExternalFunction(const std::string& funcName) override {
        return extern_functions.find(funcName) != extern_functions.end();
    }

    void emitExternFunction(const std::string& funcName, ASTNode* callContext, StackTranslator* translator) override {
        assert(callContext != nullptr);
        assert(ASTNode_id_is(callContext, "Call"));
        assert(extern_functions.find(funcName) != extern_functions.end());

        auto& declare = extern_functions[funcName];
        declare.call(callContext, translator);
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

    bool is_float_immediate(double x) {
        if (x <= 0) {
            return false;
        }

        int exponent;
        double mantissa = frexp(x, &exponent);

        // Adjust mantissa and exponent to match the desired form
        double n = mantissa * 128; // since frexp gives mantissa in range [0.5, 1), we shift by 7 to get in range [64, 128)
        exponent -= 7; // compensate the shift done above

        // Now check if n and adjusted exponent fit the required range
        return (n >= 16 && n <= 31 && exponent >= 0 && exponent <= 7);
    }

    void loadImmediate(const std::string& reg, float x) override {
        unsigned int ux = reinterpret_cast<unsigned int &>(x);
        if (is_float_immediate(x)) {
            asm_file.line("\tvmov.f32 %s, #%u", reg.c_str(), ux);
        } else {
            loadImmediate("r3", (int) ux);
            fmov("s0", "r3");
        }
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
        if (abs(imm) > 255) {
            loadImmediate(getRegName(5), imm);
            uniOp(op, dst, src, getRegName(5));
        } else {
            asm_file.line("\t%s %s, %s, #%d", op.c_str(), dst.c_str(), src.c_str(), imm);
        }
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

    void floatCmpGeneral(const std::string& src1, const std::string& src2) {
        asm_file.line("\tvcmp.f32 %s, %s", src1.c_str(), src2.c_str());
        asm_file.line("\tvmrs APSR_nzcv, FPSCR");
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

    std::string getFRegisterName(int reg) override {
        return "s" + std::to_string(reg);
    }

    void fmov(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvmov %s, %s", dst.c_str(), src.c_str());
    }

    void floadRegister(const std::string& dst, const std::string& src, int offset) override {
        if (offset == 0) asm_file.line("\tvldr.32 %s, [%s]", dst.c_str(), src.c_str());
        else asm_file.line("\tvldr.32 %s, [%s, #%d]", dst.c_str(), src.c_str(), offset);
    }

    void fstoreRegister(const std::string& src, const std::string& dst, int offset) override {
        if (offset == 0) asm_file.line("\tvstr.32 %s, [%s]", src.c_str(), dst.c_str());
        else asm_file.line("\tvstr.32 %s, [%s, #%d]", src.c_str(), dst.c_str(), offset);
    }

    void fpushStack(std::initializer_list<std::string> regs) override {
        asm_file.line("\tvpush %s", createRegList(regs).c_str());
    }

    void fpushStack(const std::vector<std::string> &regs) override {
        asm_file.line("\tvpush %s", createRegList(regs).c_str());
    }

    void fpopStack(std::initializer_list<std::string> regs) override {
        asm_file.line("\tvpop %s", createRegList(regs).c_str());
    }

    void fpopStack(const std::vector<std::string> &regs) override {
        asm_file.line("\tvpop %s", createRegList(regs).c_str());
    }

    void i2f(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvcvt.f32.s32 %s, %s", dst.c_str(), src.c_str());
    }

    void f2i(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvcvt.s32.f32 %s, %s", dst.c_str(), src.c_str());
    }

    void f2d(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvcvt.f64.f32 %s, %s", dst.c_str(), src.c_str());
    }

    void d2f(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvcvt.f32.f64 %s, %s", dst.c_str(), src.c_str());
    }

    void fneg(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvneg.f32 %s, %s", dst.c_str(), src.c_str());
    }

    void fnotReg(const std::string& dst, const std::string& src) override {
        asm_file.line("\tvcmp.f32 s15, #0", dst.c_str(), src.c_str())
                .line("\tvmrs APSR_nzcv, FPSCR")
                .line("\tmoveq %s, #1", dst.c_str())
                .line("\tmovne r0, #0", dst.c_str());
    }

    void fadd(const std::string& dst, const std::string& src1, const std::string& src2) override {
        asm_file.line("\tvadd.f32 %s, %s, %s", dst.c_str(), src1.c_str(), src2.c_str());
    }

    void fsub(const std::string& dst, const std::string& src1, const std::string& src2) override {
        asm_file.line("\tvsub.f32 %s, %s, %s", dst.c_str(), src1.c_str(), src2.c_str());
    }

    void fmul(const std::string& dst, const std::string& src1, const std::string& src2) override {
        asm_file.line("\tvmul.f32 %s, %s, %s", dst.c_str(), src1.c_str(), src2.c_str());
    }

    void fdiv(const std::string& dst, const std::string& src1, const std::string& src2) override {
        asm_file.line("\tvdiv.f32 %s, %s, %s", dst.c_str(), src1.c_str(), src2.c_str());
    }

    void fmod(const std::string& dst, const std::string& src1, const std::string& src2) override {
        assert(false);
        // TODO: 没有相应的硬件指令
    }

    void fcmpEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmoveq %s, #1", dst.c_str());
        asm_file.line("\tmovne %s, #0", dst.c_str());
    }

    void fcmpNotEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmovne %s, #1", dst.c_str());
        asm_file.line("\tmoveq %s, #0", dst.c_str());
    }

    void fcmpLess(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmovlt %s, #1", dst.c_str());
        asm_file.line("\tmovge %s, #0", dst.c_str());
    }

    void fcmpLessEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmovle %s, #1", dst.c_str());
        asm_file.line("\tmovgt %s, #0", dst.c_str());
    }

    void fcmpGreater(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmovgt %s, #1", dst.c_str());
        asm_file.line("\tmovle %s, #0", dst.c_str());
    }

    void fcmpGreaterEqual(const std::string& dst, const std::string& src1, const std::string& src2) override {
        floatCmpGeneral(src1, src2);
        asm_file.line("\tmovge %s, #1", dst.c_str());
        asm_file.line("\tmovlt %s, #0", dst.c_str());
    }

    void fjumpEqual(const std::string& src1, const float imm, const std::string& labelName) override {
        asm_file.line("\tvcmp.f32 %s, #%f", src1.c_str(), imm);
        asm_file.line("\tbeq %s", labelName.c_str());
    }

};

#endif