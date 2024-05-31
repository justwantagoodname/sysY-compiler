#pragma once

#ifndef AST_H
#define AST_H

#include "sym.h"

struct ASTNode
{
    const char* id;
    struct Scope *scope;
    struct ASTNode *children, *parent;

    struct ASTNode *next, *prev;
};
typedef struct ASTNode ASTNode;

ASTNode *ASTNode_create(const char* id, struct Scope *scope);
void ASTNode_add_child(ASTNode *parent, ASTNode *child);
void ASTNode_print(struct ASTNode *node);

#endif