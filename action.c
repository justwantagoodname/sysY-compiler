#include "lib/uthash.h"
#include "lib/utlist.h"
#include "action.h"

struct ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol) {
    assert(vSymbol != NULL);

    LL_PREPEND(array, vSymbol);
    return array;
}

void modifyVSType(struct ValueSymbol *array, enum ValueType type) {
    assert(array != NULL);

    struct ValueSymbol *cur;
    LL_FOREACH(array, cur) {
        cur->type = type;
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

    ASTNode *node = ASTNode_create(op, NULL);
    ASTNode_add_child(node, left);
    ASTNode_add_child(node, right);

    return node;
}

ASTNode *addASTList(ASTNode *list, ASTNode *target) {
    assert(target != NULL);

    DL_APPEND(list, target);

    return list;
}

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt) {
    assert(cond != NULL);
    assert(then != NULL);

    ASTNode *node = ASTNode_create("If", NULL);

    // wrapper cond in a condition node
    ASTNode *condNode = ASTNode_create("Cond", NULL);
    ASTNode_add_child(condNode, cond);
    ASTNode_add_child(node, condNode);

    ASTNode *thenNode = ASTNode_create("Then", NULL);
    ASTNode_add_child(thenNode, then);
    ASTNode_add_child(node, thenNode);

    if (elseStmt != NULL) {
        ASTNode *elseNode = ASTNode_create("Else", NULL);
        ASTNode_add_child(elseNode, elseStmt);
        ASTNode_add_child(node, elseNode);
    }

    return node;
}