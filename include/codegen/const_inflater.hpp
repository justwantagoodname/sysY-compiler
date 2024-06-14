#pragma once

#ifndef CONST_INFLATER_H
#define CONST_INFLATER_H

#include "ast.h"
#include "ashelper.hpp"

class ConstInflater {
private:
    ASTNode* compunit;
    int word_size;
    int word_align;

    char* getLabel(ASTNode* const_decl);
    void inflateConstDecl(ASTNode* const_decl, AssemblyBuilder& asm_builder);

public:
    ConstInflater(ASTNode* Compunit, int wordSize = 4, int wordAlign = 2) { // default for arm32
        assert(Compunit != nullptr);
        assert(ASTNode_id_is(Compunit, "CompUnit"));
        this->word_size = wordSize;
        this->word_align = wordAlign;
        this->compunit = Compunit;
    }
    void inflate(AssemblyBuilder& asm_builder);
};

#endif