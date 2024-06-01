#include <stdio.h>
#include "lib/uthash.h"
#include "lib/utlist.h"
#include "action.h"

ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol) {
    assert(vSymbol != NULL);

    vSymbol->next = vSymbol->prev = NULL;
    DL_APPEND(array, vSymbol);
    return array;
}

FunctionSymbol *addFSArray(FunctionSymbol *array, FunctionSymbol *fSymbol) {
    assert(fSymbol != NULL);
    
    fSymbol->next = fSymbol->prev = NULL;
    DL_APPEND(array, fSymbol);
    return array;
}

void modifyVSType(struct ValueSymbol *array, enum ValueType type, bool isConst) {
    assert(array != NULL);

    struct ValueSymbol *cur;
    DL_FOREACH(array, cur) {
        if (cur->type == ANY_ARRAY) cur->type = type + 1; // to Array Version
        else cur->type = type;
        if (isConst) cur->isConst = true;
        else cur->isConst = false;
    }
}

ValueSymbol *appendVSList(ValueSymbol *array, ValueSymbol *array2) {
    assert(array2 != NULL);

    struct ValueSymbol *cur;
    LL_CONCAT(array, array2);

    return array;
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
