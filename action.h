#pragma once

#ifndef ACTION_H
#define ACTION_H

#include "ast.h"
#include "sym.h"

struct ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol);

void modifyVSType(struct ValueSymbol *array, enum ValueType type);

ValueSymbol *appendVSList(ValueSymbol *array, ValueSymbol *array2);

ASTNode *createOpNode(const char *op, ASTNode *left, ASTNode *right);

ASTNode *addASTList(ASTNode *list, ASTNode *target);

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt);

ASTNode *createWhileNode(ASTNode *cond, ASTNode *stmt);

char* trimQuoteStr(const char *str);
#endif