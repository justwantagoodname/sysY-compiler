#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/utlist.h"
#include "sym.h"
#include "ast.h"



struct ASTNode *ASTNode_create(const char* id, struct Scope *scope) {
    assert(id != NULL && scope != NULL);

    struct ASTNode *node = (struct ASTNode *)calloc(1, sizeof(struct ASTNode));
    node->id = strdup(id);
    node->scope = scope;

    return node;
}

void ASTNode_print_impl(struct ASTNode *node, int depth) {
    assert(node != NULL);

#define INDENT do { for (int i = 0; i < depth; i++) putchar('\t'); } while (0);

    // I Love XML. XD
    INDENT printf("<%s>\n", node->id);
    if (node->parent == NULL || (node->parent != NULL && node->parent->scope != node->scope)) {
        Scope_print(node->scope);
    }
    // Scope_print(node->scope);

    // print children
    if (node->children) {
        struct ASTNode *cur = NULL;
        DL_FOREACH(node->children, cur) {
            ASTNode_print_impl(cur, depth + 1);
        }
    }

    INDENT printf("</%s>\n", node->id);
#undef INDENT
}

void ASTNode_print(struct ASTNode *node) {
    assert(node != NULL);

    ASTNode_print_impl(node, 0);
}