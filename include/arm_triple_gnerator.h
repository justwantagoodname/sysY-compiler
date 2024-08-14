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
            fa8, fa9, fa10, fa11, fa12,
            fa13, fa14, fa15, // 用作临时浮点寄存器

            imd, dimd, //立即数, 双字立即数
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

            vmov,
            vldr,
            vstr,

            vadd,
            vsub,
            vmul,
            vdiv,

            vcvtd2f,
            vcvtf2d,

            cmp,
            beq,
            bne,
            ble,
            blt,
            bge,
            bgt,

            tag,    // 需特判, 放置tag
            word,   // 需特判, 放置word
            ascii,  // 需特判, 放置ascii字符串
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
        // tag
        Addr(std::string s)
            :base(AB.tag), tag(s), value(0) {}
        Addr(ADDRBASE::ADDRBASEENUM b, std::string s)
            :base(b), tag(s), value(0) {}
        Addr(ADDRBASE::ADDRBASEENUM b, int v, std::string s)
            :base(b), tag(s), value(v) {}
        std::string toString();
    };

    struct Instr {
        ARMCMD::ARMCMDENUM cmd;
        Addr e1, e2, e3;

        Instr(ARMCMD::ARMCMDENUM c) :cmd(c) {}
        Instr(ARMCMD::ARMCMDENUM c, Addr e1)
            :cmd(c), e1(e1) {}
        Instr(ARMCMD::ARMCMDENUM c, Addr e1, Addr e2)
            :cmd(c), e1(e1), e2(e2) {}
        Instr(ARMCMD::ARMCMDENUM c, Addr e1, Addr e2, Addr e3)
            :cmd(c), e1(e1), e2(e2), e3(e3) {}
    };

    class ArmTripleGenerator : ::Generator {
    private:
        std::vector <Instr> instrs;

        // value -> 位置
        std::vector<Addr> value_addr;
        // temp -> 位置
        std::vector<Addr> temp_addr;
        // 函数栈大小
        std::vector<int> func_stack_size;

        // 临时寄存器及占用情况表
        std::vector<std::pair<int, bool>>
            int_temp_reg = {
                {AB.r1, false},
                {AB.r2, false},
                {AB.r3, false}
        },
            float_temp_reg = {
                {AB.fa13, false},
                {AB.fa14, false},
                {AB.fa15, false}
        };

    private:
        Addr loadInt(const Addr&);
        Addr loadFloat(const Addr&);
        void storeInt(const Addr&, const Addr&);
        void storeFloat(const Addr&, const Addr&);

        Addr getEmptyIntTempReg();
        Addr getEmptyFloatTempReg();
        void setTempRegState(const Addr&, bool);
    public:
        ArmTripleGenerator();
        // getplace
        void getStackPlace(Triples&);
        // make
        virtual void generate(Triples&, bool);

        // 输出地址分配
        void printAddrs(Triples& triples);

    };
}