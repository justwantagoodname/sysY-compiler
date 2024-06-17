#pragma once

#ifndef STACK_TRANSLATOR_H
#define STACK_TRANSLATOR_H

#include "sysY.h"
#include "ast.h"
#include "adapter.hpp"

/**
 * 用于将AST转换为栈式指令的类，可能会大幅度修改AST
 */
class StackTranslator {
private:
    ASTNode* comp_unit;
    std::unique_ptr<Adapter> adapter;
    std::string tempReg{"r0"};
    std::string accumulatorReg{"r4"};

    void translateFunc(ASTNode* func);
    void translateStmt(ASTNode* stmt);
    void translateExp(ASTNode* exp);
    void translateAssign(ASTNode* assign);
    void translateFetch(ASTNode* fetch);
    void translateCall(ASTNode* call);
    void translateExternCall(ASTNode* call);
    void translateIf(ASTNode* ifstmt);
    void translateWhile(ASTNode* whilestmt);


public:
    StackTranslator(ASTNode* comp_unit, std::unique_ptr<Adapter> adapter) : comp_unit(comp_unit), adapter(std::move(adapter)) {
        assert(comp_unit != nullptr);
        assert(ASTNode_id_is(comp_unit, "CompUnit"));
    }

    void translate();
};

#endif