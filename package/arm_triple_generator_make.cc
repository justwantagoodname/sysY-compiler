#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    void ArmTripleGenerator::genArith(Triples& triples, Triples::Triple& triple) {
        bool is_float = false;
        if (triples.getValueType(triple.to) == 2)
            is_float = true;

        Addr op1 = loadTripleValueAddr(triples, triple.e1);
        Addr op2 = loadTripleValueAddr(triples, triple.e2);
        Addr to = loadTripleValueAddr(triples, triple.to);
        auto cmd = ACmd.nop;

        if (!is_float) {
            op1 = loadInt(op1, triples.getValueType(triple.e1));
        } else {
            op1 = loadFloat(op1, triples.getValueType(triple.e1));
        }
        setTempRegState(op1, true);

        if (!is_float) {
            op2 = loadInt(op2, triples.getValueType(triple.e2));
        } else {
            op2 = loadFloat(op2, triples.getValueType(triple.e2));
        }
        setTempRegState(op2, true);

        Addr dst;

        if (!is_float) {

            switch (triple.cmd)
            {
            case TCmd.add:
                cmd = ACmd.add;
                break;
            case TCmd.sub:
                cmd = ACmd.sub;
                break;
            case TCmd.mul:
                cmd = ACmd.mul;
                break;
            case TCmd.div:
                cmd = ACmd.div;
                break;
            case TCmd.mod:
                cmd = ACmd.mod;
                break;
            default:
                panic("ERROR: gen arith int oper.");
                break;
            }

            dst = getEmptyIntTempReg();
            instrs.push_back({ cmd, dst, op1, op2 });

            storeInt(to, dst);

        } else {

            switch (triple.cmd) {
            case TCmd.add:
                cmd = ACmd.vadd;
                break;
            case TCmd.sub:
                cmd = ACmd.vsub;
                break;
            case TCmd.mul:
                cmd = ACmd.vmul;
                break;
            case TCmd.div:
                cmd = ACmd.vdiv;
                break;
            default:
                panic("ERROR: gen arith float oper.");
                break;
            }

            dst = getEmptyFloatTempReg();
            instrs.push_back({ cmd, dst, op1, op2 });

            storeFloat(to, dst);
        }

        setTempRegState(op1, false);
        setTempRegState(op2, false);

    }

    void ArmTripleGenerator::genCall(Triples& triples, Triples::Triple& triple)
    {
        // TODO 特判putf
            
        auto* cur = triple.e1.added; // 得到第一个参数
        int count = 0;
        // 存入第 0 - 3 参数到 r0 - r3
        // 倒序存入 4 - 8 参数到栈顶下方
        while (cur) {
            

            
            cur = cur->added;
            ++count;
        }


        // call
        
        // 将r0存入临时变量
    }

    void ArmTripleGenerator::genTag(Triples& triples, Triples::Triple& triple)
    {
        instrs.push_back({ ACmd.tag, triples.getLabelName(triple.e1.value) });
    }

    void ArmTripleGenerator::genMove(Triples& triples, Triples::Triple& triple)
    {
        Addr op1 = loadTripleValueAddr(triples, triple.e1);
        Addr dst = loadTripleValueAddr(triples, triple.to);
        if (triples.getValueType(triple.e1) != 2) {
            op1 = loadInt(op1, triples.getValueType(triple.e1));
        } else {
            op1 = loadFloat(op1, triples.getValueType(triple.e1));
        }

        if (triples.getValueType(triple.to) != 2) {
            storeInt(dst, op1);
        } else {
            storeFloat(dst, op1);
        }
    }

    void ArmTripleGenerator::genReturn(Triples& triples, Triples::Triple& triple) {
        int return_type = triples.funcid_params[now_func_id][0].second;

        if (return_type == 1) {
            // int
            auto& ret = triple.e1;
            Addr temp;

            temp = loadTripleValueAddr(triples, triple.e1);
            temp = loadInt(temp, triples.getValueType(triple.e1));

            instrs.push_back({ ACmd.mov, AB.r0, temp });

        } else if (return_type == 2) {
            // float
            auto& ret = triple.e1;
            Addr ftemp;

            ftemp = loadTripleValueAddr(triples, triple.e1);
            ftemp = loadInt(ftemp, triples.getValueType(triple.e1));

            instrs.push_back({ ACmd.vmov, AB.fa0, ftemp });
        }

        instrs.push_back({ ACmd.b, { ".endof" + triples.getFuncName({now_func_id, TTT.func})} });
    }

    void ArmTripleGenerator::genFuncBegin(Triples& triples, int func_id)
    {
        instrs.push_back({ ACmd.tag, {triples.getFuncName({func_id, TTT.func})} });

        func_reg[func_id].push_back(AB.lr);
        instrs.push_back({ ACmd.push, func_reg[func_id] });
        func_reg[func_id].pop_back();

        instrs.push_back({ ACmd.mov, AB.s0, AB.sp });
        instrs.push_back({ ACmd.sub, AB.sp, AB.sp, func_stack_size[func_id] * 4 });

        // TODO 考虑浮点数在浮点数寄存器上的存放
        // 将r0 - r3 放入栈
        for (int i = 1; i < std::min<int>(4 + 1, triples.funcid_params[func_id].size()); ++i) {
            storeInt(value_addr[triples.funcid_params[func_id][i].first], { AB.reg, AB.r0 + i - 1 });
        }
    }

    void ArmTripleGenerator::genFuncEnd(Triples& triples, int func_id)
    {
        instrs.push_back({ ACmd.tag, { ".endof" + triples.getFuncName({func_id, TTT.func})} });

        instrs.push_back({ ACmd.add, AB.sp, AB.sp, func_stack_size[func_id] * 4 });

        func_reg[func_id].push_back(AB.pc);
        instrs.push_back({ ACmd.pop, func_reg[func_id] });
        func_reg[func_id].pop_back();
    }

    int TriplesArmGenerator::ArmTripleGenerator::genFunction(Triples& triples, int begin)
    {
        int func_id = triples[begin].e1.value;
        int block_id = triples[begin + 1].e1.value;

        now_func_id = func_id;
        genFuncBegin(triples, func_id);

        int i = begin;
        for (; i < triples.size(); ++i) {
            Triples::Triple& cur_triple = triples[i];
            // 当前函数结束
            if (cur_triple.cmd == TCmd.blke && cur_triple.e1.value == block_id) {
                break;
            }

            switch (cur_triple.cmd) {
            case TCmd.add:
            case TCmd.sub:
            case TCmd.mul:
            case TCmd.div:
            case TCmd.mod:
                genArith(triples, cur_triple);
                break;

            case TCmd.jmp:
            case TCmd.jn0:
            case TCmd.jeq:
            case TCmd.jne:
            case TCmd.jgt:
            case TCmd.jlt:
            case TCmd.jge:
            case TCmd.jle:
                //genCompare(triples, cur_triple);
                break;

            case TCmd.call:
                genCall(triples, cur_triple);
                break;

            case TCmd.load:
            case TCmd.store:
            case TCmd.mov:
                genMove(triples, cur_triple);
                break;

            case TCmd.tag:
                genTag(triples, cur_triple);
                break;

            case TCmd.ret:
            case TCmd.rev:
                genReturn(triples, cur_triple);
                break;


            default:
                // panic("Error");
                break;
            }
        }

        genFuncEnd(triples, func_id);
        now_func_id = -1;
        return i;
    }

    void ArmTripleGenerator::generate(Triples& triples, bool O)
    {
        getStackPlace(triples);

        printAddrs(triples);

        for (int i = 0; i < triples.size(); ++i) {
            Triples::Triple& triple = triples[i];
            // 是函数， 进入函数分析
            if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
                i = genFunction(triples, i);
            }
        }
    }
}