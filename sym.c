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
    switch (type)
    {
    case INT:
        vSymbol->constVal.intVal = *(int *)constValue;
        break;
    
    default:
        break;
    }

    return vSymbol;
}

void ValueSymbol_print(struct ValueSymbol *self) {
    assert(self != NULL);
    printf("<");
    switch (self->type)
    {
    case INT:
        printf("Int");
        break;
    case INT_ARRAY:
        printf("IntArray");
        break;

    default:
        printf("Any");
        break;
    }
    if (self->isConst) printf(" const");

    printf(" id=\"%s\"", self->id); 
    if (self->hasInitVal) {
        switch (self->type)
        {
        case INT:
            printf(" value=\"%d\"", self->constVal.intVal);
            break;
        
        default:
            printf(" value");
            break;
        }
    }
    if (self->type == INT_ARRAY && self->extra.arraySize != NULL) {
        printf(">\n");
        ASTNode *cur = NULL;
        DL_FOREACH(self->extra.arraySize, cur) ASTNode_print(cur);
        printf("</IntArray>\n");
    } else {
        printf("/>\n");
    }

}

struct ValueSymbol *ValueSymbol_create_array(const char *id, enum ValueType type, struct ASTNode *arraySize, void* constValue) {
    assert(id != NULL);

    struct ValueSymbol *vSymbol = (struct ValueSymbol *)calloc(1, sizeof(struct ValueSymbol));
    vSymbol->id = strdup(id);
    vSymbol->type = type;
    vSymbol->extra.arraySize = arraySize;

    if (constValue == NULL) {
        vSymbol->hasInitVal = false;
        return vSymbol;
    } else {
        vSymbol->hasInitVal = true;
    }

    return vSymbol;
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