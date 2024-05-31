#pragma once

#ifndef SYM_H
#define SYM_H

#include "lib/uthash.h"
#include <stdbool.h>

enum FuncType {
    ANY_FUNC = 0,
    INT_FUNC,
    VOID_FUNC,
    _FuncType_Count
};
typedef enum FuncType FuncType;

struct FunctionSymbol
{
    const char *id;
    FuncType type;
    struct ValueSymbol *params;
    struct ASTNode *body;

    struct FunctionSymbol *next, *prev;
    UT_hash_handle hh;
};
typedef struct FunctionSymbol FunctionSymbol;

FunctionSymbol *FunctionSymbol_create(const char *id, FuncType type, struct ValueSymbol *params, struct ASTNode *body);
void FunctionSymbol_print(FunctionSymbol *fSymbol);

enum ValueType {
        ANY = 0, // For undetermined type 
        INT,
        CONST_INT,
        _ValueType_Count
};
typedef enum ValueType ValueType;

struct ValueSymbol {
    const char* id; // identifier literal name
    enum ValueType type;

    union {
        int intVal;
    } constVal; // store initial value or constant value
    bool hasInitVal; // whether has initial value

    UT_hash_handle hh;
    struct ValueSymbol *next, *prev;
};
typedef struct ValueSymbol ValueSymbol;

struct ValueSymbol *ValueSymbol_create(const char* id, enum ValueType type, void *constValue);
void ValueSymbol_print(struct ValueSymbol *vSymbol);

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