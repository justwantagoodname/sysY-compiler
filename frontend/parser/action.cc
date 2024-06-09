#include "sysY.h"
#include "action.h"

void modifyValueType(ASTNode *value_defs, const char* type) {
    assert(value_defs != NULL);
    assert(type != NULL);

    ASTNode *child = NULL;
    DL_FOREACH(value_defs->children, child) {
        ASTNode_add_attr_str(child, "type", type);
    }
}

ASTNode *createOpNode(const char *op, ASTNode *left, ASTNode *right) {
    assert(op != NULL);
    assert(left != NULL);
    assert(right != NULL);

    ASTNode *node = ASTNode_create(op);
    ASTNode_add_child(node, left);
    ASTNode_add_child(node, right);

    return node;
}

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt) {
    assert(cond != NULL);
    assert(then != NULL);

    ASTNode *node = ASTNode_create("If");

    // wrapper cond in a condition node
    ASTNode *condNode = ASTNode_create("Cond");
    ASTNode_add_child(condNode, cond);
    ASTNode_add_child(node, condNode);

    ASTNode *thenNode = ASTNode_create("Then");
    ASTNode_add_child(thenNode, then);
    ASTNode_add_child(node, thenNode);

    if (elseStmt != NULL) {
        ASTNode *elseNode = ASTNode_create("Else");
        ASTNode_add_child(elseNode, elseStmt);
        ASTNode_add_child(node, elseNode);
    }

    return node;
}

ASTNode *createWhileNode(ASTNode *cond, ASTNode *stmt) {
    assert(cond != NULL);
    assert(stmt != NULL);

    ASTNode *node = ASTNode_create("While");

    // wrapper cond in a condition node
    ASTNode *condNode = ASTNode_create("Cond");
    ASTNode_add_child(condNode, cond);
    ASTNode_add_child(node, condNode);

    ASTNode *stmtNode = ASTNode_create("Stmt");
    ASTNode_add_child(stmtNode, stmt);
    ASTNode_add_child(node, stmtNode);

    return node;
}

char* trimQuoteStr(const char *str) {
    assert(str != NULL);

    int len = strlen(str);
    char *newStr = (char *)calloc(len - 1, sizeof(char));
    strncpy(newStr, str + 1, len - 2);

    return newStr;    
}

ASTNode *collectDecl(ASTNode *scope_node, ASTNode *decls) {
    assert(scope_node != NULL);
    assert(decls != NULL);

    ASTNode *scope_decl = ASTNode_querySelectorOne(scope_node, "/Decl");
    ASTNode *block = ASTNode_querySelectorOne(scope_node, "/Block");
    if (ASTNode_id_is(decls, "VarTemp")) {
        ASTNode_copy_children(decls, block);
        ASTNode_move_children(decls, scope_decl);
    } else if (ASTNode_id_is(decls, "ConstantTemp")) {
        ASTNode_move_children(decls, scope_decl);    
    } else {
        fprintf(stderr, "Error! Decl with %s", decls->id);
    }
    ASTNode_free(decls);
    return scope_node;
}