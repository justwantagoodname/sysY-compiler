#include "cfg.h"

#include <map>

BasicBlock::BasicBlock() : range_begin(-1), range_end(-1), prev(nullptr), succ(nullptr) {
    preds.clear();
    succs.clear();
}
BasicBlock::BasicBlock(int range_begin, int range_end)
    : range_begin(range_begin), range_end(range_end), prev(nullptr), succ(nullptr) {
    preds.clear();
    succs.clear();
}


CFG::CFG(Triples &triples) {
    createCFG(triples);
}
CFG::~CFG() {
    for (auto p : blocks) {
        delete p;
    }
}

bool isJump(Triples::Triple& triple) {
    static auto& TCmd = Triples::Cmd;
    return triple.cmd == TCmd.jeq || triple.cmd == TCmd.jge || triple.cmd == TCmd.jgt || triple.cmd == TCmd.jle
        || triple.cmd == TCmd.jlt || triple.cmd == TCmd.jmp || triple.cmd == TCmd.jn0 || triple.cmd == TCmd.jne;
}

void CFG::createCFG(Triples& triples) {
    blocks.clear();

    // 某个 label 在哪个 bb 里
    // label id -> bb*
    std::map<int, BasicBlock*> label2bb;
    

    BasicBlock *cur_block = new BasicBlock(0, 0);
    for (int cur_line = 0; cur_line < triples.size(); ++cur_line) {
        Triples::Triple &t = triples[cur_line];

        if (!isJump(t)) continue;
        cur_block->range_end = cur_line;
        blocks.push_back(cur_block);
        cur_block = new BasicBlock(cur_line + 1, 0);
    }

    blocks.push_back(cur_block);
}