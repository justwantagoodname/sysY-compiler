#include "codegen/stack_rv_generator.h"

// typedef Cmd and TT
auto& TCmd = Triples::Cmd;
auto& TTT = Triples::TT;

StackRiscVGenerator::StackRiscVGenerator() : simm_count(0), string_count(0) {
    instrs.clear();
    simm_table.clear();
    string_table.clear();
    func_size.clear();

    tempvar_type.clear();

    cur_smallest_temp = -1;
}
StackRiscVGenerator::~StackRiscVGenerator() {
    for (auto p : instrs) delete p;
}

void StackRiscVGenerator::genArith(Triples& triples, Triples::Triple& triple) {
    // 正在施工
    bool is_float = false;
    if (triples.getValueType(triple.to) == 2)
        is_float = true;

    RVOp cmd;
    RVOperand op1, op2;

    auto loadOpr = [&](const Triples::TripleValue& e, RVRegs reg, RVRegs sreg) -> RVOperand {
        RVOperand oper;
        if (e.type == TTT.temp) {
            oper = getTempOpr(triples, e.value);
            if (!is_float) {
                instrs.push_back(new RVMem(RVOp::LW, make_reg(reg), oper));
                return make_reg(reg);
            } else {
                instrs.push_back(new RVMem(RVOp::FLW, make_sreg(sreg), oper));
                return make_sreg(sreg);
            }
        } else if (e.type == TTT.dimd) {
            if (!is_float) {
                if (e.value >= -2048 && e.value <= 2047)
                    return make_imm(e.value);
                else {
                    instrs.push_back(new RVMem(RVOp::LI, make_reg(reg), make_imm(e.value)));
                    return make_reg(reg);
                }
            } else {
                float tf = e.value;
                int value = *(int*)(&tf);
                if (simm_table.find(value) == simm_table.end()) {
                    simm_table[value] = simm_count++;
                }
                oper = make_addr(".LC" + std::to_string(simm_table[value]));
                instrs.push_back(new RVMem(RVOp::FLD, make_sreg(sreg), oper));
                return make_sreg(sreg);
            }

        } else if (e.type == TTT.fimd) {
            oper = make_addr(".LC" + std::to_string(simm_table[e.value]));
            instrs.push_back(new RVMem(RVOp::FLD, make_sreg(sreg), oper));
            return make_sreg(sreg);
        }
        panic("Error in loadOper at genArith");
        };

    op1 = loadOpr(triple.e1, a3, fa3);
    op2 = loadOpr(triple.e2, a4, fa4);

    if (op1.isimm()) {
        instrs.push_back(new RVMem(RVOp::LI, make_reg(a3), op1));
        op1 = make_reg(a3);
    }

    if (!is_float) {
        switch (triple.cmd)
        {
        case TCmd.add:
            cmd = RVOp::ADD;
            break;
        case TCmd.sub:
            cmd = RVOp::SUB;
            break;
        case TCmd.mul:
            cmd = RVOp::MUL;
            break;
        case TCmd.div:
            cmd = RVOp::DIV;
            break;
        case TCmd.mod:
            cmd = RVOp::MOD;
            break;
        default:
            break;
        }
    } else {
        switch (triple.cmd) {
        case TCmd.add:
            cmd = RVOp::FADD;
            break;
        case TCmd.sub:
            cmd = RVOp::FSUB;
            break;
        case TCmd.mul:
            cmd = RVOp::FMUL;
            break;
        case TCmd.div:
            cmd = RVOp::FDIV;
            break;
        case TCmd.mod:
            cmd = RVOp::FMOD;
            break;
        default:
            break;
        }
    }

    RVOperand dst = is_float ? make_sreg(fa5) : make_reg(a5);

    instrs.push_back(new RVArith(cmd, dst, op1, op2));
    if (!is_float) {
        instrs.push_back(new RVMem(RVOp::SW, getTempOpr(triples, triple.to.value), dst));
    } else {
        instrs.push_back(new RVMem(RVOp::FSW, getTempOpr(triples, triple.to.value), dst));
    }

}

/// <summary>
/// 初始化常量表
/// </summary>
/// <param name="triples"></param>
void StackRiscVGenerator::createTable(Triples& triples) {

    // 处理非call的float立即数和str常量，保存至对应表
    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple& triple = triples[index];
        if (triple.cmd == TCmd.call) continue;

        if (triple.e1.type == TTT.fimd) {
            int value = triple.e1.value;
            if (simm_table.find(value) == simm_table.end()) {
                simm_table[value] = simm_count++;
            }
        } else if (triple.e1.type == TTT.str) {
            const std::string& str = triples.getValueString(triple.e1);
            if (string_table.find(str) == string_table.end()) {
                string_table[str] = string_count++;
            }
        }

        if (triple.e2.type == TTT.fimd) {
            int value = triple.e2.value;
            if (simm_table.find(value) == simm_table.end()) {
                simm_table[value] = simm_count++;
            }
        } else if (triple.e2.type == TTT.str) {
            const std::string& str = triples.getValueString(triple.e2);
            if (string_table.find(str) == string_table.end()) {
                string_table[str] = string_count++;
            }
        }
    }

    // 处理call中的float立即数和str常量
    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple& triple = triples[index];
        if (triple.cmd != TCmd.call) continue;
        bool is_putf = (triples.getFuncName(triple.e1) == "putf");

        // WTF is this genius design...
        for (auto t = triple.e2.added; t; t = t->added) {
            if (t->type == TTT.fimd) {
                int value = t->value;
                if (!is_putf && simm_table.find(value) == simm_table.end()) {
                    simm_table[value] = simm_count++;
                }
                if (is_putf && putf_simm_table.find(value) == putf_simm_table.end()) {
                    putf_simm_table[value] = simm_count++;
                }
            } else if (t->type == TTT.str) {
                const std::string& str = triples.getValueString(*t);
                if (string_table.find(str) == string_table.end()) {
                    string_table[str] = string_count++;
                }
            }
        }
    }
}

/// <summary>
/// 计算函数所占用的栈帧空间
/// </summary>
/// <param name="triples"></param>
void StackRiscVGenerator::calculateSize(Triples& triples) {
    size_t cur_line = 0;
    // 循环直到处理完整个序列
    while (cur_line < triples.size()) {
        Triples::Triple& triple = triples[cur_line];
        // 遇到函数时， 处理
        if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
            ++cur_line;
            std::string func_name = triples.getFuncName(triple.e1);

            std::map<int, bool> visited_temp;
            visited_temp.clear();

            // 获取函数最外层块的id
            int block_id = triples[cur_line].e1.value;
            size_t stack_size = 16;
            // 处理直到block结束
            while (cur_line < triples.size() &&
                !(triples[cur_line].cmd == TCmd.blke &&
                    triples[cur_line].e1.value == block_id)) {

                Triples::Triple& cur_trip = triples[cur_line];

                if (cur_trip.cmd == TCmd.var) {
                    // 如果是变量声明，开对应空间
                    stack_size += cur_trip.e2.value * 4;
                } else {
                    // 对于临时变量， 开对应空间
                    auto changeStackSize = [&](const Triples::TripleValue& tv) {
                        if (tv.type == TTT.temp && !visited_temp[tv.value]) {
                            stack_size += 4;
                            visited_temp[tv.value] = true;
                        }
                        };

                    changeStackSize(cur_trip.e1);
                    changeStackSize(cur_trip.e2);
                    changeStackSize(cur_trip.to);
                }
                ++cur_line;
            }
            // 保存函数对应栈帧大小
            func_size[func_name] = stack_size;

            // It will stop at the end of block
            // No need to skip this line
        }

        // To next line
        ++cur_line;
    }
}
void StackRiscVGenerator::getTempVarType(Triples& triples) {
    size_t cur_line = 0;
    while (cur_line < triples.size()) {
        ++cur_line;
    }
}
RVOperand StackRiscVGenerator::getTempOpr(Triples& triples, int temp_id) {
    return make_stack(RVRegs::s0, -((cur_stacks.top().size() - cur_smallest_temp + temp_id) * 4 + 16));
}
RVOperand StackRiscVGenerator::getVarOpr(Triples& triples, const std::string& var_name) {
    auto& cur_stack = cur_stacks.top();

    int count = 0;
    for (auto& [name, num] : cur_stack) {
        if (name == var_name) {
            return make_stack(RVRegs::s0, -(count * 4 + 16));
        }
        count += num;
    }

    auto& cur_args = triples.func_params[cur_func_name];
    int int_count = 0, float_count = 0;
    for (auto& [name, type] : cur_args) {
        if (type == 1 || type == 3 || type == 4) ++int_count;
        else if (type == 2) ++float_count;
    }

    for (size_t i = cur_args.size() - 1; i >= 0; --i) {
        std::string& name = cur_args[i].first;
        int type = cur_args[i].second;

        if (type == 1 || type == 3 || type == 4) {
            if (name == var_name) {
                if (int_count <= 6) return make_areg(int_count - 1);
                else return make_stack(RVRegs::s0, (cur_args.size() - i + 1) * 4);
            }
            --int_count;
        } else if (type == 2) {
            if (name == var_name) {
                if (float_count <= 6) return make_sreg(float_count - 1);
                else return make_stack(RVRegs::s0, (cur_args.size() - i + 1) * 4);
            }
        }
    }

    panic("Bad var name");
    return make_imm(0);
}

void StackRiscVGenerator::genLoad(Triples& triples, Triples::Triple& triple) {
    auto& e1 = triple.e1;
    auto& to = triple.to;

    instrs.push_back(new RVMem(RVOp::LW, make_reg(a5), getVarOpr(triples, triples.getVarName(e1))));
    instrs.push_back(new RVMem(RVOp::SW, getTempOpr(triples, to.value), make_reg(a5)));

    return;
}

void StackRiscVGenerator::genStore(Triples& triples, Triples::Triple& triple) {
    auto& e1 = triple.e1;
    auto& to = triple.to;

    instrs.push_back(new RVMem(RVOp::LW, make_reg(a5), getTempOpr(triples, e1.value)));
    instrs.push_back(new RVMem(RVOp::SW, getVarOpr(triples, triples.getVarName(to)), make_reg(a5)));

    return;
}


void StackRiscVGenerator::genCall(Triples& triples, Triples::Triple& triple) {
    std::string func_name = triples.getFuncName(triple.e1);

    // 特判putf
    if (func_name == "putf") {
        genPutf(triples, triple);
        return;
    }

    // 处理非putf的一般函数     TODO---
    int int_count = 0, float_count = 0;
    for (auto cur_arg = triple.e2.added; cur_arg; cur_arg = cur_arg->added) {
        if (cur_arg->type == TTT.dimd) {
            if (int_count < 8) {
                instrs.push_back(new RVMem(RVOp::LI, make_areg(int_count), make_imm(cur_arg->value)));
            } else {
                panic("Push");
            }
            ++int_count;
        } else if (cur_arg->type == TTT.fimd) {
            size_t label = simm_table[cur_arg->value];
            if (float_count < 8) {
                instrs.push_back(new RVMem(RVOp::FLW, make_sreg(5), make_addr(".LC" + std::to_string(label))));
                // instrs.push_back(new RVConvert(RVOp::FCVTDS, make_sreg(5), make_sreg(5)));
                instrs.push_back(new RVMov(RVOp::FMVS, make_sreg(float_count), make_sreg(5)));
            } else {
                panic("Push");
            }
            ++float_count;
        } else if (cur_arg->type == TTT.temp) {
            int temp_type = triples.getTempType(cur_arg->value);
            if (temp_type == 1) {
                // int
                if (int_count < 8) {
                    instrs.push_back(new RVMem(RVOp::LW, make_areg(int_count), getTempOpr(triples, cur_arg->value)));
                } else {
                    panic("Push");
                }
                ++int_count;
            } else if (temp_type == 2) {
                // float
                if (float_count < 8) {
                    instrs.push_back(new RVMem(RVOp::FLW, make_sreg(5), getTempOpr(triples, cur_arg->value)));
                    instrs.push_back(new RVMov(RVOp::FMVS, make_sreg(float_count), make_sreg(5)));
                } else {
                    panic("Push");
                }
                ++float_count;
            } else {
                panic("bad temp type");
            }
        } else if (cur_arg->type == TTT.addr) {
            panic("TODO!");
        } else {
            panic("Error on call's args");
        }
    }

    instrs.push_back(new RVCall(func_name));
}

/// <summary>
/// 特判：：生成putf函数 TODO->传入变量
/// </summary>
/// <param name="triples"></param>
/// <param name="triple"></param>
void StackRiscVGenerator::genPutf(Triples& triples, Triples::Triple& triple) {
    int args_count = 0;
    for (auto cur_arg = triple.e2.added; cur_arg; cur_arg = cur_arg->added) {
        if (cur_arg->type == TTT.dimd) {
            if (args_count < 8){
                instrs.push_back(new RVMem(RVOp::LI, make_areg(args_count), make_imm(cur_arg->value)));
            } else
                panic("Push");
        } else if (cur_arg->type == TTT.fimd) {
            size_t label = putf_simm_table[cur_arg->value];
            if (args_count < 8) {
                instrs.push_back(new RVMem(RVOp::FLD, make_sreg(5), make_addr(".LC" + std::to_string(label))));
                // instrs.push_back(new RVConvert(RVOp::FCVTDS, make_sreg(5), make_sreg(5)));
                instrs.push_back(new RVMov(RVOp::FMVXD, make_areg(args_count), make_sreg(5)));
            } else
                panic("Push");
        } else if (cur_arg->type == TTT.str) {
            size_t str_label = string_table[triples.getValueString(*cur_arg)];
            if (args_count < 8) {
                instrs.push_back(new RVMem(RVOp::LSTR, make_areg(args_count), make_addr("STR" + std::to_string(str_label))));
            } else
                panic("Push");
        } else if (cur_arg->type == TTT.temp) {
            int temp_type = triples.getTempType(cur_arg->value);
            if (temp_type == 1) {
                // int
                if (args_count < 8) {
                    instrs.push_back(new RVMem(RVOp::LW, make_areg(args_count), getTempOpr(triples, cur_arg->value)));
                } else {
                    panic("Push");
                }
            } else if (temp_type == 2) {
                // float
                if (args_count < 8) {
                    instrs.push_back(new RVMem(RVOp::FLW, make_sreg(5), getTempOpr(triples, cur_arg->value)));
                    instrs.push_back(new RVConvert(RVOp::FCVTDS, make_sreg(5), make_sreg(5)));
                    instrs.push_back(new RVMov(RVOp::FMVXD, make_areg(args_count), make_sreg(5)));
                } else {
                    panic("Push");
                }
            } else {
                printf("value: %d, type %d\n", cur_arg->value, temp_type);
                panic("bad temp type");
            }
        } else {
            panic("Error on putf's args");
        }
        ++args_count;
    }
    instrs.push_back(new RVCall("putf"));

    // panic("Pop");
}
void StackRiscVGenerator::genTag(Triples& triples, Triples::Triple& triple) {
    if (triple.e1.type == TTT.func) {
        instrs.push_back(new RVTag(triples.getFuncName(triple.e1)));
    } else {
        instrs.push_back(new RVTag(triples.getLabelName(triple.e1)));
    }
}
void StackRiscVGenerator::genStack(Triples& triples, Triples::Triple& triple, size_t index) {
    int stack_size = func_size[cur_func_name];
    instrs.push_back(new RVArith(RVOp::ADD, make_reg(RVRegs::sp), make_reg(RVRegs::sp), make_imm(0 - stack_size)));
    instrs.push_back(new RVMem(RVOp::SD, make_reg(RVRegs::ra), stack_size - 8));
    instrs.push_back(new RVMem(RVOp::SD, make_reg(RVRegs::s0), stack_size - 16));
    instrs.push_back(new RVArith(RVOp::ADD, make_reg(RVRegs::s0), make_reg(RVRegs::sp), make_imm(stack_size)));

    std::vector<std::pair<std::string, int>> stack_info;

    int block_id = triples[index + 1].e1.value;
    for (size_t cur_line = index + 2; cur_line < triples.size(); ++cur_line) {
        Triples::Triple& t = triples[cur_line];
        if (t.cmd == TCmd.blke && t.e1.value == block_id) break;

        if (cur_smallest_temp != -1 && t.e1.type == TTT.temp) cur_smallest_temp = t.e1.value;
        if (cur_smallest_temp != -1 && t.e2.type == TTT.temp) cur_smallest_temp = t.e2.value;
        if (cur_smallest_temp != -1 && t.to.type == TTT.temp) cur_smallest_temp = t.to.value;

        if (t.cmd != TCmd.var) continue;
        stack_info.push_back({ triples.getVarName(t.e1), t.e2.value });
    }

    cur_stacks.push(stack_info);
}
void StackRiscVGenerator::genReturn(Triples& triples, Triples::Triple& triple) {
    int stack_size = func_size[cur_func_name];
    instrs.push_back(new RVMov(RVOp::MV, make_areg(0), make_areg(5)));
    instrs.push_back(new RVMem(RVOp::LD, make_reg(RVRegs::ra), stack_size - 8));
    instrs.push_back(new RVMem(RVOp::LD, make_reg(RVRegs::s0), stack_size - 16));
    instrs.push_back(new RVArith(RVOp::ADD, make_reg(RVRegs::sp), make_reg(RVRegs::sp), make_imm(stack_size)));
    instrs.push_back(new RVJump(RVOp::JR, make_reg(RVRegs::ra)));
    return;
}
/// <summary>   
/// 生成所有float与str常量
/// </summary>
void StackRiscVGenerator::genAllStrsFloats() {
    // 写入float常量
    for (auto [value, index] : simm_table) {
        instrs.push_back(new RVTag(".LC" + std::to_string(index)));
        instrs.push_back(new RVword(value));
    }
    // 写入str常量
    for (auto [value, index] : string_table) {
        instrs.push_back(new RVTag("STR" + std::to_string(index)));
        instrs.push_back(new RVstring(value));
    }
    // 写入用于传入putf的float常量，采用double形式
    for (auto [value, index] : putf_simm_table) {
        instrs.push_back(new RVTag(".LC" + std::to_string(index)));
        float v32 = *(float*)(&value);
        double v64 = (double)v32;
        union {
            double d;
            uint64_t u64;
        } v;
        v.d = v64;
        uint32_t lo = (uint32_t)(v.u64 >> 32);
        uint32_t hi = (uint32_t)(v.u64 & 0xFFFFFFFF);
        instrs.push_back(new RVword(hi));
        instrs.push_back(new RVword(lo));
    }
}
void StackRiscVGenerator::genMove(Triples& triples, Triples::Triple& triple) {
    Triples::TripleValue &tv = triple.e1, &to = triple.to;

    RVOperand dst;
    if (to.type == TTT.value) {
        dst = getVarOpr(triples, triples.getVarName(to));
    } else if (to.type == TTT.temp) {
        dst = getTempOpr(triples, to.value);
    }

    if (tv.type == TTT.dimd) {
        instrs.push_back(new RVMem(RVOp::LI, make_areg(5), make_imm(tv.value)));
        instrs.push_back(new RVMem(RVOp::SW, dst, make_areg(5)));
    } else if (tv.type == TTT.fimd) {
        assert(simm_table.find(tv.value) != simm_table.end());
        int label = simm_table[tv.value];
        instrs.push_back(new RVMem(RVOp::FLW, make_sreg(5), make_addr(".LC" + std::to_string(label))));
        instrs.push_back(new RVMem(RVOp::FSW, dst, make_sreg(5)));
    } else {
        panic("bad triplevalue type");
    }
}

void StackRiscVGenerator::generate(Triples& triples, bool optimize_flag) {
    if (optimize_flag) {
        panic("TODO: StackRiscVGenerator::generate: optimize");
    }

    createTable(triples);
    calculateSize(triples);
    getTempVarType(triples);

    for (auto [key, value] : func_size) {
        printf("%s: %u\n", key.c_str(), value);
    }
    for (size_t index = 0; index < triples.size(); ++index) {
        Triples::Triple& cur_triple = triples[index];

        switch (cur_triple.cmd) {
        case TCmd.add:
        case TCmd.sub:
        case TCmd.mul:
        case TCmd.div:
        case TCmd.mod:
            genArith(triples, cur_triple);
            break;

        case TCmd.load:
            genLoad(triples, cur_triple);
            break;
        case TCmd.store:
            genStore(triples, cur_triple);
            break;

        case TCmd.call:
            genCall(triples, cur_triple);
            break;

        case TCmd.mov:
            genMove(triples, cur_triple);
            break;

        case TCmd.tag:
            genTag(triples, cur_triple);
            if (cur_triple.e1.type == TTT.func) {
                cur_func_name = triples.getFuncName(cur_triple.e1);
            }
            break;

        case TCmd.blkb:
            cur_blocks.push(cur_triple.e1.value);
            if (cur_blocks.size() == 1) {
                // create new stack
                genStack(triples, cur_triple, index);
            }
            break;
        case TCmd.blke:
            cur_blocks.pop();
            if (cur_blocks.size() == 0) {
                cur_stacks.pop();
                cur_smallest_temp = -1;
                genReturn(triples, cur_triple);
            }
            break;


        default:
            // panic("Error");
            break;
        }
    }

    genAllStrsFloats();

    for (auto e : instrs) {
        std::cout << e->toASM();
    }
    panic("TODO!!!!!!!");
    return;
}