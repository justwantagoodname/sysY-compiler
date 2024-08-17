#pragma once

#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <map>

#include "codegen/riscvinstr.h"
#include "sysY.h"
#include "triples.h"

class Generator {
public:
    virtual void generate(Triples& triples, bool optimize_flag) = 0;
};

class RiscVGenerator : Generator {
private:
    std::vector<RVInstr*> instrs;
    std::map<int, size_t> simm_table;
    std::map<std::string, size_t> string_table;
    size_t simm_count;
    size_t string_count;
    void createTable(Triples& triples);
    void genArith(Triples::Triple& triple);
    void genLoad(Triples::Triple& triple);
public:
    RiscVGenerator();
    void generate(Triples& triples, bool optimize_flag);
};


#endif