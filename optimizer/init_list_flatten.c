#include "sysY.h"
#include "ast.h"

ASTNode* ArrayInitNode_flatten(const ASTNode* decl) {
    assert(decl != NULL);
    assert(ASTNode_id_is(decl, "Const") || ASTNode_id_is(decl, "Var"));

    ASTNode* init_value = ASTNode_querySelectorOne(decl, "/ConstInitValue");
    QueryResult* dims = ASTNode_querySelector(decl, "//ArraySize/Number");

    if (init_value == NULL) return ASTNode_clone(decl);
}