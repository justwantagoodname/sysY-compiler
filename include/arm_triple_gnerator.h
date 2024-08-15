#pragma once
#include "triples.h"
#include "codegen/generator.h"
#include <codegen/asm_helper.hpp>
namespace TriplesArmGenerator {

    static auto& TTT = Triples::TT;
    static auto& TCmd = Triples::Cmd;
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
            up_tag, low_tag, //高位读取， 低位读取
            reglist, // 寄存器列表
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

            vcmp,
            vmrs,

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
        Addr(const std::vector<ADDRBASE::ADDRBASEENUM>& inits)
            :base(AB.reglist), value(0) {
            for (auto& r : inits) tag.push_back(r);
        }
        std::string toString() const;
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

        std::string toASM();
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
        // 函数寄存器占用情况, 仅应被调用方保存寄存器
        std::vector<std::vector<ADDRBASE::ADDRBASEENUM>> func_reg;
        // 每个函数参数进入前应保存位置
        std::vector<std::vector<Addr>> func_params_load;

        // 临时寄存器及短时占用状态表（应当在任何跳转前结束占用）
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
        // 正在解析的函数id
        int now_func_id;

    private:
        Addr loadInt(const Addr&, int f = 1);
        Addr loadFloat(const Addr&, int f = 2);
        void storeInt(const Addr&, const Addr&);
        void storeFloat(const Addr&, const Addr&);

        Addr loadTripleValueAddr(const Triples& triples, const Triples::TripleValue&);

        Addr getEmptyIntTempReg();
        Addr getEmptyFloatTempReg();
        int setTempRegState(const Addr&, bool);

        void genArith(Triples& triples, Triples::Triple& triple);
        void genCompare(Triples& triples, Triples::Triple& triple);
        void genCall(Triples& triples, Triples::Triple& triple);
        void genPutf(Triples& triples, Triples::Triple& triple);
        void genJmp(Triples& triples, Triples::Triple& triple);
        void genTag(Triples& triples, Triples::Triple& triple);
        void genMove(Triples& triples, Triples::Triple& triple);
        void genReturn(Triples& triples, Triples::Triple& triple);

        int genFunction(Triples& triples, int begin);
        void genFuncBegin(Triples& triples, int func_id);
        void genFuncEnd(Triples& triples, int func_id);


    public:
        ArmTripleGenerator();
        // getplace
        void getStackPlace(Triples&);
        // make
        virtual void generate(Triples&, bool);

        // 输出地址分配
        void printAddrs(Triples& triples);
        void print();

        void write(AssemblyBuilder&);
    };
}