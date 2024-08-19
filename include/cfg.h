#pragma once

#ifndef CFG_H
#define CFG_H

#include "sysY.h"
#include "triples.h"

#include <vector>


struct BasicBlock {
    // [range_begin, range_end]
    int range_begin, range_end;

    std::vector<BasicBlock*> preds, succs;
    BasicBlock* prev;
    BasicBlock* succ;

    BasicBlock();
    BasicBlock(int range_begin, int range_end);
};

class CFG {
public:
    CFG(Triples& triples);
    ~CFG();
public:
    std::vector<BasicBlock*> blocks;
    void createCFG(Triples& triples);
    Triples createTriples(Triples& triples);
};

#endif
