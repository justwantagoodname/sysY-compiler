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

#ifdef __cplusplus
}
#endif

#endif