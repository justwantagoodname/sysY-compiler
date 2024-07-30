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


/*

Stack info:

high -> low
+-----------+-------------+
|  ra (8B)  | old s0 (8B) |
+-----------+-------------+



*/


class StackRiscVGenerator : Generator {
private:
    struct Symbol {
        std::string name;
        
    };
    std::vector<RVInstr*> instrs;
    std::map<int, size_t> simm_table;
    std::map<std::string, size_t> string_table;
    std::map<std::string, size_t> func_size;

    // temp variable -> type
    // 0, 1, 2 -> int, float, addr
    std::map<uint, uint> tempvar_type;
    
    size_t simm_count;
    size_t string_count;
    void createTable(Triples& triples);
    void calculateSize(Triples& triples);
    void getTempVarType(Triples& triples);

    void genArith(Triples& triples, Triples::Triple& triple);
    void genLoad(Triples& triples, Triples::Triple& triple);
    void genCall(Triples& triples, Triples::Triple& triple);
    void genTag(Triples& triples, Triples::Triple& triple);
    void genStack(Triples& triples, Triples::Triple& triple);
    void genAllStrsFloats();
public:
    StackRiscVGenerator();
    void generate(Triples& triples, bool optimize_flag);
};


#endif