#pragma once

#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <map>
#include <stack>

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

    // float value (in integer) -> [index, is 64bit?]
    std::map<int, size_t> simm_table;
    std::map<int, size_t> putf_simm_table;
    std::map<std::string, size_t> string_table;
    std::map<std::string, size_t> func_size;

    // temp variable -> type
    // 0, 1, 2 -> int, float, addr
    std::map<uint, uint> tempvar_type;

    // index -> [name, size]
    std::stack<std::vector<std::pair<std::string, int>>> cur_stacks;
    std::stack<int> cur_blocks;
    std::string cur_func_name;

    size_t simm_count;
    size_t string_count;
    void createTable(Triples& triples);
    void calculateSize(Triples& triples);
    void getTempVarType(Triples& triples);

    void genArith(Triples& triples, Triples::Triple& triple);
    void genLoad(Triples& triples, Triples::Triple& triple);
    void genCall(Triples& triples, Triples::Triple& triple);
    void genPutf(Triples& triples, Triples::Triple& triple);
    void genTag(Triples& triples, Triples::Triple& triple);
    void genStack(Triples& triples, Triples::Triple& triple);
    void genReturn(Triples& triples, Triples::Triple& triple);
    void genAllStrsFloats();
public:
    StackRiscVGenerator();
    void generate(Triples& triples, bool optimize_flag);
};


#endif