#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    void ArmTripleGenerator::genArith(Triples& triples, Triples::Triple& triple) {
        printf("---genArith\n");

        bool is_float = false;
        if (triples.getValueType(triple.to) == 2)
            is_float = true;

        Addr op1 = loadTripleValueAddr(triples, triple.e1);
        Addr op2 = loadTripleValueAddr(triples, triple.e2);
        auto cmd = ACmd.nop;

        bool if_r1_save_flg = false;
        Addr r1_save;

        if (!is_float && (triple.cmd == TCmd.div || triple.cmd == TCmd.mod)) {
            // 除法和取模
            // 将op1放入r0
            loadInt(op1, Addr(AB.r0));

            // 检测r1是否被占用
            if (int_temp_reg[0].second) {
                // 将r1存入新的临时变量
                r1_save = getEmptyIntTempReg();
                instrs.push_back({ ACmd.mov, r1_save, AB.r1 });
                if_r1_save_flg = true;
                setTempRegState(r1_save, true);// 设置为占用
            }
            int_temp_reg[0].second = true;
            // 将op2放入r1
            loadInt(op2, Addr(AB.r1));

            Addr dst;
            if (triple.cmd == TCmd.div) {
                instrs.push_back({ ACmd.bl, {"__aeabi_idiv"} });
                dst = AB.r0;
            } else {
                instrs.push_back({ ACmd.bl, {"__aeabi_idivmod"} });
                dst = AB.r1;
            }
            Addr to = loadTripleValueAddr(triples, triple.to);

            storeInt(to, dst);

            if (if_r1_save_flg) {
                instrs.push_back({ ACmd.mov, AB.r1, r1_save });
                setTempRegState(r1_save, false);
            }

            setTempRegState(AB.r1, false);
        } else {
            if (!is_float) {
                op1 = loadInt(op1, triples.getValueType(triple.e1));
            } else {
                op1 = loadFloat(op1, triples.getValueType(triple.e1));
            }

            if (!is_float) {
                op2 = loadInt(op2, triples.getValueType(triple.e2));
            } else {
                op2 = loadFloat(op2, triples.getValueType(triple.e2));
            }

            Addr dst;

            if (!is_float) {

                switch (triple.cmd) {
                case TCmd.add:
                    cmd = ACmd.add;
                    break;
                case TCmd.sub:
                    cmd = ACmd.sub;
                    break;
                case TCmd.mul:
                    cmd = ACmd.mul;
                    break;
                default:
                    panic("ERROR: gen arith int oper.");
                    break;
                }

                dst = getEmptyIntTempReg();
                instrs.push_back({ cmd, dst, op1, op2 });
                setTempRegState(dst, true);

                setTempRegState(op1, false);
                setTempRegState(op2, false);

                Addr to = loadTripleValueAddr(triples, triple.to);

                storeInt(to, dst);
                setTempRegState(dst, false);

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
                setTempRegState(dst, true);
                instrs.push_back({ cmd, dst, op1, op2 });

                setTempRegState(op1, false);
                setTempRegState(op2, false);

                Addr to = loadTripleValueAddr(triples, triple.to);
                storeFloat(to, dst);
                setTempRegState(dst, false);
            }
        }


    }

    void ArmTripleGenerator::genCompare(Triples& triples, Triples::Triple& triple)
    {
        printf("---genCompare\n");

        bool is_float = false;
        if (triples.getValueType(triple.e1) == 2
            || triples.getValueType(triple.e2) == 2)
            is_float = true;

        Addr op1 = loadTripleValueAddr(triples, triple.e1);
        Addr op2 = loadTripleValueAddr(triples, triple.e2);
        auto cmd = ACmd.nop;

        if (!is_float) {
            op1 = loadInt(op1, triples.getValueType(triple.e1));
        } else {
            op1 = loadFloat(op1, triples.getValueType(triple.e1));
        }

        if (!is_float) {
            op2 = loadInt(op2, triples.getValueType(triple.e2));
        } else {
            op2 = loadFloat(op2, triples.getValueType(triple.e2));
        }

        Addr dst;

        switch (triple.cmd)
        {
        case TCmd.jeq:
            cmd = ACmd.beq;
            break;
        case TCmd.jne:
            cmd = ACmd.bne;
            break;
        case TCmd.jlt:
            cmd = ACmd.blt;
            break;
        case TCmd.jle:
            cmd = ACmd.ble;
            break;
        case TCmd.jgt:
            cmd = ACmd.bgt;
            break;
        case TCmd.jge:
            cmd = ACmd.bge;
            break;
        case TCmd.jn0:
            cmd = ACmd.bne;
            break;
        default:
            panic("error cmp command");
            break;
        }

        if (!is_float) {
            instrs.push_back({ ACmd.cmp, op1, op2 });
        } else {
            string cmd_s;
            string cmd_n;
            switch (triple.cmd)
            {
            case TCmd.jeq:
                cmd_s = "eq";
                cmd_n = "ne";
                break;
            case TCmd.jne:
                cmd_s = "ne";
                cmd_n = "eq";
                break;
            case TCmd.jlt:
                cmd_s = "mi";
                cmd_n = "pl";
                break;
            case TCmd.jle:
                cmd_s = "ls";
                cmd_n = "hi";
                break;
            case TCmd.jgt:
                cmd_s = "gt";
                cmd_n = "le";
                break;
            case TCmd.jge:
                cmd_s = "ge";
                cmd_n = "lt";
                break;
            case TCmd.jn0:
                cmd_s = "ne";
                cmd_n = "eq";
                break;
            default:
                panic("error cmp command");
                break;
            }
            instrs.push_back({ ACmd.vcmp, op1, op2 });
            instrs.push_back({ ACmd.vmrs, {"APSR_nzcv"}, {"FPSCR"} });

            //Addr temp = getEmptyIntTempReg();

            //instrs.push_back({ ACmd.mov_when, cmd_s, temp, 1});
            //instrs.push_back({ ACmd.mov_when, cmd_n, temp, 0 });
            //instrs.push_back({ ACmd.uxtb, temp, temp });

            //instrs.push_back({ ACmd.cmp, temp, 0 });

            //cmd = ACmd.bne;
        }

        setTempRegState(op1, false);
        setTempRegState(op2, false);

        instrs.push_back({ cmd, ".l" + std::to_string(triple.to.value) });

    }

    void ArmTripleGenerator::genCall(Triples& triples, Triples::Triple& triple)
    {
        printf("---genCall\n");
        // TODO 特判putf

        // 保留寄存器, 整形和浮点各一个
        Addr int_reg = getEmptyIntTempReg();
        Addr float_reg = getEmptyFloatTempReg();

        setTempRegState(int_reg, true);
        setTempRegState(float_reg, true);

        Triples::TripleValue* icur = nullptr, * fcur = nullptr;

        auto& param_loads = func_params_load[triple.e1.value];

        auto* cur = triple.e2.added; // 得到第一个参数
        int count = 0;

        // 存入参数
        while (cur) {
            int ptype = triples.getValueType(*cur);

            Addr dst = param_loads[count];
            if (dst.base == AB.null) {
                cur = cur->added;
                count++;
                continue;
            }

            if (dst.base >= AB.r0 && dst.base <= AB.pc) {
                if (dst.value > MAX_OFFSET || dst.value < -MAX_OFFSET) {
                    Addr temp = loadInt(dst.value * 4);
                    instrs.push_back({ ACmd.add, temp, dst.base, temp });
                    setTempRegState(dst, false);
                    dst = { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                }
            }

            if (dst.base == AB.reg &&
                (dst.value == float_reg.value
                    || dst.value == int_reg.value)) {
                if (dst.value == int_reg.value)
                    icur = cur;
                else
                    fcur = cur;
            } else {

                Addr p = loadTripleValueAddr(triples, *cur);

                if (triples.funcid_params[triple.e1.value][count + 1].second != 2) {
                    loadInt(p, int_reg, ptype);
                    storeInt(dst, int_reg);
                } else {
                    loadFloat(p, float_reg, ptype);
                    storeFloat(dst, float_reg);
                }
            }

            cur = cur->added;
            count++;
        }

        // 将应当保存到临时寄存器上的放寄存器上
        if (icur) {
            Addr p = loadTripleValueAddr(triples, *icur);
            int ptype = triples.getValueType(*icur);
            loadInt(p, int_reg, ptype);
        }
        if (fcur) {
            Addr p = loadTripleValueAddr(triples, *fcur);
            int ptype = triples.getValueType(*fcur);
            loadFloat(p, float_reg, ptype);
        }

        // call
        instrs.push_back({ ACmd.bl, triples.getFuncName(triple.e1) });
        setTempRegState(int_reg, false);
        setTempRegState(float_reg, false);

        for (auto& a : func_params_load[triple.e1.value]) {
            if (a.base != AB.null)
                setTempRegState(a, false);
        }

        // 将r0存入临时变量
        int ftype = triples.funcid_params[triple.e1.value][0].second;
        if (ftype) {
            Addr ret = ftype != 2 ? AB.r0 : AB.fa0;
            Addr addr = loadTripleValueAddr(triples, triple.to);

            if (triples.getValueType(triple.to) != 2) {
                storeInt(addr, ret);
            } else {
                storeFloat(addr, ret);
            }
        }
    }

    void ArmTripleGenerator::genJmp(Triples& triples, Triples::Triple& triple)
    {
        printf("---genJmp\n");
        instrs.push_back({ ACmd.b, ".l" + std::to_string(triple.to.value) });
    }

    void ArmTripleGenerator::genTag(Triples& triples, Triples::Triple& triple)
    {
        printf("---genTag\n");
        if (triple.e1.type == TTT.lamb)
            instrs.push_back({ ACmd.tag, triples.getLabelName(triple.e1) });
    }

    void ArmTripleGenerator::genMove(Triples& triples, Triples::Triple& triple)
    {
        printf("---genMove\n");

        Addr op1 = loadTripleValueAddr(triples, triple.e1);
        printf("finish load op1 as %s--\n", op1.toString().c_str());

        Addr dst = loadTripleValueAddr(triples, triple.to);
        printf("finish load ast as %s--\n", dst.toString().c_str());

        if (triples.getValueType(triple.to) != 2) {
            op1 = loadInt(op1, triples.getValueType(triple.e1));
            storeInt(dst, op1);
        } else {
            op1 = loadFloat(op1, triples.getValueType(triple.e1));
            storeFloat(dst, op1);
        }

        //if (triples.getValueType(triple.to) != 2) {
        //    storeInt(dst, op1);
        //} else {
        //    storeFloat(dst, op1);
        //}
    }

    void ArmTripleGenerator::genReturn(Triples& triples, Triples::Triple& triple) {
        printf("---genReturn\n");

        int return_type = triples.funcid_params[now_func_id][0].second;

        if (return_type == 1) {
            // int
            auto& ret = triple.e1;
            Addr temp;

            temp = loadTripleValueAddr(triples, triple.e1);
            temp = loadInt(temp, triples.getValueType(triple.e1));

            instrs.push_back({ ACmd.mov, AB.r0, temp });
            setTempRegState(temp, false);

        } else if (return_type == 2) {
            // float
            auto& ret = triple.e1;
            Addr ftemp;

            ftemp = loadTripleValueAddr(triples, triple.e1);
            ftemp = loadFloat(ftemp, triples.getValueType(triple.e1));

            instrs.push_back({ ACmd.vmov, AB.fa0, ftemp });

            setTempRegState(ftemp, false);
        }

        instrs.push_back({ ACmd.b, { ".endof" + triples.getFuncName({now_func_id, TTT.func})} });
    }

    void ArmTripleGenerator::genAllGlobeVars() {
        for (auto& e : globe_map) {
            int idx = e.first;
            string& name = std::get<0>(e.second);
            int type = std::get<1>(e.second);
            int size = std::get<2>(e.second);
            vector<unsigned int>& init_num = std::get<3>(e.second);
            instrs.push_back({ ACmd.lamb, {".data"} });
            instrs.push_back({ ACmd.lamb, {".align  2"} });


            for (auto item : init_num) {
                instrs.push_back({ ACmd.word, item });
                size -= 1;
            }
            if (size > 0) {
                instrs.push_back({ ACmd.space, size * 4 });
            }
            instrs.push_back({ ACmd.tag, {name} });

        }
    }

    void ArmTripleGenerator::genFuncBegin(Triples& triples, int func_id)
    {
        string func_name = triples.getFuncName({ func_id, TTT.func });
        instrs.push_back({ ACmd.lamb, {".text"} });
        instrs.push_back({ ACmd.lamb, {".align  1"} });
        instrs.push_back({ ACmd.lamb, {".global " + func_name} });
        instrs.push_back({ ACmd.lamb, {".syntax unified"} });
        instrs.push_back({ ACmd.lamb, {".type   " + func_name + ", %function"} });

        instrs.push_back({ ACmd.tag, {func_name} });

        func_reg[func_id].push_back(AB.lr);
        instrs.push_back({ ACmd.push, func_reg[func_id] });
        func_reg[func_id].pop_back();



        // 从参数存放位置读取参数并存入相应地址
        auto& params = triples.funcid_params[func_id];
        auto& param_loads = func_params_load[now_func_id];

        // 对参数存储寄存器标记占用
        for (int j = 0; j < param_loads.size(); ++j) {
            if (params[j + 1].first == -1)
                continue;
            setTempRegState(param_loads[j], true);
        }

        instrs.push_back({ ACmd.mov, AB.s0, AB.sp });
        unsigned int d = func_stack_size[func_id] * 4;
        if (d < MAX_ARITH_IMMD)
            instrs.push_back({ ACmd.sub, AB.sp, AB.sp, {(int)d} });
        else {
            Addr temp = loadInt({ (int)d });
            instrs.push_back({ ACmd.sub, AB.sp, AB.sp, temp });
            setTempRegState(temp, false);
        }

        for (int j = 0; j < param_loads.size(); ++j) {
            bool mov_flg = true;
            if (params[j + 1].first == -1)
                continue;
            if (param_loads[j].base == AB.sp && value_addr[params[j + 1].first].base == AB.sp
                && param_loads[j].value == value_addr[params[j + 1].first].value
                - (func_stack_size[now_func_id] + func_reg[now_func_id].size() + 1)) {
                mov_flg = false;
            } else if (param_loads[j].base == value_addr[params[j + 1].first].base
                && param_loads[j].value == value_addr[params[j + 1].first].value) {
                mov_flg = false;
            }

            if (mov_flg) {
                Addr load_addr = param_loads[j];

                Addr dst = value_addr[params[j + 1].first];

                if (dst.base >= AB.r0 && dst.base <= AB.pc) {
                    if (dst.value > MAX_OFFSET || dst.value < -MAX_OFFSET) {
                        Addr temp = loadInt(dst.value * 4);
                        instrs.push_back({ ACmd.add, temp, dst.base, temp });
                        setTempRegState(dst, false);
                        dst = { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                    }
                }

                if (load_addr.base == AB.sp) {
                    load_addr.value += func_stack_size[now_func_id];
                }

                if (params[j + 1].second != 2) {
                    Addr temp = loadInt(load_addr);
                    storeInt(dst, temp);
                } else {
                    Addr temp = loadFloat(load_addr);
                    storeFloat(dst, temp);
                }
                setTempRegState(dst, false);
            }
        }
    }

    void ArmTripleGenerator::genFuncEnd(Triples& triples, int func_id)
    {
        instrs.push_back({ ACmd.tag, { ".endof" + triples.getFuncName({func_id, TTT.func})} });

        unsigned int d = func_stack_size[func_id] * 4;
        if (d < MAX_ARITH_IMMD)
            instrs.push_back({ ACmd.add, AB.sp, AB.sp, {(int)d} });
        else {
            Addr temp = loadInt({ (int)d });
            instrs.push_back({ ACmd.add, AB.sp, AB.sp, temp });
            setTempRegState(temp, false);
        }

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
                genJmp(triples, cur_triple);
                break;

            case TCmd.jn0:
            case TCmd.jeq:
            case TCmd.jne:
            case TCmd.jgt:
            case TCmd.jlt:
            case TCmd.jge:
            case TCmd.jle:
                genCompare(triples, cur_triple);
                break;

            case TCmd.call:
                genCall(triples, cur_triple);
                break;

            case TCmd.load:
            case TCmd.store:
            case TCmd.mov:
                genMove(triples, cur_triple);
                break;

            case TCmd.mset:
                //genMset(triples, cur_triple);
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

        genAllGlobeVars();

        for (int i = 0; i < triples.size(); ++i) {
            Triples::Triple& triple = triples[i];
            // 是函数， 进入函数分析
            if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
                i = genFunction(triples, i);
            }
        }
        getVar2Reg(triples, 5);
    }
}