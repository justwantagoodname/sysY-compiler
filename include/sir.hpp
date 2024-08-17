#pragma once

#ifndef SIR_H
#define SIR_H

#include "sysY.h"

namespace SIR {
    enum InstructionType {
        NOP = 0x00,
        ICONST, // push a int const
        FCONST, // push a float const

        ILDLOCAL, // load local
        FLDLOCAL,

        ALDLOCAL, // load address to stack

        IALD, // load address with offset
        FALD,

        IAST, // store to address with offset
        FAST,

        ISTLOCAL, // store local
        FSTLOCAL,

        ILDGOBAL, // load global
        FLDGOBAL,
        ALDGOBAL, // load a global address

        ISTGOBAL, // store global
        FSTGOBAL, // store global


        IRET,
        FRET,

        CALL,

        IADD,
        ISUB,
        IMUL,
        IDIV,
        IREM,
        INEG,
        INOT,

        FADD,
        FSUB,
        FMUL,
        FDIV,
        FNEG,
        FNOT, // the fnot will change the type of top of stack

        ICMPEQ,
        ICMPNE,
        ICMPLT,
        ICMPGT,
        ICMPLE,
        ICMPGE,

        FCMPEQ,
        FCMPNE,
        FCMPLT,
        FCMPGT,
        FCMPLE,
        FCMPGE,

        I2F,
        F2I,

        DUP, // duplicate the top

    };

    class Instruction {
    public:
        InstructionType type;

        std::variant<int, float> operand1;
        
        Instruction(const InstructionType type = NOP) {
            this->type = type;
        }

        string to_readable() const {
            switch (type) {
                case NOP: return "nop";
                case ICONST: {
                    return "iconst " + to_string(get<int>(operand1));
                }
                case FCONST: {
                    return "fconst " + to_string(get<float>(operand1));
                }
            
                default:
                    return "ERR";
            }
        }
    };

    typedef shared_ptr<Instruction> InstructionRef;

    class Function {
    public:
        vector<shared_ptr<Instruction>> body;
        string name, returnType;
        vector<string> paramtersType;

        Function(NodeRef func) {
            ASTNode_get_attr_str_s(func, "name", this->name);
            ASTNode_get_attr_str_s(func, "return", this->returnType);
            
            auto params = ASTNode_querySelector(func, "/Params/ParamDecl");
            params->foreach([&](auto param) {
                string param_type;
                ASTNode_get_attr_str_s(param, "type", param_type);
                paramtersType.push_back(param_type);
            });
        }

        void add_instr(const shared_ptr<Instruction>& instr) {
            body.push_back(instr);
        }

        auto get_current_pos() const {
            return body.size();
        }

        void add_instr_at(size_t pos, const shared_ptr<Instruction>& instr) {
            body.insert(body.begin() + pos, instr);
        }


        string to_readable() const {
            std::stringstream ss;
            ss << returnType << " " << name << "(";
            for (const auto& param_type : paramtersType) {
                ss << param_type << ", ";
            }
            ss << "):" << endl;
            for (const auto& instr: body) {
                ss << "\t" << instr->to_readable() << endl;
            }

            return ss.str();
        }


    };

    class Unit {
    public:
        map<string, std::shared_ptr<Function>> functions;

        bool hasFunction(const string& name) const {
            return functions.find(name) != functions.end();
        }
        string to_readable() const {
            std::stringstream ss;
            for (const auto& [name, func]: functions) {
                ss << func->to_readable() << endl;
            }
            return ss.str();
        }
    };


    class I {
        static auto noArgInstr(InstructionType type) {
            return make_shared<Instruction>(type);
        }

    public:
        static auto nop() {
            return noArgInstr(NOP);
        }

        static auto iconst(int x) {
            auto ret = noArgInstr(ICONST);

            ret->operand1 = x;
            return ret;
        }

        static auto fconst(float x) {
            auto ret = noArgInstr(FCONST);

            ret->operand1 = x;
            return ret;
        }

        static auto i2f() {
            return noArgInstr(I2F);
        }

        static auto f2i() {
            return noArgInstr(F2I);
        }

        static auto iadd() {
            return noArgInstr(IADD);
        }

        static auto isub() {
            return noArgInstr(ISUB);
        }

        static auto imul() {
            return noArgInstr(IMUL);
        }

        static auto idiv() {
            return noArgInstr(IDIV);
        }

        static auto irem() {
            return noArgInstr(IREM);
        }

        static auto ineg() {
            return noArgInstr(INEG);
        }

        static auto inot() {
            return noArgInstr(INOT);
        }

        static auto fadd() {
            return noArgInstr(FADD);
        }

        static auto fsub() {
            return noArgInstr(FSUB);
        }

        static auto fmul() {
            return noArgInstr(FMUL);
        }

        static auto fdiv() {
            return noArgInstr(FDIV);
        }

        static auto fneg() {
            return noArgInstr(FNEG);
        }

        static auto fnot() {
            return noArgInstr(FNOT);
        }

        static auto icmpeq() {
            return noArgInstr(ICMPEQ);
        }

        static auto icmpne() {
            return noArgInstr(ICMPNE);
        }

        static auto icmplt() {
            return noArgInstr(ICMPLT);
        }

        static auto icmpgt() {
            return noArgInstr(ICMPGT);
        }

        static auto icmple() {
            return noArgInstr(ICMPLE);
        }

        static auto icmpge() {
            return noArgInstr(ICMPGE);
        }

        static auto fcmpeq() {
            return noArgInstr(FCMPEQ);
        }

        static auto fcmpne() {
            return noArgInstr(FCMPNE);
        }

        static auto fcmplt() {
            return noArgInstr(FCMPLT);
        }

        static auto fcmpgt() {
            return noArgInstr(FCMPGT);
        }

        static auto fcmple() {
            return noArgInstr(FCMPLE);
        }

        static auto fcmpge() {
            return noArgInstr(FCMPGE);
        }

        static auto iret(int offset) {
            return noArgInstr(IRET);
        }

        static auto fret(int offset) {
            return noArgInstr(FRET);
        }
    };
}

#endif