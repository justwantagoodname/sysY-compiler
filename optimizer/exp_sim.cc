#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"

#define EXP_LOG printf
//#define EXP_LOG(...) (0)

typedef double sim_result_t;

bool ExpNode_is_atomic(const ASTNode *node) {
    assert(node != nullptr);

    return ASTNode_id_is(node, "Number") || ASTNode_has_attr(node, "value");
}

const char *op_literal[] = {"Plus", "Minus", "Mult",
                            "Div", "Mod", "And",
                            "Or", "Equal", "NotEq",
                            "Less", "LessEq", "Greater",
                            "GreaterEq"};

sim_result_t ExpNode_op_calc(const char *op, sim_result_t left, sim_result_t right) {
    assert(op != nullptr);
    for (int i = 0; i < sizeof(op_literal) / sizeof(op_literal[0]); i++) {
        if (strcmp(op, op_literal[i]) == 0) {
            switch (i) {
                case 0:
                    return left + right;
                case 1:
                    return left - right;
                case 2:
                    return left * right;
                case 3:
                    return left / right;
                case 4:
                    return (int) left % (int) right;
                case 5:
                    return (int) left && (int) right;
                case 6:
                    return (int) left || (int) right;
                case 7:
                    return left == right;
                case 8:
                    return left != right;
                case 9:
                    return left < right;
                case 10:
                    return left <= right;
                case 11:
                    return left > right;
                case 12:
                    return left >= right;
            }
        }
    }

    assert(false);
    return 0; // Error
}

/**
 * 判读一个二元表达式是否是部分计算的，即其中一个子表达式是字面量，如果都不是返回NULL，否则返回字面量的子表达式
 * @param node
 * @return
 */
ASTNode* ExpNode_is_partial(ASTNode *node, bool& is_left) {
    assert(node != nullptr);
    assert(ASTNode_children_size(node) == 2);

    ASTNode *left = ASTNode_querySelectorOne(node, "/*[0]");
    ASTNode *right = ASTNode_querySelectorOne(node, "/*[1]");

    if (ExpNode_is_atomic(left)) {
        is_left = true;
        return left;
    } else if (ExpNode_is_atomic(right)) {
        is_left = false;
        return right;
    } else {
        return nullptr;
    }
}

ASTNode *ExpNode_calc_partial(const ASTNode *exp, ASTNode *sim_left, ASTNode *sim_right, bool left_atomic,
                              bool right_atomic);

ASTNode *ExpNode_fetch_const_array_value(const ASTNode *fetch, const ASTNode *target);

ASTNode *ExpNode_try_fetch_const(const ASTNode *node);

ASTNode *ExpNode_simplify_binary_operator(const ASTNode *exp);

ASTNode *ExpNode_simplify_unary_operator(const ASTNode *exp);

ASTNode *ExpNode_simplify_call_params(const ASTNode *exp);

ASTNode *ExpNode_simplify_recursive(const ASTNode *node) {
    assert(node != nullptr);

    // 一些特殊节点的处理
    if (ASTNode_id_is(node, "Call")) { //函数调用是不定长的
        return ExpNode_simplify_call_params(node);
    }

    auto child = ASTNode_children_size(node);
    ASTNode *ret = nullptr;

    EXP_LOG("Sim: %s with %zu children.\n", node->id, child);
    switch (child) {
        case 1: {
            ret = ExpNode_simplify_unary_operator(node);
            break;
        }
        case 2: {
            ret = ExpNode_simplify_binary_operator(node);
            break;
        }
        default: {
            // If we can't handle the expression, just clone it
            ret = ASTNode_clone(node);
            break;
        }
    }

    assert(ret != nullptr);
    return ret;
}

ASTNode *ExpNode_simplify_unary_operator(const ASTNode *exp) {
    assert(exp != NULL);
    assert(ASTNode_children_size(exp) == 1);
    ASTNode *ret = NULL;

    if (ExpNode_is_atomic(exp)) {
        ret = ASTNode_clone(exp);
    } else if (ASTNode_id_is(exp, "UnPlus")) {
        ASTNode *child = ASTNode_querySelectorOne(exp, "/*");
        ret = ExpNode_simplify_recursive(child);
    } else if (ASTNode_id_is(exp, "UnMinus")) {
        ASTNode *child = ASTNode_querySelectorOne(exp, "/*");
        ret = ExpNode_simplify_recursive(child);
        int value = -1;
        ASTNode_get_attr_int(ret, "value", &value);
        ASTNode_free(ret);
        ret = ASTNode_create("Number");
        ASTNode_add_attr_int(ret, "value", -value);
    } else if (ASTNode_id_is(exp, "Fetch")) {
        return ExpNode_try_fetch_const(exp);
    }

    assert(ret != nullptr);
    return ret;
}

ASTNode *ExpNode_simplify_binary_operator(const ASTNode *exp) {
    assert(ASTNode_children_size(exp) <= 2);

    if (ExpNode_is_atomic(exp))
        return ASTNode_clone(exp);

    const ASTNode *left = ASTNode_querySelectorOne(exp, "/*[0]");
    const ASTNode *right = ASTNode_querySelectorOne(exp, "/*[1]");

    ASTNode *sim_left = ExpNode_simplify_recursive(left);
    ASTNode *sim_right = ExpNode_simplify_recursive(right);

    bool left_atomic = ExpNode_is_atomic(sim_left),
            right_atomic = ExpNode_is_atomic(sim_right);

    /* Obvisually, we can only handle int this time */
    int left_value = -1, right_value = -1;

    ASTNode *ret = nullptr;
    if (left_atomic && right_atomic) {

        ASTNode_get_attr_int(sim_left, "value", &left_value);
        ASTNode_get_attr_int(sim_right, "value", &right_value);

        ASTNode_free(sim_left);
        ASTNode_free(sim_right);

        sim_result_t sim_val = ExpNode_op_calc(exp->id, left_value, right_value);
        ret = ASTNode_create("Number");
        ASTNode_add_attr_int(ret, "value", (int) sim_val);
        EXP_LOG("Sim A ConstExp with %d %s %d = %lf\n",
                left_value, exp->id, right_value, sim_val);
    } else if (left_atomic ^ right_atomic) {
        ret = ExpNode_calc_partial(exp, sim_left, sim_right, left_atomic, right_atomic);
    } else {
        // TODO: 都不是字面量，理论上可以用一些运算性质化简
        ret = ASTNode_create(exp->id);
        ASTNode_add_nchild(ret, 2, sim_left, sim_right);
    }

    assert(ret != NULL);
    return ret;
}

ASTNode *ExpNode_try_fetch_const(const ASTNode *node) {
    assert(node != NULL);
    assert(ASTNode_id_is(node, "Fetch"));

    ASTNode *base_node = ASTNode_querySelectorOne(node, "//*[@base][0]");
    const char *base_name = NULL;
    ASTNode_get_attr_str(base_node, "base", &base_name);

    assert(base_name != NULL);

    ASTNode *target = ASTNode_querySelectorfOne(node, "/ancestor::Scope//Const[@name='%s'][0]", base_name);

    if (target == NULL) {
        return ASTNode_clone(node);
    }

    assert(target != NULL);
    // can ref self check
    assert(ASTNode_id_is(target, "Const"));

    ASTNode *value = NULL;

    if (ASTNode_has_attr(target, "array") || ASTNode_querySelectorOne(node, "//Locator[0]") != NULL) {
        value = ExpNode_fetch_const_array_value(node, target);
    } else {
        value = ASTNode_querySelectorOne(target, "//Exp/Number");
        value = ASTNode_clone(value);
    }
    return value;
}

ASTNode *ExpNode_fetch_const_array_value(const ASTNode *fetch, const ASTNode *target) {
    assert(fetch != NULL && target != NULL);
    assert(ASTNode_id_is(fetch, "Fetch"));
    assert(ASTNode_id_is(target, "Const"));

    /* 化简Fetch元素的每一个层取址表达式 */
    QueryResult *iter = NULL;
    QueryResult *locator_dims = ASTNode_querySelector(fetch, "//Locator/Dimension/*");
    ASTNode *locator_sims = ASTNode_create("Locator"); // Simplified each dimension

    bool const_foldable = true;

    DL_FOREACH(locator_dims, iter) {
        ASTNode *sim_dim_exp = ExpNode_simplify_recursive(iter->node);
        ASTNode *sim_dim = ASTNode_create("Dimension");
        if (!ASTNode_id_is(sim_dim_exp, "Number")) const_foldable = false;
        ASTNode_add_child(sim_dim, sim_dim_exp);
        ASTNode_add_child(locator_sims, sim_dim);
    }

    /* 如果不是常量那就仅仅化简取地址表达式 */
    if (!const_foldable) {
        assert(locator_sims != NULL);
        return locator_sims;
    }

    /* 开始从常量数组中取值 */

    // 先检查常量数组的初始化表达式是否已经是规范形式 (一维表示)

    if (ArrayInitNode_need_flatten(target)) {
        ArrayInitNode_flatten(
                (ASTNode *) target); // 这里实际修改了AST，但是AST中的ArrayInitValue节点是一个孩子节点，所以不会影响到原AST，同样会在第二次pass中处理所以影响不大
    }

    ASTNode *number = ArrayInitNode_get_value_by_linear_index(target, locator_sims);

    assert(number != NULL);

    return number;
}

ASTNode *ExpNode_calc_partial(const ASTNode *exp, ASTNode *sim_left, ASTNode *sim_right, const bool left_atomic,
                              const bool right_atomic) {
    assert(exp != NULL);
    assert(sim_left != NULL);
    assert(sim_right != NULL);
    assert(left_atomic ^ right_atomic);

    int partial_value = -1;
    if (strcmp(exp->id, "Or") == 0) {
        if (left_atomic) {
            ASTNode_get_attr_int(sim_left, "value", &partial_value);
            if (partial_value) {
                return sim_left; // true || a => true
            } else {
                return sim_right; // false || a => a
            }
        } else {
            ASTNode_get_attr_int(sim_right, "value", &partial_value);
            if (partial_value) {
                return sim_right;
            } else {
                return sim_left;
            }

        }
    } else if (strcmp(exp->id, "And") == 0) {
        if (left_atomic) {
            ASTNode_get_attr_int(sim_left, "value", &partial_value);
            if (partial_value) {
                return sim_right; // true && a => a
            } else {
                return sim_left; // false && a => false
            }
        } else {
            ASTNode_get_attr_int(sim_right, "value", &partial_value);
            if (partial_value) {
                return sim_left;
            } else {
                return sim_right;
            }
        }
        // TODO: 可以继续添加一些简化规则例如子节点和父节点的关系 Add -> Mult
    } else if (strcmp(exp->id, "Plus") == 0 || strcmp(exp->id, "Mult") == 0) { // a + (b + c) => (a + b) + c
        ASTNode *non_atomic = left_atomic ? sim_right : sim_left;
        ASTNode *atomic = left_atomic ? sim_left : sim_right;

        if (strcmp(non_atomic->id, exp->id) != 0) goto clone_new;

        bool is_left = false; // 子节点的哪边是常量
        ASTNode *const_of_non_atomic = ExpNode_is_partial(non_atomic, is_left); // 非原子节点的常量子节点

        if (const_of_non_atomic != nullptr) {

            // 将非原子节点的常量子节点和原子节点相运算

            int non_atomic_value = -1, atomic_value = -1;
            ASTNode_get_attr_int(const_of_non_atomic, "value", &non_atomic_value);
            ASTNode_get_attr_int(atomic, "value", &atomic_value);

            ASTAttribute* value = ASTNode_get_attr_or_null(atomic, "value");
            assert(value != nullptr);
            value->value.int_value = (int) ExpNode_op_calc(exp->id, non_atomic_value, atomic_value); // TODO: Change to full double

            ASTNode *non_atomic_desc = ASTNode_querySelectorfOne(non_atomic, "/*[%d]", !is_left ? 0 : 1);

            ASTNode *new_sim_side = ASTNode_clone(non_atomic_desc);
            ASTNode *ret = ASTNode_create(exp->id);
            ASTNode_add_nchild(ret, 2, new_sim_side, atomic);

            ASTNode_free(non_atomic);
            return ret;

        } else goto clone_new; // can't handle
    }


    clone_new:
        ASTNode *ret = ASTNode_create(exp->id);
        ASTNode_add_nchild(ret, 2, sim_left, sim_right);
        return ret;
}

ASTNode *ExpNode_simplify_call_params(const ASTNode *exp) {
    assert(ASTNode_id_is(exp, "Call"));

    const char* func_name = nullptr;
    ASTNode_get_attr_str(exp, "name", &func_name);
    auto ret = ASTNode_create_attr(exp->id, 1, "name", func_name);

    QueryResult *queryResult = ASTNode_querySelector(exp, "Param"), *cur = NULL;

    DL_FOREACH(queryResult, cur) {
        if (ASTNode_attr_eq_str(cur->node, "type", "StringConst")) {
            ASTNode_add_child(ret, ASTNode_clone(cur->node));
        } else if (ASTNode_attr_eq_str(cur->node, "type", "Exp")) {
            auto param_node = ASTNode_create("Param");
            auto param_exp = ASTNode_querySelectorOne(cur->node, "/*"); // 化简参数
            ASTNode_add_child(param_node, ExpNode_simplify_recursive(param_exp));
            ASTNode_add_child(ret, param_node);
        }
    }

    assert(ASTNode_children_size(ret) == ASTNode_children_size(exp));
    return ret;
}

/**
 * Simplify the expression node
 * @note This function won't change the orignal node but return a new one 
 *       **注意这个函数可能会修改原AST的初始化列表**
 */
ASTNode *ExpNode_simplify(const ASTNode *exp) {
    assert(exp != NULL);
    assert(ASTNode_id_is(exp, "Exp"));
    assert(ASTNode_children_size(exp) == 1);

    ASTNode *child = ASTNode_querySelectorOne(exp, "/*[0]"); // 获取内部第一个节点，实际上应该只有一个

    ASTNode *simplified_child = ExpNode_simplify_recursive(child);

    ASTNode *new_exp = ASTNode_create("Exp");
    ASTNode_add_child(new_exp, simplified_child);
    return new_exp;
}