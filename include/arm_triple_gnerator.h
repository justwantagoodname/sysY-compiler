#pragma once
#include "triples.h"
#include "codegen/generator.h"
namespace TriplesArmGenerator {
    static class ADDRBASE {
    public:
        enum ADDRBASEENUM {
            null,
            reg, // 寄存器上

            r0, r1, r2, r3, // 参数寄存器 | 临时寄存器
            r4, r5, r6, r7, r8, r9, r10, r11, // 通用寄存器
            s0, // r12, ip, scratch寄存器, 可挪用, 而且不用恢复, 挪去做s0了（
            sp, // r13, 栈寄存器
            lr, // r14, 中断寄存器
            pc, // r15, pc寄存器

            fa0, fa1, fa2, fa3, fa4, fa5, fa6, fa7, // 浮点寄存器

            imd, //立即数
            tag, // 是tag
            up_tag, low_tag //高位读取， 低位读取
        };
    } AddrBase;
    static ADDRBASE& AB = AddrBase;

    static class ARMCMD {
    public:
        enum ARMCMDENUM {
            nop,

            mov,
            movw,
            movt,
            movs,
            str,
            ldr,

            push,
            pop,

            b,  // jmp
            bl, // call
            bx,

            add,
            adds,
            sub,
            mul,
            div,
            mod,

            lsls,
            lsrs,

            fadd,
            fsub,
            fmul,
            fdiv,

            cmp,
            beq,
            bne,
            ble,
            blt,
            bge,
            bgt,

        };
    }ACmd;

    struct Addr {
        ADDRBASE::ADDRBASEENUM base;
        int value;
        std::string tag;
        Addr() :base(AB.null), value(0) {}
        // stack 地址
        Addr(ADDRBASE::ADDRBASEENUM b, int v)
            :base(b), value(v) {}
        // 寄存器地址
        Addr(ADDRBASE::ADDRBASEENUM r)
            :base(AB.reg), value(r) {}
        // 立即数
        Addr(int n)
            :base(AB.imd), value(n) {}
        Addr(std::string s)
            :base(AB.tag), tag(s), value(0) {}
        std::string toString();
    };

    struct Instr {
        ARMCMD::ARMCMDENUM cmd;
        Addr e1, e2, e3;
    };

    class ArmTripleGenerator : ::Generator {
    private:
        std::vector <Instr> instrs;

        // TODO
        // value -> 位置
        std::vector<Addr> value_addr;
        // temp -> 位置
        std::vector<Addr> temp_addr;
        // 函数栈大小
        std::vector<int> func_stack_size;

    private:
        Addr loadInt(Addr);
        Addr loadFloat(Addr);
    public:
        ArmTripleGenerator();
        // getplace
        void getStackPlace(Triples&);
        // make
        virtual void generator(Triples&, bool);

        // 输出地址分配
        void printAddrs(Triples& triples);

    };
}