#pragma once

#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>

#include "codegen/riscvinstr.h"
#include "sysY.h"
#include "triples.h"

class Generator {
public:
    virtual void generate(Triples& triples, bool optimize_flag) = 0;
};

class RiscVGenerator : Generator {
private:
    std::vector<RiscVInstr> instrs;
public:
    void generate(Triples& triples, bool optimize_flag);
};


#endif