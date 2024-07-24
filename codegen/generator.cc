#include "codegen/generator.h"


RiscVGenerator::RiscVGenerator() : simm_count(0), string_count(0) {
    instrs.clear();
    simm_table.clear();
    string_table.clear();
}

void RiscVGenerator::createTable(Triples& triples) {
    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple &triple = triples[index];
        if (triple.cmd == Triples::Cmd.call) continue;
        
        if (triple.e1.type == Triples::TT.fimd) {
            int value = triple.e1.value;
            if (simm_table.find(value) == simm_table.end()) {
                simm_table[value] = simm_count++;
            }
        } else if (triple.e1.type == Triples::TT.str) {
            const std::string &str = triples.getValueString(triple.e1);
            if (string_table.find(str) == string_table.end()) {
                string_table[str] = string_count++;
            }
        }


        if (triple.e2.type == Triples::TT.fimd) {
            int value = triple.e2.value;
            if (simm_table.find(value) == simm_table.end()) {
                simm_table[value] = simm_count++;
            }
        } else if (triple.e2.type == Triples::TT.str) {
            const std::string &str = triples.getValueString(triple.e2);
            if (string_table.find(str) == string_table.end()) {
                string_table[str] = string_count++;
            }
        }
    }

    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple &triple = triples[index];
        if (triple.cmd != Triples::Cmd.call) continue;

        // WTF is this genius design...
        for (auto t = triple.e2.added; t; t = t->added) {
            if (t->type == Triples::TT.fimd) {
                int value = t->value;
                if (simm_table.find(value) == simm_table.end()) {
                    simm_table[value] = simm_count++;
                }
            } else if (t->type == Triples::TT.str) {
                const std::string &str = triples.getValueString(*t);
                if (string_table.find(str) == string_table.end()) {
                    string_table[str] = string_count++;
                }
            }
        }
    }
}

void RiscVGenerator::generate(Triples& triples, bool optimize_flag) {
    if (optimize_flag) {
        panic("TODO: RiscVGenerator::generate: optimize");
    }

    createTable(triples);
    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple &triple = triples[index];

        
    }
    panic("TODO!!!!!!!");
    return;
}