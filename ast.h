#pragma once

#ifndef AST_H
#define AST_H

#include "sym.h"

enum AttributeType
{
    ATTR_TYPE_UNKNOWN,
    ATTR_TYPE_INT,
    ATTR_TYPE_FLOAT,
    ATTR_TYPE_STR,
};

struct ASTAttribute
{
    const char* key;
    union {
        int int_value;
        float float_value;
        char* str_value;
    } value;
    enum AttributeType type;
    struct ASTAttribute *next, *prev;
};
typedef struct ASTAttribute ASTAttribute;

struct ASTNode
{
    const char* id;
    struct Scope *scope;
    
    ASTAttribute *attrs;

    struct ASTNode *children, *parent;
    struct ASTNode *next, *prev;
};
typedef struct ASTNode ASTNode;

ASTNode *ASTNode_create(const char* id, struct Scope *scope);
void ASTNode_add_child(ASTNode *parent, ASTNode *child);
void ASTNode_add_attr_int(ASTNode *node, const char* key, int value);
void ASTNode_add_attr_str(ASTNode *node, const char* key, const char* value);
void ASTNode_print(struct ASTNode *node);
bool ASTNode_id_is(ASTNode *node, const char* id);
#endif