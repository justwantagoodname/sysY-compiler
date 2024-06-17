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
    adapter->emitLabel(funcName);

    adapter->pushStack({adapter->getFramePointerName(), adapter->getReturnAddressName()});

    // 让栈底寄存器指向栈帧的开始（栈底为上一个栈帧的栈底值）
    adapter->add(adapter->getFramePointerName(), adapter->getStackPointerName(), 4);

    // 查找所有的函数，为函数生成引用 label
    QueryResult *params = ASTNode_querySelector(func, "/Params/ParamDecl"), *cur = nullptr;
    int funcParamIndex = 0;
    DL_FOREACH(params, cur) {
        auto param = cur->node;
        const char* paramName;
        bool hasParamName = ASTNode_get_attr_str(param, "name", &paramName);
        assert(hasParamName);
        // 注意修改Decl中的ParamDecl
    }

    // TODO: 保存局部变量

    // TODO: 实际移动栈顶指针

    // TODO: 翻译函数体

    // TODO: 返回值处理

    // TODO: 返回调用
}
