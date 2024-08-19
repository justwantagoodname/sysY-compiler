#include "cfg.h"

#include <map>

static auto& TCmd = Triples::Cmd;

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
    return triple.cmd == TCmd.jeq || triple.cmd == TCmd.jge || triple.cmd == TCmd.jgt || triple.cmd == TCmd.jle
        || triple.cmd == TCmd.jlt || triple.cmd == TCmd.jmp || triple.cmd == TCmd.jn0 || triple.cmd == TCmd.jne;
}

void CFG::createCFG(Triples& triples) {
    blocks.clear();

    // 某个 label 在哪个 bb 里
    // label id -> bb*

    BasicBlock *cur_block = new BasicBlock(0, 0);
    for (int cur_line = 0; cur_line < triples.size(); ++cur_line) {
        Triples::Triple &t = triples[cur_line];
        cur_block->range_end = cur_line;

        // if (t.cmd == TCmd.blkb || t.cmd == TCmd.blkb) {
        //     cur_block->range_end = cur_line - 1;
        //     blocks.push_back(cur_block);
        //     BasicBlock *temp = new BasicBlock(cur_line, cur_line);
        //     temp->prev = cur_block;
        //     cur_block->succ = temp;
        //     temp->succ = new BasicBlock(cur_line + 1, cur_line + 1);
        //     temp->succ->prev = temp;
        //     blocks.push_back(temp);
        //     cur_block = temp->succ;
        // }

        if (!isJump(t) && t.cmd != TCmd.tag) continue;

        BasicBlock *temp;
        if (isJump(t)) {
            cur_block->range_end = cur_line;
            temp = cur_block;
            cur_block = new BasicBlock(cur_line + 1, cur_line + 1);
        } else {
            cur_block->range_end = cur_line - 1;
            temp = cur_block;
            cur_block = new BasicBlock(cur_line, cur_line);
        }

        blocks.push_back(temp);

        temp->succ = cur_block;
        temp->succs.push_back(cur_block);
        cur_block->prev = temp;
        cur_block->preds.push_back(temp);
    }
    blocks.push_back(cur_block);
    
    
    std::map<int, BasicBlock*> label2bb;
    for (auto *p : blocks) {
        if (triples[p->range_begin].cmd == TCmd.tag) {
            label2bb[triples[p->range_begin].e1.value] = p;
        }
    }

    for (auto *p : blocks) {
        if (isJump(triples[p->range_end])) {
            auto *target = label2bb[triples[p->range_end].to.value];
            p->succs.push_back(target);
            target->preds.push_back(p);
        }
    }


    auto check = [&](BasicBlock *p) -> bool {
        if (p->range_begin < 0 || p->range_end >= blocks.size()) return false;
        if (p->range_begin > p->range_end) return false;
        return true;
    };
    if (!check(blocks[0])) blocks.erase(blocks.begin());
    if (!check(blocks[blocks.size() - 1])) blocks.erase(blocks.end() - 1);
}