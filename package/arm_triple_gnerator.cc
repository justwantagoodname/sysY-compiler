#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    auto& TTT = Triples::TT;
    auto& TCmd = Triples::Cmd;

    ArmTripleGenerator::ArmTripleGenerator()
    {
    }

    void TriplesArmGenerator::ArmTripleGenerator::getStackPlace(Triples& triples)
    {
        value_addr.clear();
        value_addr.resize(triples.value_pointer.size());

        temp_addr.clear();
        temp_addr.resize(triples.temp_count);

        func_stack_size.clear();
        func_stack_size.resize(triples.funcid_params.size());

        int now_func_block_id = -1;
        int now_func_id = -1;
        int stack_size = 0;
        for (int i = 0; i < triples.size(); ++i) {
            Triples::Triple& triple = triples[i];
            // 是函数， 进入栈分析
            if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
                //printf("into func\n");
                // 获得func 编号 与 block 编号
                now_func_block_id = triples[i + 1].e1.value;
                now_func_id = triple.e1.value;
                // 分配函数参数地址到栈上
                auto& params = triples.funcid_params[now_func_id];
                for (int j = 1; j < params.size(); ++j) {
                    value_addr[params[j].first] = Addr(AB.s0, j);
                }
                stack_size = params.size() - 1;
            }

            // 结束函数，结束栈分析
            if (triple.cmd == TCmd.blke && triple.e1.value == now_func_block_id) {
                //printf("out func\n");
                // 退出当前函数分析
                now_func_block_id = -1;
                // 存储函数栈大小
                func_stack_size[now_func_id] = stack_size;
                stack_size = 0;
                now_func_id = -1;
            }

            // 为了（只是为了）方便，混合存储temp和value
            // 如果是var，分配栈
            if (triple.cmd == TCmd.var) {
                //printf("var def\n");
                value_addr[triple.e1.value] = Addr(AB.s0, stack_size);
                stack_size += triple.e2.value;
            }

            // 如果是新的temp，分配栈
            if (triple.to.type == TTT.temp
                && temp_addr[triple.to.value].base == AB.null) {
                //printf("temp def\n");
                temp_addr[triple.to.value] = Addr(AB.s0, stack_size);
                stack_size += 1;
            }

        }
    }

    void ArmTripleGenerator::generator(Triples& triples, bool O)
    {
        getStackPlace(triples);
    }

    void ArmTripleGenerator::printAddrs(Triples& triples)
    {

        printf("函数栈分配：\n");
        for (int i = 0; i < func_stack_size.size(); ++i) {
            string name = triples.getFuncName({ i , TTT.func });
            printf("%d@%s : %d\n", i, name.c_str(), func_stack_size[i] * 4);
        }

        printf("\n变量地址分配：\n");
        for (int i = 0; i < value_addr.size(); ++i) {
            string name = triples.getVarName({ i, TTT.value });
            printf("%s(%d) : %s\n", name.c_str(), i, value_addr[i].toString().c_str());
        }

        printf("\n%d 个临时变量地址分配：\n", temp_addr.size());
        for (int i = 0; i < temp_addr.size(); ++i) {
            printf("T%d : %s\n", i, temp_addr[i].toString().c_str());
        }
    }

    std::string Addr::toString()
    {
        std::string names[] = {
            "null",
            "reg", // 寄存器上
            "r0", "r1", "r2", "r3", // 参数寄存器
            "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", // 通用寄存器
            "r12", // r12, ip, scratch寄存器, 可挪用, 而且不用恢复, 挪去做s0了（
            "sp", // r13, 栈寄存器
            "lr", // r14, 中断寄存器
            "pc", // r15, pc寄存器
            "imd", "dimd", //立即数, 双字立即数
        };

        if (base == AB.null) {
            return "null";
        } else if (base == AB.reg) {
            return names[value];
        } else if (base == AB.imd) {
            return "#" + std::to_string(value);
        } else if (base == AB.tag) {
            return tag;
        } else if (base == AB.up_tag) {
            return tag;
        } else if (base == AB.low_tag) {
            return tag;
        } else {
            return "{" + names[base] + ", #" + std::to_string(value * 4) + "}";
        }
    }
}