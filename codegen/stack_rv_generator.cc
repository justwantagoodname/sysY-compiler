#include "codegen/stack_rv_generator.h"

StackRiscVGenerator::StackRiscVGenerator() : simm_count(0), string_count(0) {
    instrs.clear();
    simm_table.clear();
    string_table.clear();
    func_size.clear();

    tempvar_type.clear();
}

void StackRiscVGenerator::genArith(Triples& triples, Triples::Triple& triple) {
    panic("TODO!");
}

void StackRiscVGenerator::createTable(Triples &triples) {
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
        bool is_putf = (triples.getFuncName(triple.e1) == "putf");

        // WTF is this genius design...
        for (auto t = triple.e2.added; t; t = t->added) {
            if (t->type == Triples::TT.fimd) {
                int value = t->value;
                if (!is_putf && simm_table.find(value) == simm_table.end()) {
                    simm_table[value] = simm_count++;
                }
                if (is_putf && putf_simm_table.find(value) == putf_simm_table.end()) {
                    putf_simm_table[value] = simm_count++;
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
void StackRiscVGenerator::calculateSize(Triples& triples) {
    size_t cur_line = 0;
    while (cur_line < triples.size()) {
        Triples::Triple &triple = triples[cur_line];
        if (triple.cmd == Triples::Cmd.tag && triple.e1.type == Triples::TT.func) {
            ++cur_line;
            std::string func_name = triples.getFuncName(triple.e1);
            
            std::map<int, bool> visited_temp;
            visited_temp.clear();
            
            int block_id = triples[cur_line].e1.value;
            size_t stack_size = 16;
            while (cur_line < triples.size() && 
                   !(triples[cur_line].cmd == Triples::Cmd.blke && 
                   triples[cur_line].e1.value == block_id)) {

                Triples::Triple &cur_trip = triples[cur_line];

                if (cur_trip.cmd == Triples::Cmd.var) {
                    stack_size += cur_trip.e2.value * 4;
                } else {
                    auto changeStackSize = [&](const Triples::TripleValue& tv) {
                        if (tv.type == Triples::TT.temp && !visited_temp[tv.value]) {
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

void StackRiscVGenerator::genLoad(Triples& triples, Triples::Triple& triple) {
    panic("TODO!: Register allocation");
    return;
}

void StackRiscVGenerator::genCall(Triples& triples, Triples::Triple& triple) { 
    std::string func_name = triples.getFuncName(triple.e1);
    
    if (func_name == "putf") {
        genPutf(triples, triple);
        return;
    }

    int int_count = 0, float_count = 0;
    for (auto cur_arg = triple.e2.added; cur_arg; cur_arg = cur_arg->added) {
        
    }
}
void StackRiscVGenerator::genPutf(Triples& triples, Triples::Triple& triple) {
    int args_count = 0;
    for (auto cur_arg = triple.e2.added; cur_arg; cur_arg = cur_arg->added) {
        if (cur_arg->type == Triples::TT.dimd) {
            if (args_count < 8)
                instrs.push_back(new RVMem(RVOp::LI, make_areg(args_count), make_imm(cur_arg->value)));
            else
                panic("Push");
        } else if (cur_arg->type == Triples::TT.fimd) {
            size_t label = putf_simm_table[cur_arg->value];
            if (args_count < 8) {
                instrs.push_back(new RVMem(RVOp::FLW, make_sreg(5), make_simm(cur_arg->value)));
                instrs.push_back(new RVMem(RVOp::FCVTDS, make_sreg(5), make_sreg(5)));
                instrs.push_back(new RVMov(RVOp::FMVXD, make_areg(args_count), make_sreg(5)));
            }
            else
                panic("Push");
        } else if (cur_arg->type == Triples::TT.str) {
            size_t str_label = string_table[triples.getValueString(*cur_arg)];
            if (args_count < 8) {
                instrs.push_back(new RVMem(RVOp::LSTR, make_areg(args_count), make_addr("STR" + std::to_string(str_label))));
            }
            else
                panic("Push");
        } else if (cur_arg->type == Triples::TT.temp) {
            panic("TODO!!!");
        } else {
            panic("Error on putf's args");
        }
    }
    instrs.push_back(new RVCall("putf"));

    // panic("Pop");
}
void StackRiscVGenerator::genTag(Triples& triples, Triples::Triple& triple) {
    if (triple.e1.type == Triples::TT.func) {
        instrs.push_back(new RVTag(triples.getFuncName(triple.e1)));
    } else {
        instrs.push_back(new RVTag(triples.getLabelName(triple.e1)));
    }
}
void StackRiscVGenerator::genStack(Triples& triples, Triples::Triple& triple) {
    panic("TODO");
    return;
}
void StackRiscVGenerator::genAllStrsFloats() {
    for (auto [value, index] : simm_table) {
        instrs.push_back(new RVTag(".LC" + std::to_string(index)));
        instrs.push_back(new RVword(value));
    }
    for (auto [value, index] : string_table) {
        instrs.push_back(new RVTag("STR" + std::to_string(index)));
        instrs.push_back(new RVstring(value));
    }
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

void StackRiscVGenerator::generate(Triples &triples, bool optimize_flag) {
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
        Triples::Triple &cur_triple = triples[index];

        switch (cur_triple.cmd) {
            case Triples::Cmd.add:
            case Triples::Cmd.sub:
            case Triples::Cmd.mul:
            case Triples::Cmd.div:
            case Triples::Cmd.mod:
                // genArith(triples, cur_triple);
                break;

            case Triples::Cmd.load:
                // genLoad(triples, cur_triple);
                break;
            
            case Triples::Cmd.call:
                genCall(triples, cur_triple);
                break;
            
            case Triples::Cmd.tag:
                genTag(triples, cur_triple);
                if (cur_triple.e1.type == Triples::TT.func) {
                    // genStack(triples, cur_triple);
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