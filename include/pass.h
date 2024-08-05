#pragma once

#ifndef PASS_H
#define PASS_H

#include "ast.h"

void ConstNode_fold(ASTNode* root);
void ArrayDecl_flatten(ASTNode* root);

#endif // PASS_H