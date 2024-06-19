#include "codegen/stack_translator.hpp"
#include "utils.h"

void StackTranslator::translate() {
    QueryResult* funcs = ASTNode_querySelector(this->comp_unit, "/Scope/FunctionDef/Function"), *cur;
    DL_FOREACH(funcs, cur) {
        ASTNode* func = cur->node;
        translateFunc(func);
    }
}

/**
 * 翻译函数申明，因为这里是堆栈模式，我们自己的调用约定如下：
 * 1. 函数的返回值保存在 r0 中
 * 2. 函数的参数保存在 r1-r3 中，大于 3 个参数的情况下，多余 3 参数保存在栈中
 * 栈帧定义：
 * 假定有函数 f(int a, int b, int c, int d, int e)
 *  高地址 --------------------------------------------------------------> 低地址
 *  |  参数 d   |  参数 e  |    旧栈底位置      | 返回地址 |  局部变量区 |   临时变量         |
 *  | 如果存在多于 3 个参数  |     ^-栈底指针     |                    |    ^- 栈顶指针     |
 *
 * @note 栈顶指针指向栈顶变量地址
 * @param func 函数 AST 节点
 */
void StackTranslator::translateFunc(ASTNode *func) {
    assert(ASTNode_id_is(func, "Function"));
    const char* funcName;
    bool hasFuncName = ASTNode_get_attr_str(func, "name", &funcName);
    assert(hasFuncName);

    std::string retLabel = std::string(funcName) + "_ret";

    ASTNode_add_attr_str(func, "returnLabel", retLabel.c_str());

    // 创建函数标签
    adapter->emitFunctionLabel(funcName);

    adapter->pushStack({adapter->getFramePointerName(), adapter->getReturnAddressName()});

    // 让栈底寄存器指向栈帧的开始（栈底为上一个栈帧的栈底值）
    adapter->add(adapter->getFramePointerName(), adapter->getStackPointerName(), 4);

    // 查找所有的函数，为函数生成引用 label
    int paramSize = ASTNode_children_size(ASTNode_querySelectorOne(func, "/Params"));
    QueryResult *params = ASTNode_querySelector(func, "/Params/ParamDecl"), *cur = nullptr;
    int funcParamIndex = 0;
    DL_FOREACH(params, cur) {
        auto param = cur->node;
        const char* paramName;
        bool hasParamName = ASTNode_get_attr_str(param, "name", &paramName);
        assert(hasParamName);
        auto param_in_decl = ASTNode_querySelectorfOne(func, "/Scope/Decl/ParamDecl[@name='%s']", paramName);
        assert(param_in_decl != nullptr);
        if (funcParamIndex < 3) {
            // 参数在寄存器中
            ASTNode_add_attr_str(param_in_decl, "pos", "reg");
            ASTNode_add_attr_str(param_in_decl, "reg", adapter->getRegName(funcParamIndex + 1).c_str());
        } else {
            ASTNode_add_attr_str(param_in_decl, "pos", "stack");
            ASTNode_add_attr_int(param_in_decl, "offset", adapter->getWordSize() * (paramSize - funcParamIndex));
        }
        funcParamIndex++;
    }

    // 保存局部变量
    // 计算栈帧局部变量大小
    size_t localVarSize = 0;
    // 查询所有局部变量包括匿名作用域的变量
    QueryResult *localVars = ASTNode_querySelector(func, "//Scope/Decl/Var");
    cur = nullptr;
    DL_FOREACH(localVars, cur) {
        auto var = cur->node;
        if (ASTNode_has_attr(var, "array")) {
            assert(ASTNode_has_attr(var, "size"));
            int size;
            ASTNode_get_attr_int(var, "size", &size);
            localVarSize += adapter->getWordSize() * size;
        } else {
            localVarSize += adapter->getWordSize(); // 单个变量大小
        }
        ASTNode_add_attr_int(var, "offset", -localVarSize - adapter->getWordSize()); // 从栈低开始计算
    }

    // 实际移动栈顶指针
    if (localVarSize > 0)
        adapter->sub(adapter->getStackPointerName(), adapter->getStackPointerName(), localVarSize); 

    // TODO: 翻译函数体 返回值在翻译 return 语句时设置
    auto *block = ASTNode_querySelectorOne(func, "/Scope/Block");

    translateBlock(block);

    // 在函数调用结束后，将返回值保存到 r0 中
    adapter->loadImmediate(adapter->getRegName(0), 0); // 临时设置为 0 方便调试

    adapter->emitLabel(retLabel);
    // 恢复栈顶指针
    adapter->sub(adapter->getStackPointerName(), adapter->getFramePointerName(), 4);
    // 这里直接弹出到 pc，寄存器中实现转跳
    adapter->popStack({adapter->getFramePointerName(), adapter->getPCName()});
    
    adapter->emitComment();
}

void StackTranslator::translateBlock(ASTNode *block) {
    assert(ASTNode_id_is(block, "Block"));

    QueryResult *stmts = ASTNode_querySelector(block, "*"), *cur = nullptr;
    DL_FOREACH(stmts, cur) {
        auto stmt = cur->node;
        translateStmt(stmt);
    }
}

void StackTranslator::translateStmt(ASTNode *stmt) {
    if (ASTNode_id_is(stmt, "Exp")) {
        translateExp(stmt);
    } else if (ASTNode_id_is(stmt, "Assign")) {

    } else if (ASTNode_id_is(stmt, "Fetch")) {

    } else if (ASTNode_id_is(stmt, "If")) {

    } else if (ASTNode_id_is(stmt, "While")) {

    } else if (ASTNode_id_is(stmt, "Return")) {

    } else {
        assert(false);
    }

}

void StackTranslator::translateCall(ASTNode *call) {
    assert(ASTNode_id_is(call, "Call"));
    const char* funcName;
    bool hasFuncName = ASTNode_get_attr_str(call, "name", &funcName);
    assert(hasFuncName);
    if (is_lib_function(funcName)) {
        translateExternCall(call);
    } else {
        assert(0);
    }
}

void StackTranslator::translateExp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Exp"));
    translateExpInner(ASTNode_querySelectorOne(exp, "*"));
}

void StackTranslator::translateExpInner(ASTNode *exp) {
    if (ASTNode_id_is(exp, "Call")) {
        translateCall(exp);
    } else if (ASTNode_id_is(exp, "Number")) {
        int value;
        int hasValue = ASTNode_get_attr_int(exp, "value", &value);
        assert(hasValue);
        adapter->loadImmediate(accumulatorReg, value);
    } else {
        assert(0);
    }
}

/**
 * 调用外部函数
 * @param call
 */
void StackTranslator::translateExternCall(ASTNode *call) {
    assert(ASTNode_id_is(call, "Call"));

    const char* funcName;
    ASTNode_get_attr_str(call, "name", &funcName);

    auto paramSize = ASTNode_children_size(call);

    assert(is_lib_function(funcName));

    // 首先计算所有参数
    if (strcmp(funcName, "starttime") == 0 || strcmp(funcName, "stoptime") == 0) {
        // 这俩函数用到行号特殊处理一下
        int lineno;
        ASTNode_get_attr_int(call, "line", &lineno);
        adapter->mov(tempReg, accumulatorReg);
        adapter->loadImmediate(adapter->getRegName(0), lineno); // TODO: 应该按照平台调用约定来放寄存器
        adapter->call(std::string("_sysy_") + funcName);
        adapter->mov(accumulatorReg, tempReg);
    } else if (strcmp(funcName, "putf") == 0) {
        // putf 和其他可变参数需要特殊处理，因为 GNU C 语法中可变参数在栈上是从右向左入栈的，和我们的约定相反
        adapter->pushStack({accumulatorReg});
        // 计算所有参数
        QueryResult *params = ASTNode_querySelector(call, "Param"), *cur = params->prev;
        std::vector<ASTNode*> paramNodes;
        DL_FOREACH(params, cur) {
            paramNodes.push_back(cur->node);
        }
        for (int idx = paramNodes.size() - 1; idx >= 0; idx--) {
            // 反向遍历参数
            printf("index: %d\n", idx);
            const char* type;
            bool hasType = ASTNode_get_attr_str(paramNodes[idx], "type", &type);
            if (hasType && strcmp(type, "StringConst") == 0) {
                const char* label;
                ASTNode_get_attr_str(paramNodes[idx], "label", &label);
                adapter->loadLabelAddress(accumulatorReg, label);
                // 这里因为 putf 的字符参数必定是第一个，所以不入栈了
                // adapter->pushStack({accumulatorReg});
            } else {
                // 计算参数
                assert(ASTNode_id_is(paramNodes[idx], "Param"));
                ASTNode* inner = ASTNode_querySelectorOne(paramNodes[idx], "*");
                translateExpInner(inner);
                if (idx >= 3) {
                    adapter->pushStack({accumulatorReg});
                } else {
                    adapter->mov(adapter->getRegName(idx), accumulatorReg);
                }
            }
        }
        adapter->call(funcName);
        adapter->popStack({accumulatorReg});
    }

}
