#include "codegen/stack_translator.hpp"
#include "utils.h"

void passAttr(ASTNode* cur, const ASTNode *inner, const char* to_attr = "type", const char* from_attr = "type") {
    const char *type;
    bool hasInner = ASTNode_get_attr_str(inner, from_attr, &type);
    assert(hasInner);
    if (hasInner) {
        ASTNode_add_attr_str(cur, to_attr, type);
    }
}

void StackTranslator::translate() {
    QueryResult *funcs = ASTNode_querySelector(this->comp_unit, "/Scope/FunctionDef/Function"), *cur;
    DL_FOREACH(funcs, cur) {
        ASTNode *func = cur->node;
        translateFunc(func);
    }
}

/**
 * 翻译函数申明，因为这里是堆栈模式，我们自己的调用约定如下：
 * 1. 函数的返回值保存在 r0 中
 * 2. 函数的参数全部保存栈上，放在old fp之前
 * 3. 参数的入栈顺序是从右到左
 * 栈帧定义：
 * 假定有函数 f(int a, int b)
 *  高地址 --------------------------------------------------------------> 低地址
 *  |  参数  b  |  参数 a  |    旧栈底位置      | 返回地址 |  局部变量区 |   临时变量         |
 *  |        参数区        |     ^-栈底指针     |                    |    ^- 栈顶指针     |
 *
 * @note 栈顶指针指向栈顶变量地址
 * @param func 函数 AST 节点
 */
void StackTranslator::translateFunc(ASTNode *func) {
    assert(ASTNode_id_is(func, "Function"));
    const char *funcName;
    bool hasFuncName = ASTNode_get_attr_str(func, "name", &funcName);
    assert(hasFuncName);

    std::string retLabel = generateLabel();

    ASTNode_add_attr_str(func, "returnLabel", retLabel.c_str());

    // 创建函数标签
    adapter->emitFunctionLabel(funcName);

    adapter->pushStack({adapter->getFramePointerName(), adapter->getReturnAddressName()});

    // 让栈底寄存器指向栈帧的开始（栈底为上一个栈帧的栈底值）
    adapter->add(adapter->getFramePointerName(), adapter->getStackPointerName(), 4);

#ifdef DEBUG
    adapter->emitComment("栈帧建立好了");
#endif

    // 查找所有的函数，为函数生成引用 label
    int paramSize = ASTNode_children_size(ASTNode_querySelectorOne(func, "/Params"));
    QueryResult *params = ASTNode_querySelector(func, "/Params/ParamDecl"), *cur = nullptr;
    int funcParamIndex = 0;
    DL_FOREACH(params, cur) {
        auto param = cur->node;
        const char *paramName;
        bool hasParamName = ASTNode_get_attr_str(param, "name", &paramName);
        assert(hasParamName);
        auto param_in_decl = ASTNode_querySelectorfOne(func, "/Scope/Decl/ParamDecl[@name='%s']", paramName);
        assert(param_in_decl != nullptr);
#if 0
        if (funcParamIndex < 3) {
            // 参数在寄存器中
            ASTNode_add_attr_str(param_in_decl, "pos", "reg");
            ASTNode_add_attr_str(param_in_decl, "reg", adapter->getRegName(funcParamIndex + 1).c_str());
        } else {
#endif
            ASTNode_add_attr_str(param_in_decl, "pos", "stack");
            ASTNode_add_attr_int(param_in_decl, "offset", adapter->getWordSize() * (paramSize - funcParamIndex));
#if 0
        }
#endif
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

#ifdef DEBUG
    adapter->emitComment("局部变量空间分配好了");
#endif

    // TODO: 翻译函数体 返回值在翻译 return 语句时设置
    auto *block = ASTNode_querySelectorOne(func, "/Scope/Block");

    translateBlock(block);

    // 在 sysy 中，有返回值的函数没有返回是未定义行为，所以这里直接返回 0，也是合理的
    adapter->loadImmediate(adapter->getRegName(0), 0);

    // TODO: 考虑合并到 return 语句中

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
        translateAssign(stmt);
    } else if (ASTNode_id_is(stmt, "If")) {
        // TODO
    } else if (ASTNode_id_is(stmt, "While")) {
        // TODO
    } else if (ASTNode_id_is(stmt, "Return")) {
        translateReturn(stmt);
    } else if (ASTNode_id_is(stmt, "Scope")) {
        // 已经在函数入口处理好局部变量了
        auto inner_block = ASTNode_querySelectorOne(stmt, "Block");
        translateBlock(inner_block);
    } else if (ASTNode_id_is(stmt, "NOP")) {
        adapter->nop();
    } else {
        assert(false);
    }

}

void StackTranslator::translateCall(ASTNode *call) {
    assert(ASTNode_id_is(call, "Call"));
    const char *funcName;
    bool hasFuncName = ASTNode_get_attr_str(call, "name", &funcName);
    assert(hasFuncName);
    if (is_lib_function(funcName)) {
        translateExternCall(call);
        return;
    }

    // 寻找函数声明
    auto func = ASTNode_querySelectorfOne(this->comp_unit, "/Scope/FunctionDef/Function[@name='%s']", funcName);
    assert(func);

    passAttr(call, func, "type", "return");

    int paramSize = ASTNode_children_size(call);

    assert(paramSize == ASTNode_children_size(ASTNode_querySelectorOne(func, "/Params"))); // 参数数量必须一致

    // TODO: 做类型检查

    if (paramSize) {
        // 计算所有参数
        QueryResult *params = ASTNode_querySelector(call, "Param"), *cur = params->prev;
        int idx = paramSize - 1;
        do {
            // 反向遍历参数
            const char *type;
            bool hasType = ASTNode_get_attr_str(cur->node, "type", &type);
            if (hasType && strcmp(type, "StringConst") == 0) {
                // 字符串常量 特殊处理，因为没有字符串常量类型
                const char *label;
                ASTNode_get_attr_str(cur->node, "label", &label);
                adapter->loadLabelAddress(accumulatorReg, label);
            } else {
                // 计算参数
                assert(ASTNode_id_is(cur->node, "Param"));
                ASTNode *inner = ASTNode_querySelectorOne(cur->node, "*");
                translateExpInner(inner);
            }
            adapter->pushStack({accumulatorReg}); // 第一个参数不需要 push
            cur = cur->prev;
            idx--;
        } while (cur != params->prev);
    }
    adapter->call(funcName);
}

void StackTranslator::translateExp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Exp"));
    auto inner_node = ASTNode_querySelectorOne(exp, "*");
    translateExpInner(inner_node);
    passAttr(exp, inner_node);
}

void StackTranslator::translateExpInner(ASTNode *exp) {
    static std::set<std::string> relOp = {"Or", "And", "Equal", "NotEq", "Less", "LessEq", "Greater", "GreaterEq",
                                          "Not"};
    static std::set<std::string> arithOp = {"Plus", "Minus", "Mult", "Div", "Mod"};
    static std::set<std::string> unaryOp = {"UnPlus", "UnMinus"};
    if (ASTNode_id_is(exp, "Call")) {
        translateCall(exp);
    } else if (ASTNode_id_is(exp, "Number")) {
        int value;
        int hasValue = ASTNode_get_attr_int(exp, "value", &value);
        assert(hasValue);
        adapter->loadImmediate(accumulatorReg, value);
    } else if (ASTNode_id_is(exp, "Fetch")) {
        translateFetch(exp);
    } else if (relOp.find(exp->id) != relOp.end()) {
        // 逻辑运算
    } else if (arithOp.find(exp->id) != arithOp.end()) {
        // 算术运算
        translateArithmeticOp(exp);
    } else if (unaryOp.find(exp->id) != unaryOp.end()) {
        // 一元运算
        if (ASTNode_id_is(exp, "UnPlus")) {
            auto inner = ASTNode_querySelectorOne(exp, "*");
            translateExpInner(inner);
        } else if (ASTNode_id_is(exp, "UnMinus")) {
            auto inner = ASTNode_querySelectorOne(exp, "*");
            translateExpInner(inner);
            adapter->neg(accumulatorReg, accumulatorReg);
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }
}

void StackTranslator::translateArithmeticOp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Plus")
           || ASTNode_id_is(exp, "Minus")
           || ASTNode_id_is(exp, "Mult")
           || ASTNode_id_is(exp, "Div")
           || ASTNode_id_is(exp, "Mod"));

    ASTNode *lhs = ASTNode_querySelectorOne(exp, "*[0]"), *rhs = ASTNode_querySelectorOne(exp, "*[1]");

    assert(lhs != nullptr && rhs != nullptr);

    translateExpInner(lhs);
    adapter->pushStack({accumulatorReg});
    translateExpInner(rhs);
    adapter->popStack({tempReg});

    const char *rhs_type, *lhs_type;
    bool rhs_has_type = ASTNode_get_attr_str(rhs, "type", &rhs_type);
    bool lhs_has_type = ASTNode_get_attr_str(lhs, "type", &lhs_type);
    
    assert(rhs_has_type && lhs_has_type);
    assert(strcmp(lhs_type, "Void") != 0 && strcmp(rhs_type, "Void") != 0);

    if (strcmp(lhs_type, "Float") == 0 || strcmp(rhs_type, "Float") == 0) {
        // 浮点数运算类型变为浮点
        ASTNode_add_attr_str(exp, "type", "Float");
    } else {
        // 整数运算类型变为整数
        ASTNode_add_attr_str(exp, "type", "Int");
    }

    if (ASTNode_id_is(exp, "Plus")) {
        adapter->add(accumulatorReg, tempReg, accumulatorReg);
    } else if (ASTNode_id_is(exp, "Minus")) {
        adapter->sub(accumulatorReg, tempReg, accumulatorReg);
    } else if (ASTNode_id_is(exp, "Mult")) {
        adapter->mul(accumulatorReg, tempReg, accumulatorReg);
    } else if (ASTNode_id_is(exp, "Div")) {
        adapter->div(accumulatorReg, tempReg, accumulatorReg);
    } else if (ASTNode_id_is(exp, "Mod")) {
        adapter->mod(accumulatorReg, tempReg, accumulatorReg);
    }
}

const char* lib_function_get_return_type(const char* type) {
    static std::map<std::string, std::string> lib_func_return_type = {
        {"starttime", "Void"},
        {"stoptime", "Void"},
        {"getarray", "Int"},
        {"getch", "Int"},
        {"getfarray", "Int"},
        {"getfloat", "Float"},
        {"getint", "Int"},
        {"putarray", "Void"},
        {"putch", "Void"},
        {"putf", "Void"},
        {"putfarray", "Void"},
        {"putfloat", "Void"},
        {"putint", "Void"},
    };
    return lib_func_return_type[type].c_str();
}

/**
 * 调用外部函数
 * @param call
 */
void StackTranslator::translateExternCall(ASTNode *call) {
    assert(ASTNode_id_is(call, "Call"));

    const char *funcName;
    ASTNode_get_attr_str(call, "name", &funcName);

    int paramSize = ASTNode_children_size(call);

    assert(is_lib_function(funcName));
    // TODO: 检查参数数量是否正确和参数类型是否正确

    const char* ret_type = lib_function_get_return_type(funcName);
    ASTNode_add_attr_str(call, "type", ret_type);

    // 首先计算所有参数
    if (strcmp(funcName, "starttime") == 0 || strcmp(funcName, "stoptime") == 0) {
        // 这俩函数用到行号特殊处理一下
        int lineno;
        ASTNode_get_attr_int(call, "line", &lineno);

        adapter->loadImmediate(adapter->getRegName(0), lineno); // TODO: 应该按照平台调用约定来放寄存器
        adapter->call(std::string("_sysy_") + funcName);
    } else {
        // putf 和其他可变参数需要特殊处理，因为 GNU C 语法中可变参数在栈上是从右向左入栈的，和我们的约定相反
        if (paramSize) {
            // 计算所有参数
            QueryResult *params = ASTNode_querySelector(call, "Param"), *cur = params->prev;
            int idx = paramSize - 1;
            do {
                // 反向遍历参数
                const char *type;
                bool hasType = ASTNode_get_attr_str(cur->node, "type", &type);
                if (hasType && strcmp(type, "StringConst") == 0) {
                    // 字符串常量 特殊处理，因为没有字符串常量类型
                    const char *label;
                    ASTNode_get_attr_str(cur->node, "label", &label);
                    adapter->loadLabelAddress(accumulatorReg, label);
                } else {
                    // 计算参数
                    assert(ASTNode_id_is(cur->node, "Param"));
                    ASTNode *inner = ASTNode_querySelectorOne(cur->node, "*");
                    translateExpInner(inner);
                }
                if (idx != 0) adapter->pushStack({accumulatorReg}); // 第一个参数不需要 push
                cur = cur->prev;
                idx--;
            } while (cur != params->prev);
            // TODO: 这里需要为putf修改float传递方式
            int reg_param_size = std::min(4, paramSize);
            if (reg_param_size - 1 > 0) {
                std::vector<std::string> regs;
                for (int i = 1; i < reg_param_size; i++) {
                    regs.push_back(adapter->getRegName(i));
                }
                adapter->popStack(regs);
            }
        }
        adapter->call(funcName);
    }
}

void StackTranslator::translateFetch(ASTNode *fetch) {
    assert(ASTNode_id_is(fetch, "Fetch"));

    auto address = ASTNode_querySelectorOne(fetch, "Address");
    assert(address);
    translateLVal(address);

    const char* type;
    bool hasType = ASTNode_get_attr_str(address, "type", &type);
    assert(hasType);
    ASTNode_add_attr_str(fetch, "type", type);

    adapter->loadRegister(accumulatorReg, accumulatorReg, 0);
}

/**
 * 翻译左值, 由于左值是一个地址, 计算完成的结果*地址*会放在 accumulatorReg 中
 * @param lval
 */
void StackTranslator::translateLVal(ASTNode *lval) {
    assert(ASTNode_id_is(lval, "Address"));

    auto address = lval;

    const char *name;
    bool hasName = ASTNode_get_attr_str(address, "base", &name);
    assert(hasName);

    auto decl = ASTNode_querySelectorfOne(address, "/ancestor::Scope/Decl/*[@name='%s']", name);
    assert(decl); // 使用的变量名必须声明过

    const char* label, *type;
    bool hasType = ASTNode_get_attr_str(decl, "type", &type);
    assert(hasType);
    ASTNode_add_attr_str(address, "type", type);

    bool is_array = ASTNode_has_attr(decl, "array");

    std::vector<int> dim_sizes; // 数组的声明顺序

    if (is_array) {
        // 是数组，获取数组的索引
        // 获取访问的顺序
        auto *locator = ASTNode_querySelectorOne(address, "/Locator");
        assert(locator); // 访问数组必须有 locator 元素

        QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize/Dimension/Exp/Number"), *cur = nullptr; // 必须确保所有数组大小被计算好了

        DL_FOREACH(dims, cur) {
            int size;
            ASTNode_get_attr_int(cur->node, "value", &size);
            // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
            dim_sizes.push_back(size);
        }

        // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
        dim_sizes.push_back(adapter->getWordSize()); // 最后一个地址是元素大小

        for (auto i = dim_sizes.size() - 1; i > 0; i--) {
            dim_sizes[i - 1] = dim_sizes[i - 1] * dim_sizes[i];
        }

        // 参数声明中的数组第一维大小是缺失的所以不用删除第一个了
        if (!ASTNode_id_is(decl, "ParamDecl")) {
            dim_sizes.erase(dim_sizes.begin());
        }

        // 访问数组 依次计算索引，这里翻译为一个连加，因为我们可以控制过程，所以优化一下
        QueryResult *locators = ASTNode_querySelector(locator, "/Dimension/*");
        int idx = 0;
        DL_FOREACH(locators, cur) {
            // 计算索引
            translateExpInner(cur->node);

            // 索引类型校验
            const char* locator_type;
            bool hasLocatorType = ASTNode_get_attr_str(cur->node, "type", &locator_type);
            assert(hasLocatorType);
            assert(strcmp(locator_type, "Int") == 0);

            // 乘以维度大小
            if (dim_sizes[idx] != 1) {
                adapter->loadImmediate(tempReg, dim_sizes[idx]);
                adapter->mul(accumulatorReg, accumulatorReg, tempReg);
            }

            if (idx != 0) {
                // 如果不是第一个维度，先加上前面的维度大小
                adapter->popStack({tempReg});
                adapter->add(accumulatorReg, accumulatorReg, tempReg);
            }

            if (idx != dim_sizes.size() - 1) {
                adapter->pushStack({accumulatorReg}); // 如果后面还有维度，先保存
            }

            idx++;
        }
    }

    bool hasLabel = ASTNode_get_attr_str(decl, "label", &label);
    if (hasLabel) {
        // 全局变量
        if (is_array) {
            adapter->loadLabelAddress(tempReg, label); // 先加载基址
            adapter->add(accumulatorReg, tempReg, accumulatorReg); // 如果是数组，那么确定实际的地址
        } else {
            adapter->loadLabelAddress(accumulatorReg, label);
        }
    } else {
        int offset;
        bool hasOffset = ASTNode_get_attr_int(decl, "offset", &offset);
        assert(hasOffset);
        // 局部变量
        // TODO: 这里目前还有bug，如果是超级大的栈帧，那么这里的 offset 是不对的，不过应该交给loadRegister来处理
        if (is_array) {
            adapter->add(tempReg, adapter->getFramePointerName(), offset); // 先计算基址
            adapter->add(accumulatorReg, tempReg, accumulatorReg); // 然后确定实际地址
        } else {
            adapter->add(accumulatorReg, adapter->getFramePointerName(), offset);
        }
    }
}

void StackTranslator::translateAssign(ASTNode *assign) {
    assert(ASTNode_id_is(assign, "Assign"));

    auto lval = ASTNode_querySelectorOne(assign, "/Dest/Address");
    auto exp = ASTNode_querySelectorOne(assign, "Exp");

    assert(lval && exp);

    translateExp(exp);

    adapter->pushStack({accumulatorReg});

    translateLVal(lval);

    adapter->popStack({tempReg});

    const char* lval_type, *exp_type;
    bool hasLValType = ASTNode_get_attr_str(lval, "type", &lval_type);
    bool hasExpType = ASTNode_get_attr_str(exp, "type", &exp_type);
    assert(hasLValType && hasExpType);

    assert(strcmp(lval_type, exp_type) == 0);

    // TODO: 这里需要根据类型来判断是否需要转换类型

    adapter->storeRegister(tempReg, accumulatorReg, 0);
}

void StackTranslator::translateReturn(ASTNode *ret) {
    assert(ASTNode_id_is(ret, "Return"));

    auto exp = ASTNode_querySelectorOne(ret, "Exp");
    if (exp) {
        translateExp(exp);
    }

    auto func = ASTNode_querySelectorOne(ret, "/ancestor::Function");
    const char* ret_label;
    bool hasRetLabel = ASTNode_get_attr_str(func, "returnLabel", &ret_label);
    assert(hasRetLabel);

    adapter->jump(ret_label);

}
