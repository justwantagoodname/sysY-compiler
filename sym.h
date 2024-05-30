#pragma once

#ifndef SYM_H
#define SYM_H

#include "lib/uthash.h"
#include <stdbool.h>


struct FunctionSymbol
{
    /* data */
};

enum ValueType {
        ANY = 0, // For undetermined type 
        INT,
        CONST_INT,
};

struct ValueSymbol {
    const char* id; // identifier literal name
    enum ValueType type;

    union {
        int intVal;
        struct ASTNode *initExp;
    } constVal; // store initial value or constant value
    bool hasInitVal; // whether has initial value

    UT_hash_handle hh;
    struct ValueSymbol *next;
};
typedef struct ValueSymbol ValueSymbol;

struct ValueSymbol *ValueSymbol_create(const char* id, enum ValueType type, void *constValue);
void ValueSymbol_print(struct ValueSymbol *vSymbol, int depth);

struct Scope {
    char* id; // Scope name

    struct Scope *parent;

    struct ValueSymbol *vSymbols;

    struct FunctionSymbol *fSymbols;

    struct Scope *children;

    struct Scope *next;
};
typedef struct Scope Scope;

struct Scope *Scope_create(struct Scope *parent, const char *scopeId);
void Scope_addValueSymbol(struct Scope *scope, struct ValueSymbol *vSymbol);
void Scope_print(struct Scope *scope);
struct ValueSymbol *Scope_find(const char* id);

#endif