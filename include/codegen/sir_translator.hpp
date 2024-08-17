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
        
    }
};
};


#endif