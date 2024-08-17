#pragma once
#ifndef SIR_TRANSLATOR
#define SIR_TRANSLATOR

#include "ast.h"
#include "sir.hpp"

namespace SIRTranslatorGroup {

using namespace SIR;

class SIRTranslator {
    NodeRef comp_unit;

    std::shared_ptr<Function> cur_function;
    std::shared_ptr<Unit> result;

    void passType(ASTNode *cur, const ASTNode *child, const char *to_attr = "type", const char *from_attr = "type") {
        string type;
        ASTNode_get_attr_str_s(child, from_attr, type);
        ASTNode_set_attr_str(cur, to_attr, type);
    }

public:
    SIRTranslator(NodeRef comp_unit) {
        this->comp_unit = comp_unit;
        result = std::make_shared<Unit>();
    }
    
    auto translate() {
        auto funcs = ASTNode_querySelector(this->comp_unit, "/Scope/FunctionDef/Function");
        
        funcs->foreach([&](auto func) {
            cur_function = make_shared<Function>(func);
            assert(!(result->hasFunction(cur_function->name)));
            result->functions.insert({cur_function->name, cur_function});
            translateFunc(func);
        });

        return this->result;
    }

    void translateFunc(NodeRef func) {
        assert(ASTNode_id_is(func, "Function"));

        int i = 0;
        // 查找所有的参数，为参数生成引用 local slot
        $(func, "/Scope/Decl/ParamDecl")->foreach([&](auto param) {
            ASTNode_set_attr_int(param, "index", i++);
        });

        translateBlock(*$(func, "/Scope/Block"));
    }

    void translateBlock(NodeRef block) {
        assert(ASTNode_id_is(block, "Block"));

        $(block, "*")->foreach([&](auto stmt) {
            translateStmt(stmt);
        });
    }

    void translateStmt(NodeRef stmt) {
        When(stmt, {
            TagMatch<void>("NOP", [&]() {
                cur_function->add_instr(I::nop());
            }),
            TagMatch<void>("Exp", [&]() {
                translateExp(stmt);
            }),
        });
    }

    void translateExp(NodeRef exp) {
        assert(ASTNode_id_is(exp, "Exp"));
        auto inner = *$(exp, "*");
        translateExp(inner);
        passType(exp, inner);
    }

    void translateExpInner(NodeRef exp) {
        When(exp, {
            TagMatch<void>("Number", [&]() {
                string const_type;
                ASTNode_get_attr_str_s(exp, "type", const_type);
                if (const_type == SyInt) {
                    cur_function->add_instr(I::iconst())
                } else if (const_type == SyFloat) {
                    cur_function->add_instr(I::fconst());
                } else {
                    panic("ERR");
                }
            })
        });
    }
};
};


#endif