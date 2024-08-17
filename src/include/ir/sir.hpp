#pragma once

#ifndef SIR_H
#define SIR_H

#include "sysY.h"

namespace SIR {
    enum InstructionType {
        NOP = 0x00,
    };

    class Instruction {
    public:
        InstructionType type;
        
        Instruction(const InstructionType type = NOP) {
            this->type = type;
        }

        string to_readable() const {
            return "TODO";
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
        static Instruction nop() {
            return Instruction(NOP);
        }
    };
}

#endif