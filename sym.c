#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "lib/uthash.h"
#include "lib/utlist.h"
#include "sym.h"

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

    // print xml
    if (self->type == CONST_INT) {
        printf("<%s type=\"const_int\" value=\"%d\" />\n", self->id, self->constVal.intVal);
    } else if (self->type == INT) {
        printf("<%s type=\"int\" />\n", self->id);
    } else {
        printf("<%s type=\"any\" />\n", self->id);
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

void Scope_print_impl(struct Scope *scope, int depth) {
    assert(scope != NULL && depth >= 0);

#define INDENT do { for (int i = 0; i < depth; i++) putchar('\t'); } while (0);

    INDENT printf("<scope id=\"%s\">\n", scope->id);
    struct ValueSymbol *cur = NULL, *tmp = NULL;
    LL_FOREACH(scope->vSymbols, cur) {
        ValueSymbol_print(cur);
    }

    if (scope->children) {
        INDENT printf("Children:\n");
        struct Scope *cur = NULL;
        LL_FOREACH(scope->children, cur) {
            Scope_print_impl(cur, depth + 1);
        }
    }
    INDENT printf("</scope>\n");

#undef INDENT
}

void Scope_print(struct Scope *scope) {
    assert(scope != NULL);

    Scope_print_impl(scope, 0);
}

