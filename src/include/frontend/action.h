#pragma once

#ifndef ACTION_H
#define ACTION_H

#include "ast.h"

void modifyValueType(ASTNode *value_defs, const char* type);

ASTNode *createOpNode(const char *op, ASTNode *left, ASTNode *right);

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt);

ASTNode *createWhileNode(ASTNode *cond, ASTNode *stmt);

ASTNode *collectDecl(ASTNode *scope_node, ASTNode *decls);

char* getAnonymousName();

char* trimQuoteStr(const char *str);

AttrOption* AttrOption_create_has(const char* name);
AttrOption* AttrOption_create_str(const char* name, const char* value);
AttrOption* AttrOption_create_num(const char* name, double value);
AttrOption* AttrOption_push_with_logic(AttrOption* list, AttrOption* lastOption, bool logicAnd);

typedef QueryResult* (*SearchFunc)(QueryResult*, const SearchParam*);

#define moveList(src, dest) do { if ((src) != (dest)) freeList(&(dest)); (dest) = (src); (src) = NULL; } while(0);
#define copyList(src, dest) do { \
                                  if ((src) != (dest)) { \
                                    freeList(&(dest));   \
                                    QueryResult *cur = NULL; \
                                    DL_FOREACH((src), (cur)) { QueryResult *record = QueryResult_create(cur->node); DL_APPEND((dest), record); } \
                                  }} while (0);
void freeList(QueryResult **list);
void execSearch(QueryResult **list, QueryResult **result, SearchFunc func, const SearchParam *param);
QueryResult* searchChildName(QueryResult* cur, const SearchParam* param);
QueryResult *searchDescendentName(QueryResult* cur, const SearchParam* param);
QueryResult *searchParent(QueryResult* cur, const SearchParam* param);
QueryResult *searchAncestor(QueryResult* cur, const SearchParam* param);
QueryResult *searchCurrentName(QueryResult* cur, const SearchParam* param);
#endif