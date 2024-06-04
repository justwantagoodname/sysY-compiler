#pragma once

#ifndef ACTION_H
#define ACTION_H

#include "ast.h"
#include "sym.h"

ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol);

void modifyValueType(ASTNode *value_defs, const char* type);

ValueSymbol *appendVSList(ValueSymbol *array, ValueSymbol *array2);

ASTNode *createOpNode(const char *op, ASTNode *left, ASTNode *right);

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt);

ASTNode *createWhileNode(ASTNode *cond, ASTNode *stmt);

char* trimQuoteStr(const char *str);
#endif