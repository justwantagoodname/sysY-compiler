#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/utlist.h"
#include "sym.h"
#include "ast.h"



struct ASTNode *ASTNode_create(const char* id, struct Scope *scope) {
    assert(id != NULL);

    struct ASTNode *node = (struct ASTNode *)calloc(1, sizeof(struct ASTNode));
    node->id = strdup(id);
    node->scope = scope;

    return node;
}

void ASTNode_print_impl(struct ASTNode *node, int depth) {
    assert(node != NULL);

    if (node->children == NULL) {
        printf("<%s />\n", node->id);
        return;
    }
    // I Love XML. XD
    printf("<%s>\n", node->id);

    // print children
    if (node->children) {
        struct ASTNode *cur = NULL;
        DL_FOREACH(node->children, cur) {
            ASTNode_print_impl(cur, depth + 1);
        }
    }

    printf("</%s>\n", node->id);
}

void ASTNode_print(struct ASTNode *node) {
    assert(node != NULL);

    ASTNode_print_impl(node, 0);
}

void ASTNode_add_child(ASTNode *parent, ASTNode *child) {
    assert(parent != NULL && child != NULL);

    child->parent = parent;

    DL_APPEND(parent->children, child);
}
