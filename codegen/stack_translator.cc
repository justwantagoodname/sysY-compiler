#include "codegen/stack_translator.hpp"
#include "utils.h"

// 因为前期设计的问题，临时从数组声明中计算实际的类型吧
std::string get_array_decl_type(const ASTNode* array_decl) {
    assert(ASTNode_has_attr(array_decl, "array"));
    int dim_size = 0;
    if (ASTNode_id_is(array_decl, "ParamDecl")) {
        dim_size = ASTNode_children_size(array_decl);
    } else {
        dim_size = ASTNode_children_size(ASTNode_querySelectorOne(array_decl, "/ArraySize"));
    }
    const char* base_type;
    bool hasBaseType = ASTNode_get_attr_str(array_decl, "type", &base_type);
    assert(hasBaseType);

    std::string type = base_type;
    for (int i = 0; i < dim_size; i++) {
        type.insert(0, "[");
    }
    return type;
}

void StackTranslator::translate() {
    adapter->preGenerate();
    QueryResult *funcs = ASTNode_querySelector(this->comp_unit, "/Scope/FunctionDef/Function"), *cur;
    DL_FOREACH(funcs, cur) {
        ASTNode *func = cur->node;
        translateFunc(func);
    }
    adapter->postGenerate();
}

/**
 * 翻译函数申明，因为这里是堆栈模式，我们自己的调用约定如下：
 * 1. 函数的返回值保存在 r0 中，如果是浮点数，那么保存在 浮点寄存器 s0 中
 * 2. 函数的参数全部保存栈上，放在old fp之前
 * 3. 参数的入栈顺序是从右到左
 * 栈帧定义：
 * 假定有函数 f(int a, int b)
 *  高地址 --------------------------------------------------------------> 低地址
 *  |  参数  b  |  参数 a  |    返回地址      | 旧栈底位置 |  局部变量区 |   临时变量         |
 *  |        参数区        |     ^-栈底指针   |                    |    ^- 栈顶指针     |
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

    // Update: Place Variable To NEON Register

    int reg_number = 2; // s0 s1 was used
    auto hasRemainReg = [&reg_number]() -> bool {
        return reg_number <= 63;
    };
    auto regAlloc = [&reg_number]() -> string {
        assert(reg_number <= 63);
        return "d" + std::to_string(reg_number / 2) + "[" + std::to_string(reg_number % 2) + "]";
    };

    // 查找所有的参数，为参数生成引用 label
    QueryResult *params = ASTNode_querySelector(func, "/Params/ParamDecl"), *cur = nullptr;
    // 这里从 1 开始，因为 0 是 old fp
    int funcParamIndex = 1;
    DL_FOREACH(params, cur) {
        auto param = cur->node;
        const char *paramName;
        bool hasParamName = ASTNode_get_attr_str(param, "name", &paramName);
        assert(hasParamName);
        auto param_in_decl = ASTNode_querySelectorfOne(func, "/Scope/Decl/ParamDecl[@name='%s']", paramName);
        assert(param_in_decl != nullptr);
        ASTNode_add_attr_str(param_in_decl, "pos", "stack");
        ASTNode_add_attr_int(param_in_decl, "offset", adapter->getWordSize() * funcParamIndex);
        funcParamIndex++;

        if (hasRemainReg()) {
            ASTNode_add_attr_str(param_in_decl, "alias", regAlloc().c_str());
            reg_number++;
        }
    }

    // 保存局部变量
    // 计算栈帧局部变量大小
    size_t localVarSize = 0;
    // 查询所有局部变量包括匿名作用域的变量
    QueryResult *localVars = ASTNode_querySelector(func, "//Scope/Decl/Var");
    DL_FOREACH(localVars, cur) {
        auto var = cur->node;
        if (ASTNode_has_attr(var, "array")) {
            assert(ASTNode_has_attr(var, "size"));
            int size;
            ASTNode_get_attr_int(var, "size", &size);
            localVarSize += adapter->getWordSize() * size;
        } else {
            localVarSize += adapter->getWordSize(); // 单个变量大小
            if (hasRemainReg()) {
                // only variable will place to register
                ASTNode_add_attr_str(var, "alias", regAlloc().c_str());
                reg_number++;
            }
        }
        ASTNode_add_attr_int(var, "offset", -localVarSize - adapter->getWordSize()); // 从栈低开始计算
    }

    // 实际移动栈顶指针
    if (localVarSize > 0) {
        if (localVarSize % 8 != 0) {
            localVarSize += 4; // 对齐到 8 字节方便来调用 putf
        }
        adapter->sub(adapter->getStackPointerName(), adapter->getStackPointerName(), localVarSize);
    }

#ifdef DEBUG
    adapter->emitComment("局部变量空间分配好了");
#endif

    auto *block = ASTNode_querySelectorOne(func, "/Scope/Block");

    translateBlock(block);

    adapter->emitLabel(retLabel);
    // 在 sysy 中，有返回值的函数没有返回是未定义行为，所以这里直接返回 0，也是合理的
    adapter->loadImmediate(adapter->getRegName(0), 0);

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
        translateIf(stmt);
    } else if (ASTNode_id_is(stmt, "While")) {
        translateWhile(stmt);
    } else if (ASTNode_id_is(stmt, "Return")) {
        translateReturn(stmt);
    } else if (ASTNode_id_is(stmt, "Scope")) {
        // 已经在函数入口处理好局部变量了
        auto inner_block = ASTNode_querySelectorOne(stmt, "Block");
        translateBlock(inner_block);
    } else if (ASTNode_id_is(stmt, "NOP")) {
        adapter->nop();
    } else if (ASTNode_id_is(stmt, "Var")) {
        translateVarDecl(stmt);
    } else if (ASTNode_id_is(stmt, "Break")) {
        translateBreak(stmt);
    } else if (ASTNode_id_is(stmt, "Continue")) {
        translateContinue(stmt);
    } else {
        assert(false);
    }

}

void StackTranslator::translateCall(ASTNode *call) {
    adapter->emitComment("Call 开始");
    assert(ASTNode_id_is(call, "Call"));
    const char *funcName;
    bool hasFuncName = ASTNode_get_attr_str(call, "name", &funcName);
    assert(hasFuncName);

    if (adapter->isExternalFunction(funcName)) {
        adapter->emitExternFunction(funcName, call, this);
        return;
    }

    // 寻找函数声明
    auto func = ASTNode_querySelectorfOne(this->comp_unit, "/Scope/FunctionDef/Function[@name='%s']", funcName);
    assert(func);

    // 传递函数调用的类型
    passType(call, func, "type", "return");

    int paramSize = ASTNode_children_size(call);
    int shimSize = 0; // 对齐到 8 字节的填充
    std::vector<std::string> function_params;

    QueryResult *params = ASTNode_querySelector(func, "/Params/ParamDecl"), *cur;
    DL_FOREACH(params, cur) {
        auto param = cur->node;
        const char *paramType;
        if (ASTNode_has_attr(param, "array")) {
            function_params.push_back(get_array_decl_type(param));
        } else {
            bool hasParamType = ASTNode_get_attr_str(param, "type", &paramType);
            assert(hasParamType);
            function_params.push_back(paramType);
        }
    }
    assert(paramSize == function_params.size()); // 参数数量必须一致

    // 调用前，将栈对齐到 8 字节
    if ((paramSize * adapter->getWordSize()) % 8 != 0) {
        shimSize = (paramSize * adapter->getWordSize()) % 8;
        adapter->sub(adapter->getStackPointerName(), adapter->getStackPointerName(), shimSize);
    }

    if (paramSize) {
        // 计算所有参数
        QueryResult *params = ASTNode_querySelector(call, "Param"), *cur = params->prev;
        int idx = paramSize - 1;
        do {
            // 反向遍历参数
            const char *type;
            bool hasType = ASTNode_get_attr_str(cur->node, "type", &type);
            if (hasType && strcmp(type, "StringConst") == 0) {
                assert(false); // 直接报错，因为没有字符串常量类型
            } else {
                // 计算参数
                ASTNode *inner = ASTNode_querySelectorOne(cur->node, "*");
                translateExpInner(inner);
                hasType = ASTNode_get_attr_str(inner, "type", &type);
                assert(hasType);
                // 参数类型校验
                if (strcmp(type, function_params[idx].c_str()) != 0) {
                    translateTypeConversion(inner, function_params[idx]);
                }
                passType(cur->node, inner);
            }
            translateTypePush(cur->node);
            cur = cur->prev;
            idx--;
        } while (cur != params->prev);
    }
    adapter->call(funcName);
    adapter->add(adapter->getStackPointerName(), adapter->getStackPointerName(),
                 paramSize * adapter->getWordSize() + shimSize);
}

void StackTranslator::translateExp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Exp"));
    auto inner_node = ASTNode_querySelectorOne(exp, "*");
    translateExpInner(inner_node);

    // 传递内部节点的类型到现在这个节点
    passType(exp, inner_node);
}

void StackTranslator::translateExpInner(ASTNode *exp) {
    static std::set<std::string> logicOp = {"Or", "And"};
    static std::set<std::string> relOp = { "Equal", "NotEq", "Less", "LessEq", "Greater", "GreaterEq"};
    static std::set<std::string> arithOp = {"Plus", "Minus", "Mult", "Div", "Mod"};
    static std::set<std::string> unaryOp = {"UnPlus", "UnMinus", "Not"};
    if (ASTNode_id_is(exp, "Call")) {
        translateCall(exp);
    } else if (ASTNode_id_is(exp, "Number")) {
        const char *type;
        bool hasType = ASTNode_get_attr_str(exp, "type", &type);
        assert(hasType);
        if (strcmp(type, SyFloat) == 0) {
            float value;
            int hasValue = ASTNode_get_attr_float(exp, "value", &value);
            assert(hasValue);
            adapter->loadImmediate(floatAccumulatorReg, value);
        } else {
            int value;
            int hasValue = ASTNode_get_attr_int(exp, "value", &value);
            assert(hasValue);
            adapter->loadImmediate(accumulatorReg, value);
        }
    } else if (ASTNode_id_is(exp, "Fetch")) {
        translateFetch(exp);
    } else if (logicOp.find(exp->id) != logicOp.end()) {
        translateShortCircuitLogicOp(exp);
    } else if (relOp.find(exp->id) != relOp.end()) {
        // 逻辑运算
        translateRelOp(exp);
    } else if (arithOp.find(exp->id) != arithOp.end()) {
        // 算术运算
        translateArithmeticOp(exp);
    } else if (unaryOp.find(exp->id) != unaryOp.end()) {
        translateUnaryOp(exp);
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

    const char *rhs_type, *lhs_type;

    translateExpInner(lhs);
    translateTypePush(lhs);
    bool lhs_has_type = ASTNode_get_attr_str(lhs, "type", &lhs_type);

    translateExpInner(rhs);
    bool rhs_has_type = ASTNode_get_attr_str(rhs, "type", &rhs_type);


    
    assert(rhs_has_type && lhs_has_type);
    assert(strcmp(lhs_type, SyVoid) != 0 && strcmp(rhs_type, SyVoid) != 0);

    if (strcmp(lhs_type, SyFloat) == 0 ^ strcmp(rhs_type, SyFloat) == 0) {
        // 仅有一边为浮点的情况需要转换
        if (strcmp(lhs_type, SyFloat) == 0) {
            // 转换右边为浮点数
            translateTypeConversion(rhs, SyFloat); // s0 <- r0
            adapter->fpopStack({floatTempReg}); // s1 <- lhs
        } else if (strcmp(rhs_type, SyFloat) == 0) {
            // 转换左边为浮点数
            adapter->fpopStack({floatTempReg});
            adapter->i2f(floatTempReg, floatTempReg);
        }
    } else if (strcmp(lhs_type, SyFloat) == 0 && strcmp(rhs_type, SyFloat) == 0) {
        // 整数运算类型变为整数
        adapter->fpopStack({floatTempReg});
    } else {
        adapter->popStack({tempReg});
    }

    // 确定当前节点的类型
    std::string cur_type;
    if (strcmp(lhs_type, SyFloat) == 0 || strcmp(rhs_type, SyFloat) == 0) {
        cur_type = SyFloat;
    } else {
        cur_type = SyInt;
    }
    ASTNode_add_attr_str(exp, "type", cur_type.c_str());

    if (ASTNode_id_is(exp, "Plus")) {
        if (cur_type == SyInt) adapter->add(accumulatorReg, tempReg, accumulatorReg);
        else adapter->fadd(floatAccumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "Minus")) {
        if (cur_type == SyInt) adapter->sub(accumulatorReg, tempReg, accumulatorReg);
        else adapter->fsub(floatAccumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "Mult")) {
        if (cur_type == SyInt) adapter->mul(accumulatorReg, tempReg, accumulatorReg);
        else adapter->fmul(floatAccumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "Div")) {
        // 对 ARM 平台特殊处理
        if (cur_type == SyInt) {
            if (adapter->platformName() == "ARM32" && accumulatorReg == adapter->getRegName(0)) {
                adapter->mov(adapter->getRegName(1), accumulatorReg); // 除数
                adapter->mov(adapter->getRegName(0), tempReg); // 被除数
                adapter->div(accumulatorReg, adapter->getRegName(0), adapter->getRegName(1));
            } else {
                adapter->div(accumulatorReg, tempReg, accumulatorReg);
            }
        } else {
            adapter->fdiv(floatAccumulatorReg, floatTempReg, floatAccumulatorReg);
        }
    } else if (ASTNode_id_is(exp, "Mod")) {
        // 对 ARM 平台特殊处理
        if (cur_type == SyInt) {
            if (adapter->platformName() == "ARM32" && accumulatorReg == adapter->getRegName(0)) {
                adapter->mov(adapter->getRegName(1), accumulatorReg); // 除数
                adapter->mov(adapter->getRegName(0), tempReg); // 被除数
                adapter->mod(accumulatorReg, adapter->getRegName(0), adapter->getRegName(1));
            } else {
                adapter->mod(accumulatorReg, tempReg, accumulatorReg);
            }
        } else {
            adapter->fmod(floatAccumulatorReg, floatTempReg, floatAccumulatorReg);
        }
    }
}

void StackTranslator::translateFetch(ASTNode *fetch) {
    assert(ASTNode_id_is(fetch, "Fetch"));

    auto address = ASTNode_querySelectorOne(fetch, "Address");
    assert(address);
    auto ret = translateLVal(address);

    const char* lval_type;

    bool hasType = ASTNode_get_attr_str(address, "type", &lval_type);
    assert(hasType);

    auto fetch_type = deref_lval(lval_type);

    ASTNode_add_attr_str(fetch, "type", fetch_type.c_str());

    if (is_array_type(fetch_type)) {
        // 如果左值是地址，那么值是本身的地址
        if (!ret.empty()) {
            adapter->fmov(accumulatorReg, ret);
        }
    } else {
        // 如果左值是值，那么值是地址指向的值
        if (fetch_type == SyInt) {
            if (!ret.empty()) { // the vale was put on neon reg
                adapter->fmov(accumulatorReg, ret);
            } else {
                adapter->loadRegister(accumulatorReg, accumulatorReg, 0);
            }
        } else if (fetch_type == SyFloat) {
            if (!ret.empty()) {
                adapter->fmov(floatAccumulatorReg, ret);
            } else {
                adapter->floadRegister(floatAccumulatorReg, accumulatorReg, 0);
            }

        } else {
            assert(0);
        }
    }
}

/**
 * 翻译左值, 由于左值是一个地址, 计算完成的结果*地址*会放在 accumulatorReg 中，对于参数中的数组类型会先做一次加载确保结果始终是数组的基址
 * @param lval
 */
string StackTranslator::translateLVal(ASTNode *lval) {
    assert(ASTNode_id_is(lval, "Address"));

    auto address = lval;

    const char *name;
    bool hasName = ASTNode_get_attr_str(address, "base", &name);
    assert(hasName);

    int access_line, access_col;
    bool hasLine = ASTNode_get_attr_int(address, "line", &access_line);
    bool hasCol = ASTNode_get_attr_int(address, "column", &access_col);
    assert(hasLine && hasCol);

    QueryResult *decl_list = ASTNode_querySelectorf(address, "/ancestor::Scope/Decl/*[@name='%s']", name), *cur;

    ASTNode* decl = nullptr;
    DL_FOREACH(decl_list, cur) {
        int line, col;
        hasLine = ASTNode_get_attr_int(cur->node, "line", &line);
        hasCol = ASTNode_get_attr_int(cur->node, "column", &col);
        assert(hasLine && hasCol);

        if (line < access_line || (line == access_line && col < access_col)) { // 不能访问自己
            decl = cur->node;
            break;
        }
    }

    assert(decl); // 使用的变量名必须声明过

    const char* label;
    string alias_reg;

    bool is_array = ASTNode_has_attr(decl, "array");

    std::vector<int> dim_sizes; // 数组的声明顺序

    std::string lval_type;

    if (is_array) {
        lval_type = get_array_decl_type(decl);
    } else {
        const char* decl_type;
        bool hasType = ASTNode_get_attr_str(decl, "type", &decl_type);
        assert(hasType);
        lval_type = decl_type;
    }

    if (is_array) {
        // 在数组的情况下，先计算数组每一维度的大小
        // TODO: 考虑缓存到 ASTNode 中，避免重复计算

        When(decl, {
            TagMatch<void>("ParamDecl", [&]() {
                QueryResult *dims = ASTNode_querySelector(decl, "/Dimension/Exp/Number"), *cur; // 必须确保所有数组大小被计算好了

                DL_FOREACH(dims, cur) {
                    int size;
                    ASTNode_get_attr_int(cur->node, "value", &size);
                    // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
                    dim_sizes.push_back(size);
                }
            }),
            TagMatch<void>("Var", [&]() {
                QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize/Dimension/Exp/Number"), *cur; // 必须确保所有数组大小被计算好了

                DL_FOREACH(dims, cur) {
                    int size;
                    ASTNode_get_attr_int(cur->node, "value", &size);
                    // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
                    dim_sizes.push_back(size);
                }
            }),
            TagMatch<void>("Const", [&]() {
                QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize/Dimension/Exp/Number"), *cur; // 必须确保所有数组大小被计算好了

                DL_FOREACH(dims, cur) {
                    int size;
                    ASTNode_get_attr_int(cur->node, "value", &size);
                    // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
                    dim_sizes.push_back(size);
                }
            })
        });

        // TODO: 这里需要根据元素类型来确定大小，这里暂时先用机器字长代替，在32位机是正确的
        dim_sizes.push_back(adapter->getWordSize()); // 最后一个地址是元素大小

        for (auto i = dim_sizes.size() - 1; i > 0; i--) {
            dim_sizes[i - 1] = dim_sizes[i - 1] * dim_sizes[i];
        }

        // 参数声明中的数组第一维大小是缺失的所以不用删除第一个了
        if (!ASTNode_id_is(decl, "ParamDecl")) {
            dim_sizes.erase(dim_sizes.begin());
        }

        // 获取访问的索引
        int access_dim_size = 0;

        auto *locator = ASTNode_querySelectorOne(address, "/Locator");
        // 没有访问数组的索引，那么直接返回地址
        if (locator) {
            auto locator_size = ASTNode_children_size(locator); // subarray 运算符的数量
            // 访问数组 依次计算索引，这里翻译为一个连加，因为我们可以控制过程，所以优化一下
            QueryResult *locators = ASTNode_querySelector(locator, "/Dimension/*"), *cur;
            int idx = 0;
            DL_FOREACH(locators, cur) {

                // 每经过一个纬度，那么它的类型就会减少一个维度
                lval_type = deref_array(lval_type);

                if (ASTNode_id_is(cur->node, "Number")) {
                    // TODO: 判断Number的类型
                    // 当前索引是数组的常量索引，在编译期完成计算
                    int value;
                    ASTNode_get_attr_int(cur->node, "value", &value);
                    adapter->loadImmediate(accumulatorReg, value * dim_sizes[idx]);
                } else {
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
                }
                if (idx != 0) {
                    // 如果不是第一个维度，先加上前面的维度大小
                    adapter->popStack({tempReg});
                    adapter->add(accumulatorReg, accumulatorReg, tempReg);
                }

                // Update 这里按照实际的subarray数量来计算防止多push了
                if (idx != locator_size - 1) {
                    adapter->pushStack({accumulatorReg}); // 如果后面还有维度，先保存
                }

                idx++;
                access_dim_size++;
            }
        } else {
            // 没有访问数组的索引，那么直接返回地址 相当于 base_addr + 0
            adapter->loadImmediate(accumulatorReg, 0);
        }
        // 访问的维度必须小于等于数组的维度
        assert(access_dim_size <= dim_sizes.size());

    }

    lval_type = "L" + lval_type;

    ASTNode_add_attr_str(lval, "type", lval_type.c_str());

    bool hasLabel = ASTNode_get_attr_str(decl, "label", &label);
    bool hasAlias = ASTNode_get_attr_str(decl, "alias", alias_reg);

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
        // Update: Fix in ARMAdapter

        if (hasAlias) {
            // The Value was place in NEON Register
            return alias_reg;
        } else {
            // The Normal Process
            if (is_array) {
                adapter->add(tempReg, adapter->getFramePointerName(), offset); // 先计算基址
                if (ASTNode_id_is(decl, "ParamDecl")) {
                    // 参数数组实际是作为数组二级指针传递的
                    // 先做一次加载
                    adapter->loadRegister(tempReg, tempReg, 0);
                }
                adapter->add(accumulatorReg, tempReg, accumulatorReg); // 然后确定实际地址
            } else {
                adapter->add(accumulatorReg, adapter->getFramePointerName(), offset);
            }
        }
    }
    return "";
}

void StackTranslator::translateAssign(ASTNode *assign) {
    assert(ASTNode_id_is(assign, "Assign"));

    auto lval = ASTNode_querySelectorOne(assign, "/Dest/Address");
    auto exp = ASTNode_querySelectorOne(assign, "Exp");

    assert(lval && exp);

    translateExp(exp);

    translateTypePush(exp);

    auto ret = translateLVal(lval);

    const char* lval_type_str, *exp_type_str;
    bool hasLValType = ASTNode_get_attr_str(lval, "type", &lval_type_str);
    bool hasExpType = ASTNode_get_attr_str(exp, "type", &exp_type_str);
    assert(hasLValType && hasExpType);

    std::string lval_type = lval_type_str,
                exp_type = exp_type_str;

    // 这里必须是基本类型的左值
    assert(is_primitive_type(deref_lval(lval_type)));
    assert(is_primitive_type(exp_type));

    adapter->mov(tempReg, accumulatorReg);

    // 地址：tempReg, 值：accumulatorReg || floatAccumulatorReg
    translateTypePop(exp);
    
    if (deref_lval(lval_type) != exp_type) {
        translateTypeConversion(exp, deref_lval(lval_type));
    }

    if (deref_lval(lval_type) == SyInt) {
        if (!ret.empty()) {
            adapter->fmov(ret, accumulatorReg);
        } else {
            adapter->storeRegister(accumulatorReg, tempReg, 0);
        }
    } else if (deref_lval(lval_type) == SyFloat) {
        if (!ret.empty()) {
            adapter->fmov(ret, floatAccumulatorReg);
        } else {
            adapter->fstoreRegister(floatAccumulatorReg, tempReg, 0);
        }
    } else {
        assert(0);
    }
}

void StackTranslator::translateReturn(ASTNode *ret) {
    assert(ASTNode_id_is(ret, "Return"));

    auto func = ASTNode_querySelectorOne(ret, "/ancestor::Function");
    string ret_type;
    ASTNode_get_attr_str(func, "return", ret_type);

    auto exp = ASTNode_querySelectorOne(ret, "Exp");
    assert((ret_type == SyVoid && exp == nullptr )|| (ret_type != SyVoid && exp != nullptr));

    if (exp) {
        translateExp(exp);
        string exp_type;
        ASTNode_get_attr_str(exp, "type", exp_type);

        if (ret_type != exp_type) {
            translateTypeConversion(exp, ret_type);
        }
    }

    // 直接返回不做转跳了，应该没有什么问题
    adapter->sub(adapter->getStackPointerName(), adapter->getFramePointerName(), 4);
    // 这里直接弹出到 pc 寄存器中实现转跳
    adapter->popStack({adapter->getFramePointerName(), adapter->getPCName()});

}

void StackTranslator::translateVarDecl(ASTNode* var_decl) {
    assert(ASTNode_id_is(var_decl, "Var"));

    const char *name;
    bool hasName = ASTNode_get_attr_str(var_decl, "name", &name);
    assert(hasName);
    // 先找到Decl中变量的声明
    auto decl_entity = ASTNode_querySelectorfOne(var_decl, "ancestor::Scope/Decl/Var[@name='%s']", name);
    assert(decl_entity);

    std::string alias_reg;
    bool hasAliasReg = ASTNode_get_attr_str(decl_entity, "alias", alias_reg);

    std::string var_type;
    bool hasVarType = ASTNode_get_attr_str(decl_entity, "type", var_type);
    assert(hasVarType);

    int offset;
    bool hasOffset = ASTNode_get_attr_int(decl_entity, "offset", &offset);
    assert(hasOffset);

    if (ASTNode_has_attr(decl_entity, "array")) {
        // 数组
        int size;
        bool hasSize = ASTNode_get_attr_int(decl_entity, "size", &size);
        assert(hasSize);

        bool cleared = false;
        // 首先调用 memset 初始化数组, 这里使用 memset 的阈值是 8
        if (size >= 8) {
            adapter->loadImmediate(adapter->getRegName(2), size * adapter->getWordSize());
            adapter->loadImmediate(adapter->getRegName(1), 0);
            adapter->add(adapter->getRegName(0), adapter->getFramePointerName(), offset);
            adapter->call("memset");
            cleared = true;
        }
        QueryResult *inits = ASTNode_querySelector(decl_entity, "InitValue/*"), *cur;
        if (inits == nullptr) return;

        int idx = 0;
        DL_FOREACH(inits, cur) {
            auto init = cur->node;
            int value, repeat;
            const char* value_type_str;
            std::string value_type;

            bool hasValue = ASTNode_get_attr_int(init, "value", &value);
            bool hasRepeat = ASTNode_get_attr_int(init, "repeat", &repeat);
            assert(hasRepeat);

            if (ASTNode_id_is(init, "Exp")) {
                translateExp(init);
                bool hasType = ASTNode_get_attr_str(init, "type", &value_type_str);
                assert(hasType);
                value_type = value_type_str;
            } else {
                // 常量的情况下，直接加载
                assert(hasValue);
                // 如果是 0，那么判断数组是不是已经使用 memset 初始化过了
                if (value != 0 || !cleared) {
                    translateExpInner(init);
                }
                bool hasType = ASTNode_get_attr_str(init, "type", &value_type_str);
                assert(hasType);
                value_type = value_type_str;
            }

            if (value_type != var_type) {
                // 类型不同，需要转换
                translateTypeConversion(init, var_type);
            }

            if (!hasValue || (hasValue && value != 0) || !cleared) {
                // 如果不是 0 或者没有清空过，那么就赋值
                for (int i = 0; i < repeat; i++) {
                    if (var_type == SyInt) {
                        adapter->storeRegister(accumulatorReg, adapter->getFramePointerName(),  offset + idx * adapter->getWordSize());
                    } else if (var_type == SyFloat) {
                        adapter->fstoreRegister(floatAccumulatorReg, adapter->getFramePointerName(), offset + idx * adapter->getWordSize());
                    } else {
                        assert(0);
                    }
                    idx++;
                }
            } else {
                idx += repeat;
            }
        }
        assert(idx == size);
    } else {
        // 单个变量
        // 变量初始化表达式
        auto init_exp = ASTNode_querySelectorOne(decl_entity, "InitValue/Exp");
        // 在文法里已经确定过了，必定有初始化表达式
        if (init_exp == nullptr) return;

        translateExp(init_exp);

        const char* init_type_str;
        bool hasInitType = ASTNode_get_attr_str(init_exp, "type", &init_type_str);
        assert(hasInitType);
        std::string init_type = init_type_str;

        assert(init_type != SyVoid);

        if (init_type != var_type) {
            // 类型不同，需要转换
            translateTypeConversion(init_exp, var_type);
        }

        if (var_type == SyInt) {
            if (hasAliasReg) {
                adapter->fmov(alias_reg, accumulatorReg);
            } else {
                adapter->storeRegister(accumulatorReg, adapter->getFramePointerName(), offset);
            }
        } else if (var_type == SyFloat) {
            if (hasAliasReg) {
                adapter->fmov(alias_reg, floatAccumulatorReg);
            } else {
                adapter->fstoreRegister(floatAccumulatorReg, adapter->getFramePointerName(), offset);
            }
        } else {
            assert(0);
        }
    }
}

void StackTranslator::translateIf(ASTNode *ifstmt) {
    assert(ASTNode_id_is(ifstmt, "If"));

    adapter->emitComment("If 开始");

    // 生成真分支和假分支标签
    // TODO： 也许可以研究一下分支优化的策略，生成更好的代码

    auto true_label = generateLabel(),
        false_label = generateLabel(),
        end_label = generateLabel();

    ASTNode_add_attr_str(ifstmt, "trueLabel", true_label.c_str());
    ASTNode_add_attr_str(ifstmt, "falseLabel", false_label.c_str());

    auto cond = ASTNode_querySelectorOne(ifstmt, "Cond/*");
    assert(cond);
    translateExp(cond);

    // 添加条件跳转作为单变量情况判断的 handle
    const char* cond_type_str;
    std::string cond_type;
    bool hasType = ASTNode_get_attr_str(cond, "type", &cond_type_str);
    assert(hasType);
    cond_type = cond_type_str;

    assert(cond_type != SyVoid);
    if (cond_type == SyInt) adapter->jumpEqual(accumulatorReg, 0, false_label);
    if (cond_type == SyFloat) {
        adapter->fjumpEqual(floatAccumulatorReg, 0.0f, false_label);
        
    }
    adapter->emitLabel(true_label);
    auto true_branch = ASTNode_querySelectorOne(ifstmt, "Then/*");
    assert(true_branch);
    translateStmt(true_branch);
    adapter->jump(end_label);


    adapter->emitLabel(false_label);
    auto false_branch = ASTNode_querySelectorOne(ifstmt, "Else/*");
    if (false_branch) {
        translateStmt(false_branch);
    }
    adapter->emitLabel(end_label);
}

void StackTranslator::translateUnaryOp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "UnPlus") || ASTNode_id_is(exp, "UnMinus") || ASTNode_id_is(exp, "Not"));

    if (ASTNode_id_is(exp, "UnPlus")) {
        auto inner = ASTNode_querySelectorOne(exp, "*");
        translateExpInner(inner);
        passType(exp, inner);
    } else if (ASTNode_id_is(exp, "UnMinus")) {
        auto inner = ASTNode_querySelectorOne(exp, "*");
        translateExpInner(inner);

        std::string inner_type;
        bool hasType = ASTNode_get_attr_str(inner, "type", inner_type);
        assert(hasType);

        if (inner_type == SyInt) {
            adapter->neg(accumulatorReg, accumulatorReg);
        } else if (inner_type == SyFloat) {
            adapter->fneg(floatAccumulatorReg, floatAccumulatorReg);
        } else {
            assert(0);
        }
        passType(exp, inner);
    } else if (ASTNode_id_is(exp, "Not")) {
        auto inner = ASTNode_querySelectorOne(exp, "*");
        translateExpInner(inner);

        std::string inner_type;
        bool hasType = ASTNode_get_attr_str(inner, "type", inner_type);
        assert(hasType);

        if (inner_type == SyInt) {
            adapter->notReg(accumulatorReg, accumulatorReg);
        } else if (inner_type == SyFloat) {
            adapter->fnotReg(accumulatorReg, floatAccumulatorReg);
        } else {
            assert(0);
        }
        // Sysy 只规定了非0为真，因此无论是浮点还是整数，取反后都是整数
        ASTNode_add_attr_str(exp, "type", SyInt);
    } else {
        assert(0);
    }
}

void StackTranslator::translateRelOp(ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Equal") || ASTNode_id_is(exp, "NotEq") || ASTNode_id_is(exp, "Less") || ASTNode_id_is(exp, "LessEq") || ASTNode_id_is(exp, "Greater") || ASTNode_id_is(exp, "GreaterEq"));

    ASTNode *lhs = ASTNode_querySelectorOne(exp, "*[0]"),
            *rhs = ASTNode_querySelectorOne(exp, "*[1]");

    assert(lhs != nullptr && rhs != nullptr);

    std::string rhs_type, lhs_type, cmp_type;

    translateExpInner(lhs);
    translateTypePush(lhs);
    bool lhs_has_type = ASTNode_get_attr_str(lhs, "type", lhs_type);

    translateExpInner(rhs);
    bool rhs_has_type = ASTNode_get_attr_str(rhs, "type", rhs_type);


    assert(rhs_has_type && lhs_has_type);
    assert(lhs_type != SyVoid && rhs_type != SyVoid);

    if ((lhs_type == SyFloat) ^ (rhs_type == SyFloat)) {
        cmp_type = SyFloat;
        // 仅有一边为浮点的情况需要转换
        if (lhs_type == SyFloat) {
            // 转换右边为浮点数
            translateTypeConversion(rhs, SyFloat); // s0 <- r0
            adapter->fpopStack({floatTempReg}); // s1 <- lhs
        }

        if (rhs_type == SyFloat) {
            // 转换左边为浮点数
            adapter->fpopStack({floatTempReg});
            adapter->i2f(floatTempReg, floatTempReg);
        }
    } else if (lhs_type == SyFloat && rhs_type != SyFloat) {
        cmp_type = SyFloat;
        adapter->fpopStack({floatTempReg});
    } else {
        cmp_type = SyInt;
        adapter->popStack({tempReg});
    }

    ASTNode_add_attr_str(exp, "type", SyInt); // 做完比较后的结果是整数

    if (ASTNode_id_is(exp, "Equal")) {
        if (cmp_type == SyInt) adapter->cmpEqual(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpEqual(accumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "NotEq")) {
        if (cmp_type == SyInt) adapter->cmpNotEqual(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpNotEqual(accumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "Less")) {
        if (cmp_type == SyInt) adapter->cmpLess(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpLess(accumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "LessEq")) {
        if (cmp_type == SyInt) adapter->cmpLessEqual(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpLessEqual(accumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "Greater")) {
        if (cmp_type == SyInt) adapter->cmpGreater(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpGreater(accumulatorReg, floatTempReg, floatAccumulatorReg);
    } else if (ASTNode_id_is(exp, "GreaterEq")) {
        if (cmp_type == SyInt) adapter->cmpGreaterEqual(accumulatorReg, tempReg, accumulatorReg);
        if (cmp_type == SyFloat) adapter->fcmpGreaterEqual(accumulatorReg, floatTempReg, floatAccumulatorReg);
    }
}

void StackTranslator::translateShortCircuitLogicOp(ASTNode *logic) {
    assert(ASTNode_id_is(logic, "Or") || ASTNode_id_is(logic, "And"));

    // 在父节点中寻找真假分支的标签
    const char *true_label, *false_label;
    auto parent = ASTNode_querySelectorOne(logic, "ancestor::*[@trueLabel,@falseLabel]");
    assert(parent);
    ASTNode_get_attr_str(parent, "trueLabel", &true_label);
    ASTNode_get_attr_str(parent, "falseLabel", &false_label);

    auto rhs_label = generateLabel();

    if (ASTNode_id_is(logic, "And")) {
        // 短路与

        // 如果 lhs 为假，可以确定整个表达式的值为假，那么直接跳转到父节点的 false_label
        ASTNode_add_attr_str(logic, "falseLabel", false_label);

        // 如果 lhs 为真，那么继续计算 rhs 的结果
        ASTNode_add_attr_str(logic, "trueLabel", rhs_label.c_str());
    } else if (ASTNode_id_is(logic, "Or")) {
        // 短路或

        // 如果 lhs 为真，可以确定整个表达式的值为真，那么直接跳转到父节点的 true_label
        ASTNode_add_attr_str(logic, "trueLabel", true_label);

        // 如果 lhs 为假，那么继续计算 rhs 的结果
        ASTNode_add_attr_str(logic, "falseLabel", rhs_label.c_str());
    } else {
        assert(false);
    }

    // Update: 统一按照 SyInt 处理
    ASTNode_add_attr_str(logic, "type", SyInt);

    auto lhs = ASTNode_querySelectorOne(logic, "*[0]"),
        rhs = ASTNode_querySelectorOne(logic, "*[1]");

    assert(lhs && rhs);

    translateExpInner(lhs);

    const char* lhs_type;
    bool hasLhsType = ASTNode_get_attr_str(lhs, "type", &lhs_type);
    assert(hasLhsType);

    if (ASTNode_id_is(logic, "And")) {
        // 短路与
        adapter->jumpEqual(accumulatorReg, 0, false_label);
    } else if (ASTNode_id_is(logic, "Or")) {
        // 短路或
        adapter->jumpNotEqual(accumulatorReg, 0, true_label);
    } else {
        assert(false);
    }


    if (ASTNode_id_is(logic, "And")) {
        // 短路与
        // 计算 rhs 时，如果为真，直接转跳父节点的true_label
        ASTNode_set_attr_str(logic, "trueLabel", true_label);
    } else if (ASTNode_id_is(logic, "Or")) {
        // 短路或
        // 计算 rhs 时，如果为假，转跳父节点的falseLabel
        ASTNode_set_attr_str(logic, "falseLabel", false_label);
    } else {
        assert(false);
    }

    adapter->emitLabel(rhs_label);
    translateExpInner(rhs);

    const char* rhs_type;
    bool hasRhsType = ASTNode_get_attr_str(rhs, "type", &rhs_type);
    assert(hasRhsType);

    assert(strcmp(rhs_type, "Int") == 0);

    // 计算到这里，说明左边无法得到整个表达式的值，根据右边的值进行转跳
    adapter->jumpEqual(accumulatorReg, 0, false_label, true_label);
}

void StackTranslator::translateWhile(ASTNode *whilestmt) {
    assert(ASTNode_id_is(whilestmt, "While"));

    auto begin_label = generateLabel(),
        true_label = generateLabel(),
        end_label = generateLabel();

    ASTNode_add_attr_str(whilestmt, "trueLabel", true_label.c_str());
    ASTNode_add_attr_str(whilestmt, "falseLabel", end_label.c_str());
    ASTNode_add_attr_str(whilestmt, "beginLabel", begin_label.c_str());

    adapter->emitLabel(begin_label);

    auto cond = ASTNode_querySelectorOne(whilestmt, "Cond/*");
    assert(cond);
    translateExp(cond);

    const char* cond_type;
    bool hasType = ASTNode_get_attr_str(cond, "type", &cond_type);
    assert(hasType);

    if (strcmp(cond_type, "Int") == 0) adapter->jumpEqual(accumulatorReg, 0, end_label);

    adapter->emitLabel(true_label);

    auto body = ASTNode_querySelectorOne(whilestmt, "Stmt/*");

    translateStmt(body);

    adapter->jump(begin_label);

    adapter->emitLabel(end_label);
}

void StackTranslator::translateBreak(ASTNode *brk) {
    assert(ASTNode_id_is(brk, "Break"));

    auto parent = ASTNode_querySelectorOne(brk, "ancestor::While");
    assert(parent);

    const char* end_label;
    bool hasEndLabel = ASTNode_get_attr_str(parent, "falseLabel", &end_label);
    assert(hasEndLabel);

    adapter->jump(end_label);
}

void StackTranslator::translateContinue(ASTNode *cont) {
    assert(ASTNode_id_is(cont, "Continue"));

    auto parent = ASTNode_querySelectorOne(cont, "ancestor::While");
    assert(parent);

    const char* begin_label;
    bool hasBeginLabel = ASTNode_get_attr_str(parent, "beginLabel", &begin_label);
    assert(hasBeginLabel);

    adapter->jump(begin_label);
}

void StackTranslator::passType(ASTNode *cur, const ASTNode *child, const char *to_attr, const char *from_attr) {
    const char *type;
    bool hasInner = ASTNode_get_attr_str(child, from_attr, &type);
    assert(hasInner);
    if (hasInner) {
        ASTNode_add_attr_str(cur, to_attr, type);
    }
}

void StackTranslator::translateTypeConversion(ASTNode* exp, std::string target_type) {
    assert(exp);
    
    const char* type;
    bool hasType = ASTNode_get_attr_str(exp, "type", &type);
    assert(hasType);

    std::string cur_type = type;

    if (cur_type == target_type) return;

    if (cur_type == SyInt && target_type == SyFloat) {
        // 整数转浮点数
        adapter->fmov(floatAccumulatorReg, accumulatorReg);
        adapter->i2f(floatAccumulatorReg, floatAccumulatorReg);
    } else if (cur_type == SyFloat && target_type == SyInt) {
        // 浮点数转整数
        adapter->f2i(floatAccumulatorReg, floatAccumulatorReg);
        adapter->fmov(accumulatorReg, floatAccumulatorReg);
    } else {
        assert(false);
    }
    ASTNode_set_attr_str(exp, "type", target_type.c_str());
    ASTNode_add_attr_str(exp, "converted", "true");
}

void StackTranslator::translateTypePush(ASTNode* exp) {
    assert(exp);
    
    const char* type;
    bool hasType = ASTNode_get_attr_str(exp, "type", &type);
    assert(hasType);

    std::string cur_type = type;

    assert(cur_type != SyVoid);
    if (cur_type == SyInt || is_array_type(cur_type)) {
        // 数组（LVal）地址被放在accumulatorReg中
        adapter->pushStack({accumulatorReg});
    } else if (cur_type == SyFloat) {
        adapter->fpushStack({floatAccumulatorReg});
    } else {
        assert(false);
    }
}

void StackTranslator::translateTypePop(ASTNode* exp) {
    assert(exp);
    
    const char* type;
    bool hasType = ASTNode_get_attr_str(exp, "type", &type);
    assert(hasType);

    std::string cur_type = type;

    assert(cur_type != SyVoid);

    if (cur_type == SyInt) {
        adapter->popStack({accumulatorReg});
    } else if (cur_type == SyFloat) {
        adapter->fpopStack({floatAccumulatorReg});
    } else {
        assert(false);
    }
}

void StackTranslator::insertLiteralPool() {
    adapter->createLocalLTPool();
}
