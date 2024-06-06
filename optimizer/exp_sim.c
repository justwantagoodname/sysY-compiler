#include "sysY.h"
#include "ast.h"

typedef double sim_result_t;

bool ExpNode_is_atomic(const ASTNode *node)
{
    assert(node != NULL);

    return ASTNode_id_is(node, "Number") || ASTNode_has_attr(node, "value");
}

const char *op_literal[] = {"Plus", "Minus", "Mult",
                            "Div", "Mod", "And",
                            "Or", "Equal", "NotEq",
                            "Less", "LessEq", "Greater",
                            "GreaterEq"};

sim_result_t ExpNode_op_calc(const char *op, double left, double right)
{
    assert(op != NULL);
    for (int i = 0; i < sizeof(op_literal) / sizeof(op_literal[0]); i++)
    {
        if (strcmp(op, op_literal[i]) == 0)
        {
            switch (i)
            {
            case 0:
                return left + right;
            case 1:
                return left - right;
            case 2:
                return left * right;
            case 3:
                return left / right;
            case 4:
                return (int)left % (int)right;
            case 5:
                return left && right;
            case 6:
                return left || right;
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
    return 0; // Error
}
ASTNode* ExpNode_fetch_const(const ASTNode* node) {
    assert(node != NULL);
    assert(ASTNode_id_is(node, "Fetch"));

    ASTNode* base_node = ASTNode_querySelectorOne(node, "//*[@base][0]");
    const char* base_name = NULL;    
    ASTNode_get_attr_str(base_node, "base", &base_name);
    
    assert(base_name != NULL);

    ASTNode* target = ASTNode_querySelectorfOne(node, "/ancestor::Scope//Const[@name='%s'][0]", base_name);

    assert(target != NULL);
    assert(ASTNode_id_is(target, "Const"));

    ASTNode* value = ASTNode_querySelectorOne(target, "//Exp/Number");
    ASTNode_print(value);

    assert(ASTNode_id_is(value, "Number"));

    return ASTNode_clone(value);
}

ASTNode *ExpNode_simplify_binary_operater(const ASTNode *exp);
ASTNode *ExpNode_simplify_unary_operater(const ASTNode *exp);
ASTNode *ExpNode_simplify_recursive(const ASTNode *node)
{
    assert(node != NULL);

    int child = ASTNode_children_size(node);
    ASTNode *ret = NULL;

    printf("Sim: %s with %d\n", node->id, child);
    switch (child)
    {
    case 1:
    {
        ret = ExpNode_simplify_unary_operater(node);
        break;
    }
    case 2:
    {
        ret = ExpNode_simplify_binary_operater(node);
        break;
    }
    default:
    {
        // If we can't handle the expression, just clone it
        ret = ASTNode_clone(node);
        break;
    }
    }

    assert(ret != NULL);
    return ret;
}

ASTNode *ExpNode_simplify_unary_operater(const ASTNode *exp)
{
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
        return ExpNode_fetch_const(exp);
    }

postcondition:
    assert(ret != NULL);
    return ret;
}

ASTNode *ExpNode_simplify_binary_operater(const ASTNode *exp)
{
    assert(ASTNode_children_size(exp) <= 2);

    if (ExpNode_is_atomic(exp))
        return ASTNode_clone(exp);

    const ASTNode *left = ASTNode_querySelectorOne(exp, "/*[0]");
    const ASTNode *right = ASTNode_querySelectorOne(exp, "/*[1]");

    ASTNode *sim_left = ExpNode_simplify_recursive(left);
    ASTNode *sim_right = ExpNode_simplify_recursive(right);

    ASTNode_print(sim_left);
    ASTNode_print(sim_right);
    bool left_atomic = ExpNode_is_atomic(sim_left),
         right_atomic = ExpNode_is_atomic(sim_right);

    /* Obvisually, we can only handle float this time */
    int left_value = -1, right_value = -1;

    ASTNode *ret = NULL;
    if (left_atomic && right_atomic)
    {

        ASTNode_get_attr_int(sim_left, "value", &left_value);
        ASTNode_get_attr_int(sim_right, "value", &right_value);

        ASTNode_free(sim_left);
        ASTNode_free(sim_right);

        sim_result_t sim_val = ExpNode_op_calc(exp->id, left_value, right_value);
        ret = ASTNode_create("Number");
        ASTNode_add_attr_int(ret, "value", (int)sim_val);
        printf("Sim A ConstExp with %d %s %d = %lf\n",
               left_value, exp->id, right_value, sim_val);
    }

    assert(ret != NULL);
    return ret;
}

/**
 * Simplify the expression node
 * @note This function won't change the orignal node but return a new one
 */
ASTNode *ExpNode_simplify(const ASTNode *exp)
{
    assert(exp != NULL);
    assert(ASTNode_id_is(exp, "Exp"));

    ASTNode *child = ASTNode_querySelectorOne(exp, "/*");

    ASTNode *simplified_child = ExpNode_simplify_recursive(child);

    ASTNode *new_exp = ASTNode_create("Exp");
    ASTNode_add_child(new_exp, simplified_child);
    return new_exp;
}