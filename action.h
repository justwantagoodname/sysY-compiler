#pragma once

#ifndef ACTION_H
#define ACTION_H

#include "ast.h"

void modifyValueType(ASTNode *value_defs, const char* type);

ASTNode *createOpNode(const char *op, ASTNode *left, ASTNode *right);

ASTNode *createIfNode(ASTNode *cond, ASTNode *then, ASTNode *elseStmt);

ASTNode *createWhileNode(ASTNode *cond, ASTNode *stmt);

ASTNode *collectDecl(ASTNode *scope_node, ASTNode *decls);

char* trimQuoteStr(const char *str);
#endif