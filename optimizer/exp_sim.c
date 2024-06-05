#include "sysY.h"
#include "ast.h"

ASTNode* ExpNode_simplify_impl(ASTNode* exp) {
    assert(exp != NULL);

    return exp;
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