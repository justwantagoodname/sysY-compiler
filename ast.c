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

// I Love XML. XD
void ASTNode_print_impl(struct ASTNode *node, int depth) {
    assert(node != NULL);

    printf("<%s", node->id);

    // print attributes
    ASTAttribute *attr = NULL;
    DL_FOREACH(node->attrs, attr) {
        switch (attr->type) {
            case ATTR_TYPE_INT:
                printf(" %s=\"%d\" ", attr->key, attr->value.int_value);
                break;
            case ATTR_TYPE_FLOAT:
                printf(" %s=\"%f\" ", attr->key, attr->value.float_value);
                break;
            default:
                printf(" %s=\"UNKNOWN\" ", attr->key);
                break;
        }
    }

    if (node->children == NULL) {
        printf(" />\n");
        return;
    } else {
        printf(">\n");
    }

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

void ASTNode_add_attr_int(ASTNode *node, const char* key, int value) {
    assert(node != NULL && key != NULL);

    struct ASTAttribute *attr = (struct ASTAttribute *)calloc(1, sizeof(struct ASTAttribute));
    attr->key = strdup(key);
    attr->value.int_value = value;
    attr->type = ATTR_TYPE_INT;

    DL_APPEND(node->attrs, attr);
}

bool ASTNode_id_is(ASTNode *node, const char* id) {
    assert(node != NULL && id != NULL);

    return strcmp(node->id, id) == 0;
}