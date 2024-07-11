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

public:
    std::string tempReg;
    std::string accumulatorReg;
    std::string floatAccumulatorReg;

    /* Typing 相关 */
    // 计算静态类型

    // 辅助函数直接传递子节点的类型到当前节点
    void passType(ASTNode* cur, const ASTNode* child, const char* to_attr = "type", const char* from_attr = "type");

    void translateTypeConversion(ASTNode* exp, std::string target_type);
    void translateTypePush(ASTNode* exp);
    void translateTypePop(ASTNode* exp);
     
    void translateFunc(ASTNode* func);
    void translateBlock(ASTNode* block);
    void translateStmt(ASTNode* stmt);
    void translateExp(ASTNode* exp);
    /*
     * 在翻译过程中计算属性类型
     */
    void translateExpInner(ASTNode* exp);
    void translateArithmeticOp(ASTNode* exp);
    void translateUnaryOp(ASTNode* exp);

    void translateLVal(ASTNode* lval);
    void translateAssign(ASTNode* assign);
    void translateFetch(ASTNode* fetch);
    void translateCall(ASTNode* call);
    void translateRelOp(ASTNode* exp);
    void translateShortCircuitLogicOp(ASTNode *logic);

    void translateIf(ASTNode* ifstmt);
    void translateWhile(ASTNode* whilestmt);

    void translateBreak(ASTNode* brk);
    void translateContinue(ASTNode* cont);

    void translateReturn(ASTNode* ret);

    /**
     * 翻译栈上变量的初始化
     */
    void translateVarDecl(ASTNode* var_decl);

    StackTranslator(ASTNode* comp_unit, std::unique_ptr<Adapter> adapter) : comp_unit(comp_unit), adapter(std::move(adapter)) {
        assert(comp_unit != nullptr);
        assert(ASTNode_id_is(comp_unit, "CompUnit"));
        /* 
            使用 r4 作为临时寄存器，根据 ATPCS 规范，r4-r11 是callee-saved寄存器
            但是我们实际是仅仅是把 r4 作为临时寄存器，保存从堆栈中取出的值，因此我们在我们的调用约定中无需保存 r4 的值

            同时在调用*外部*函数时，可以使用 r4 作为 r0 的备份，外部的函数会为我们保存 r4 的值
        */
        // TODO: 考虑到不同平台的调用约定，这里可能需要改为由 Adapter 提供
        this->tempReg = "r4";
        this->accumulatorReg = "r0";
        this->floatAccumulatorReg = "s0";
    }

    void translate();
};

#endif