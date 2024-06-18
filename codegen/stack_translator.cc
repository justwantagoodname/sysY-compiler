#include "codegen/stack_translator.hpp"


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
 *  |  参数 d   |  参数 e   |    旧栈底位置      | 返回地址 |  局部变量区 |   临时变量         |
 *  | 如果存在多于 3 个参数  |     ^-栈底指针     |                     |    ^- 栈顶指针     |
 *
 * @note 栈顶指针指向栈顶变量地址
 * @param func 函数 AST 节点
 */
void StackTranslator::translateFunc(ASTNode *func) {
    assert(ASTNode_id_is(func, "Function"));
    const char* funcName;
    bool hasFuncName = ASTNode_get_attr_str(func, "name", &funcName);
    assert(hasFuncName);

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

    // TODO: 保存局部变量

    // TODO: 实际移动栈顶指针

    // TODO: 翻译函数体

    // TODO: 返回值处理
    // 在函数调用结束后，将返回值保存到 r0 中
    // 需要研究一下，是将
    adapter->loadImmediate(adapter->getRegName(0), 0); // 临时设置为 0 方便调试

    adapter->emitLabel(std::string(funcName) + "_ret");
    // 恢复栈顶指针
    adapter->sub(adapter->getStackPointerName(), adapter->getFramePointerName(), 4);
    // 这里直接弹出到 pc，寄存器中实现转跳
    adapter->popStack({adapter->getFramePointerName(), adapter->getPCName()});
    
    adapter->emitSeparator();
}
