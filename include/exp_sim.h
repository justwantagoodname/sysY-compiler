#pragma once

#ifndef EXP_SIM_H
#define EXP_SIM_H

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

ASTNode* ExpNode_simplify(const ASTNode* exp);

void ArrayInitNode_flatten(ASTNode* decl);
bool ArrayInitNode_need_flatten(ASTNode* root);
ASTNode* ArrayInitNode_get_value_by_linear_index(ASTNode* decl, ASTNode* locator);

#ifdef __cplusplus
}
#endif

#endif