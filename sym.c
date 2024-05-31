#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "lib/uthash.h"
#include "lib/utlist.h"
#include "sym.h"
#include "ast.h"

struct ValueSymbol *ValueSymbol_create(const char* id, enum ValueType type, void *constValue) {
    assert(id != NULL);

    struct ValueSymbol *vSymbol = (struct ValueSymbol *)calloc(1, sizeof(struct ValueSymbol));
    vSymbol->id = strdup(id);
    vSymbol->type = type;

    if (constValue == NULL) {
        vSymbol->hasInitVal = false;
        return vSymbol;
    } else {
        vSymbol->hasInitVal = true;
    }

    if (type == CONST_INT || type == INT) {
        vSymbol->constVal.intVal = *(int *)constValue;
    }

    return vSymbol;
}

void ValueSymbol_print(struct ValueSymbol *self) {
    assert(self != NULL);
    
    if (self->type == CONST_INT) {
        printf("<ConstInt id=\"%s\" value=\"%d\" />\n", self->id, self->constVal.intVal);
    } else if (self->type == INT) {
        printf("<Int id=\"%s\" />\n", self->id);
    } else {
        printf("<Any id=\"%s\" />\n", self->id);
    }

}

struct Scope *Scope_create(struct Scope *parent, const char *scopeId) {
    assert(scopeId != NULL);

    struct Scope *scope = (struct Scope *)calloc(1, sizeof(struct Scope));
    scope->id = strdup(scopeId);
    scope->parent = parent;

    if (parent != NULL) {
        LL_APPEND(parent->children, scope);
    }

    return scope;
}

void Scope_addValueSymbol(struct Scope *scope, struct ValueSymbol *vSymbol) {
    assert(scope != NULL && vSymbol != NULL);

    struct ValueSymbol *tmp = NULL;
    HASH_ADD_STR(scope->vSymbols, id, tmp);
}

void Scope_print_impl(struct Scope *scope) {
    assert(scope != NULL);

    printf("<Scope id=\"%s\">\n", scope->id);
    struct ValueSymbol *cur = NULL, *tmp = NULL;
    if (scope->vSymbols) {
        printf("<Values>\n");
        DL_FOREACH(scope->vSymbols, cur) {
            ValueSymbol_print(cur);
        }
        printf("</Values>\n");
    }

    if (scope->fSymbols) {
        printf("<Functions>\n");
        struct FunctionSymbol *cur = NULL;
        DL_FOREACH(scope->fSymbols, cur) {
            FunctionSymbol_print(cur);
        }
        printf("</Functions>\n");
    }

    if (scope->children) {
        printf("Children:\n");
        struct Scope *cur = NULL;
        DL_FOREACH(scope->children, cur) {
            Scope_print_impl(cur);
        }
    }
    printf("</Scope>\n");
}

void Scope_print(struct Scope *scope) {
    assert(scope != NULL);

    Scope_print_impl(scope);
}

FunctionSymbol *FunctionSymbol_create(const char *id, FuncType type, struct ValueSymbol *params, struct ASTNode *body) {
    FunctionSymbol *func = (FunctionSymbol *)calloc(1, sizeof(FunctionSymbol));
    func->id = strdup(id);
    func->type = type;
    func->params = params;
    func->body = body;
    return func;
}

const char* FT_String[_FuncType_Count] = {
    "any",
    "int",
    "void"
};

void FunctionSymbol_print(FunctionSymbol *fSymbol) {
    assert(fSymbol != NULL);

    printf("<Function type=\"%s\" id=\"%s\">\n", FT_String[fSymbol->type], fSymbol->id);
    printf("<Params>\n");
    struct ValueSymbol *cur = NULL;
    DL_FOREACH(fSymbol->params, cur) {
        ValueSymbol_print(cur);
    }
    printf("</Params>\n");

    printf("<Body>\n");
    ASTNode_print(fSymbol->body);
    printf("</Body>\n");
    printf("</Function>\n");
}