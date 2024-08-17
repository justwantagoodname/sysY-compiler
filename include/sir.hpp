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

        FADD,
        FSUB,
        FMUL,
        FDIV,
        FNEG,

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
                case NOP: return "NOP";
                case ICONST: {
                    return "ICONST " + to_string(get<int>(operand1));
                }
                case FCONST: {
                    return "FCONST " + to_string(get<float>(operand1));
                }
            
                default:
                    return "ERR";
            }
        }
    };

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

        void add_instr(shared_ptr<Instruction> instr) {
            body.push_back(instr);
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
    public:
        static shared_ptr<Instruction> nop() {
            return make_shared<Instruction>(NOP);
        }

        static shared_ptr<Instruction> iconst(int x) {
            
        } 
    };
}

#endif