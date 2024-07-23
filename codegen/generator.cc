#include "codegen/generator.h"

void RiscVGenerator::generate(Triples& triples, bool optimize_flag) {
    if (optimize_flag) {
        panic("TODO: RiscVGenerator::generate: optimize");
    }

    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple &triple = triples[index];

        
    }
    return;
}