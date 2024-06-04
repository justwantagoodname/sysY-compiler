#pragma once

#ifndef TOKEN_H
#define TOKEN_H

#include "sysY.h"

struct TokenInfo {
    const char *name;
    const char *literal;
    size_t line, col;

    struct TokenInfo *next, *prev;    
};

void push_token(const char *name, const char *literal);
void print_tokens(int line, int col);

#endif