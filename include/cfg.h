#pragma once

#ifndef CFG_H
#define CFG_H

#include "sysY.h"
#include "triples.h"

#include <set>
#include <vector>


struct BasicBlock {
    // [range_begin, range_end]
    int range_begin, range_end;

    std::vector<BasicBlock*> preds, succs;
    BasicBlock* prev;
    BasicBlock* succ;

    std::set<int> use, def;
    std::set<int> in, out;

    BasicBlock();
    BasicBlock(int range_begin, int range_end);
};

class CFG {
private:
    void initUseDef(BasicBlock* block);
    Triples &triples;
public:
    CFG(Triples& triples);
    ~CFG();
public:
    std::vector<BasicBlock*> blocks;
    void createCFG();
    void liveVarAnal();
};

#endif
