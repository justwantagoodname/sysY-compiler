#pragma once

#ifndef PASS_H
#define PASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"

void ConstNode_unfold(ASTNode* root);
void ArrayDecl_flatten(ASTNode* root);

#ifdef __cplusplus
}
#endif

#endif // PASS_H