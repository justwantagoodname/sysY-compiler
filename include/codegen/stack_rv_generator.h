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

    // float value (in integer) -> [index, is 64bit?]
    std::map<int, size_t> simm_table;
    std::map<int, size_t> putf_simm_table;
    std::map<std::string, size_t> string_table;
    std::map<std::string, size_t> func_size;

    // temp variable -> type
    // 0, 1, 2 -> int, float, addr
    std::map<uint, uint> tempvar_type;

    // globe map
    // index -> [name, type, size, init nums]
    std::map<int, std::tuple<std::string, int, int, std::vector<unsigned int>>> globe_map;

    // index -> [id, size]
    std::stack<std::vector<std::pair<int, int>>> cur_stacks;
    std::stack<int> cur_blocks;
    std::string cur_func_name;

    size_t simm_count;
    size_t string_count;
    int cur_smallest_temp;
    void createTable(Triples& triples);
    void calculateSize(Triples& triples);
    void makeGlobeMap(Triples& triples);

    RVOperand getTempOpr(Triples& triples, int temp_id);
    RVOperand getVarOpr(Triples& triples, int var_id);
    RVOperand loadValueOpr(Triples& triples, Triples::TripleValue& e);

    void genArith(Triples& triples, Triples::Triple& triple);
    void genMem(Triples& triples, Triples::Triple& triple);
    void genCompare(Triples& triples, Triples::Triple& triple);
    void genCall(Triples& triples, Triples::Triple& triple);
    void genPutf(Triples& triples, Triples::Triple& triple);
    void genTag(Triples& triples, Triples::Triple& triple);
    void genStack(Triples& triples, Triples::Triple& triple, size_t index);
    void genFuncEnd(Triples& triples, Triples::Triple& triple);
    void genMove(Triples& triples, Triples::Triple& triple);
    void genReturn(Triples& triples, Triples::Triple& triple);
    void genAllStrsFloats();
    void genAllGlobeVars();

public:
    std::vector<RVInstr*> instrs; 
    StackRiscVGenerator();
    ~StackRiscVGenerator();
    void generate(Triples& triples, bool optimize_flag);
};


#endif