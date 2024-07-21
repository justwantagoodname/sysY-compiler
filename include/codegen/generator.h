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
    std::vector<RVInstr> instrs;
    std::map<int, int> simm_table;
public:
    void generate(Triples& triples, bool optimize_flag);
};


#endif