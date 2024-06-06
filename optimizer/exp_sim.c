#include "sysY.h"
#include "ast.h"

typedef double sim_result_t;

bool ExpNode_is_atomic(ASTNode* node) {
    assert(node != NULL);
    return strcmp(node->id, "Number") == 0;
}

const char* op_literal[] = {"Plus", "Minus", "Mult", "Div", "Mod", "And", "Or"};

sim_result_t ExpNode_op_calc(const char* op, double left, double right) {
    assert(op != NULL);
    for (int i = 0; i < sizeof(op_literal) / sizeof(op_literal[0]); i++) {
        if (strcmp(op, op_literal[i]) == 0) {
            switch (i) {
                case 0: return left + right;
                case 1: return left - right;
                case 2: return left * right;
                case 3: return left / right;
                case 4: return (int)left % (int)right;
                case 5: return left && right;
                case 6: return left || right;
            }
        }
    }
}

ASTNode* ExpNode_simplify_impl(ASTNode* exp) {
    assert(exp != NULL);
    assert(ASTNode_children_size(exp) <= 2);
    
    if (ExpNode_is_atomic(exp)) return ASTNode_clone(exp);
    
    ASTNode* left = ASTNode_querySelectorOne(exp, "/*[0]");
    ASTNode* right = ASTNode_querySelectorOne(exp, "/*[1]");
    
    left = ExpNode_simplify_impl(left);
    right = ExpNode_simplify_impl(right);

    bool left_atomic = ExpNode_is_atomic(left),
         right_atomic = ExpNode_is_atomic(right);
    int left_value, right_value;

    if (left_atomic && right_atomic) {
        ASTNode_get_attr_int(left, "value", &left_value);
        ASTNode_get_attr_int(right, "value", &right_value);
        
        sim_result_t sim_val = ExpNode_op_calc(exp->id, left_value, right_value);
        ASTNode* ret = ASTNode_create("Number");
        ASTNode_add_attr_int(ret, "value", sim_val);
        
        return ret;
    } else if (left_atomic) {

    } else if (right_atomic) {

    } else {
        
    }

    ASTNode* ret = ASTNode_clone(exp);

    return ret;
}

ASTNode* ExpNode_simplify(ASTNode* exp) {
    assert(exp != NULL);
    assert(ASTNode_id_is(exp, "Exp"));
    ASTNode* child = ASTNode_querySelectorOne(exp, "/*");

    ASTNode* simplified_child = ExpNode_simplify_impl(child);
    ASTNode *new_exp = ASTNode_create("Exp");
    ASTNode_add_child(new_exp, simplified_child);
    return new_exp;
}