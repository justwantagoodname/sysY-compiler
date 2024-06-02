#pragma once

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "lib/uthash.h"

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

    UT_hash_handle hh;
};
typedef struct ASTAttribute ASTAttribute;

struct ASTNode
{
    const char* id;
    
    ASTAttribute *attrs;

    struct ASTNode *children, *parent;
    struct ASTNode *next, *prev; // for siblings
};
typedef struct ASTNode ASTNode;

    /* ASTNode */
ASTNode *ASTNode_create(const char* id);
void ASTNode_lpush_child(ASTNode *parent, ASTNode *child);
void ASTNode_add_child(ASTNode *parent, ASTNode *child);
    /* Attributes */
void ASTNode_add_attr_int(ASTNode *node, const char* key, int value);
void ASTNode_add_attr_str(ASTNode *node, const char* key, const char* value);
void ASTNode_add_attr_float(ASTNode *node, const char* key, float value);
bool ASTNode_get_attr_int(ASTNode *node, const char* key, int *value);
bool ASTNode_get_attr_str(ASTNode *node, const char* key, const char **value);
bool ASTNode_get_attr_float(ASTNode *node, const char* key, float *value);
    /* Utils */
void ASTNode_print(struct ASTNode *node);
void ASTNode_move_children(ASTNode *from, ASTNode *to);
void ASTNode_free(ASTNode *node);
    /* Operators */
bool ASTNode_id_is(ASTNode *node, const char* id);

    /* AST Query */
struct QueryResult {
    ASTNode *node;
    struct QueryResult *next, *prev;
};
typedef struct QueryResult QueryResult;

QueryResult *QueryResult_create(ASTNode *node);
QueryResult *ASTNode_querySelector(ASTNode *node, const char* selector);
#endif