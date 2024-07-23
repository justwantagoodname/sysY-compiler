#pragma once

#ifndef AST_H
#define AST_H

#include "sysY.h"

#ifdef __cplusplus
extern "C" {
#endif

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
ASTNode *ASTNode_create_attr(const char* id, int attr_count, ...);
void ASTNode_set_id(ASTNode *node, const char* id);

    /* Children */
size_t ASTNode_children_size(const ASTNode *node);
void ASTNode_add_nchild(ASTNode *parent, int n, ...);
void ASTNode_lpush_child(ASTNode *parent, ASTNode *child);
void ASTNode_add_child(ASTNode *parent, ASTNode *child);

    /* Attributes */
void ASTNode_add_attr_int(ASTNode *node, const char* key, int value);
void ASTNode_add_attr_str(ASTNode *node, const char* key, const char* value);
void ASTNode_add_attr_float(ASTNode *node, const char* key, float value);
bool ASTNode_has_attr(const ASTNode *node, const char* key);
ASTAttribute *ASTNode_get_attr_or_null(const ASTNode *node, const char* key);
bool ASTNode_get_attr_int(const ASTNode *node, const char* key, int *value);
bool ASTNode_get_attr_str(const ASTNode *node, const char* key, const char **value);
bool ASTNode_get_attr_float(const ASTNode *node, const char* key, float *value);
bool ASTNode_get_attr_number(const ASTNode *node, const char* key, double *value);
bool ASTNode_set_attr_str(ASTNode *node, const char* key, const char* value);

    /* Attribute Comparisons */
bool ASTNode_attr_eq_int(const ASTNode *node, const char* key, int value);
bool ASTNode_attr_eq_str(const ASTNode *node, const char* key, const char* value);
bool ASTNode_attr_eq_float(const ASTNode *node, const char* key, float value);
    /* Utils */
void ASTNode_print(const ASTNode *node);
void ASTNode_copy_attr(const ASTNode *from, ASTNode *to);
void ASTNode_move_children(ASTNode *from, ASTNode *to);
void ASTNode_copy_children(ASTNode *from, ASTNode *to);
void ASTNode_replace(ASTNode *after, ASTNode *before);
ASTNode *ASTNode_clone(const ASTNode *node);
void ASTNode_free(ASTNode *node);
    /* Operators */
bool ASTNode_id_is(const ASTNode *node, const char* id);

    /* AST Query */
struct QueryResult {
    ASTNode *node;
    struct QueryResult *next, *prev;
};
typedef struct QueryResult QueryResult;

QueryResult *QueryResult_create(const ASTNode *node);

QueryResult *ASTNode_querySelector(const ASTNode *node, const char* selector);
ASTNode *ASTNode_querySelectorOne(const ASTNode *node, const char* selector);

QueryResult *ASTNode_querySelectorf(const ASTNode *node, const char* fmt, ...);
ASTNode *ASTNode_querySelectorfOne(const ASTNode *node, const char* fmt, ...);

enum AttrOptionType {
  AttrOptionTypeExists,
  AttrOptionTypeString,
  AttrOptionTypeNumber
};

struct AttrOption {
  bool logicAnd; // true of `lastOption` && this
  char* name;
  union {
    char* str;
    double num;
  } value;
  enum AttrOptionType type;
  struct AttrOption* next, *prev;
};
typedef struct AttrOption AttrOption;


struct SearchParam {
  const char* id;
  AttrOption* options;
  int index;
  const char* prefix;
};
typedef struct SearchParam SearchParam;

#ifdef __cplusplus
}
#endif

bool ASTNode_get_attr_str(const ASTNode *node, const char* key, std::string& value);

#endif