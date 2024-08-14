#pragma once
#include "triples.h"
namespace TriplesArmGenerator {
    class ADDRBASE {
    public:
        enum ADDRBASEENUM{
            reg, // ºƒ¥Ê∆˜…œ
            r0, r1, r2, r3, // ≤Œ ˝ºƒ¥Ê∆˜
            r4, r5, r6, r7, r8, r9, r10, r11, // Õ®”√ºƒ¥Ê∆˜
            s0, // r12, ip, scratchºƒ¥Ê∆˜, ø…≈≤”√, ∂¯«“≤ª”√ª÷∏¥, ≈≤»•◊ˆs0¡À£®
            sp, // r13, ’ªºƒ¥Ê∆˜
            lr, // r14, ÷–∂œºƒ¥Ê∆˜
            pc, // r15, pcºƒ¥Ê∆˜

        };
    } AddrBase;
    auto& AB = AddrBase;

    struct Addr {
        ADDRBASE::ADDRBASEENUM base;
        int value;
    };
    class Generator {
    private:
        Triples triples;

        // TODO
        // value -> Œª÷√
        std::vector<Addr> value_addr;
        // temp -> Œª÷√
        std::vector<Addr> temp_addr;
    public:
        // getplace
        void getStackPlace();
        // make
    };
}