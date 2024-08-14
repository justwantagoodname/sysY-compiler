#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    auto& TTT = Triples::TT;
    auto& TCmd = Triples::Cmd;

    Addr ArmTripleGenerator::loadInt(const Addr& addr)
    {
        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 已分配给通用寄存器，直接返回
            return addr;
        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 已分配给浮点寄存器，移动到整形寄存器
            Addr ftemp = getEmptyFloatTempReg();
            Addr temp = getEmptyIntTempReg();

            instrs.push_back({ ACmd.vcvtf2d, ftemp, addr });
            instrs.push_back({ ACmd.vmov, temp, ftemp });

            setTempRegState(temp, true); // 标记占用
            return temp;

        } else if (addr.value >= AB.r0 && addr.value <= AB.pc) {
            // 在栈上（以某个寄存器为基偏移）， 读取
            Addr temp = getEmptyIntTempReg();

            instrs.push_back({ ACmd.ldr, temp, addr });

            setTempRegState(temp, true); // 标记占用
            return temp;

        } else if (addr.base == AB.imd) {
            // 是立即数，读取
            Addr temp = getEmptyIntTempReg();

            instrs.push_back({ ACmd.mov, temp, addr });

            setTempRegState(temp, true); // 标记占用
            return temp;

        } else if (addr.base == AB.dimd) {
            // 是浮点立即数，转换为整形读取
            Addr temp = getEmptyIntTempReg();

            int v = (int)*(float*)(&addr.value);

            instrs.push_back({ ACmd.mov, temp, {v} });

            setTempRegState(temp, true); // 标记占用
            return temp;
        } else {
            panic("load bad addr ( like tag ) to int reg!");
        }
    }

    Addr ArmTripleGenerator::loadFloat(const Addr& addr)
    {
        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 已分配给通用寄存器，移动到浮点寄存器
            Addr ftemp = getEmptyFloatTempReg();

            instrs.push_back({ ACmd.vmov, ftemp, addr });
            instrs.push_back({ ACmd.vcvtd2f, ftemp, ftemp });

            setTempRegState(ftemp, true); // 标记占用
            return ftemp;

        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 已分配给浮点寄存器，直接返回
            return addr;

        } else if (addr.value >= AB.r0 && addr.value <= AB.pc) {
            // 在栈上（以某个寄存器为基偏移）， 读取
            Addr ftemp = getEmptyFloatTempReg();

            instrs.push_back({ ACmd.vldr, ftemp, addr });

            setTempRegState(ftemp, true); // 标记占用
            return ftemp;

        } else if (addr.base == AB.imd) {
            // 是立即数，读取
            // TODO
            float v = addr.value;

        } else if (addr.base == AB.dimd) {
            // 是浮点立即数，读取
            // TODO
            float v = *(float*)(&addr.value);

        } else {
            panic("load bad addr ( like tag ) to int reg!");
        }
    }

    void ArmTripleGenerator::storeInt(const Addr& addr, const Addr& reg)
    {
        assert(reg.base == AB.reg && (reg.value >= AB.r0 && reg.value <= AB.pc));

        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 存储到通用寄存器，判断位置，不相同mov，相同跳过
            if (addr.value == reg.value) // 相同，ret, 不需要释放临时寄存器
                return;
            else {
                instrs.push_back({ ACmd.mov, addr, reg });
            }

        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 存储到浮点寄存器，转换类型并移动

            instrs.push_back({ ACmd.vmov, addr, reg });
            instrs.push_back({ ACmd.vcvtd2f, addr, addr });

        } else if (addr.value >= AB.r0 && addr.value <= AB.pc) {
            // 在栈上（以某个寄存器为基偏移）， 存储
            instrs.push_back({ ACmd.str, reg, addr });

        } else {
            panic("save to bad addr ( like tag ) to int reg!");
        }

        // 寄存器是临时寄存器，释放
        for (auto& r : int_temp_reg) {
            if (r.first == addr.value) {
                r.second = false;
                return;
            }
        }
    }

    void ArmTripleGenerator::storeFloat(const Addr& addr, const Addr& reg)
    {
        assert(reg.base == AB.reg && (reg.value >= AB.fa0 && reg.value <= AB.fa15));

        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 存储到通用寄存器，转换类型并移动
            Addr ftemp = getEmptyFloatTempReg();

            instrs.push_back({ ACmd.vcvtf2d, ftemp, reg });
            instrs.push_back({ ACmd.vmov, addr, ftemp });
            

        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 存储到浮点寄存器，判断位置，不相同mov，相同跳过
            if (addr.value == reg.value) // 相同，ret, 不需要释放临时寄存器
                return;
            else {
                instrs.push_back({ ACmd.vmov, addr, reg });
            }

        } else if (addr.value >= AB.r0 && addr.value <= AB.pc) {
            // 在栈上（以某个寄存器为基偏移）， 存储
            instrs.push_back({ ACmd.vstr, reg, addr });

        } else {
            panic("save to bad addr ( like tag ) to float reg!");
        }

        // 寄存器是临时寄存器，释放
        for (auto& r : float_temp_reg) {
            if (r.first == addr.value) {
                r.second = false;
                return;
            }
        }
    }

    Addr ArmTripleGenerator::getEmptyIntTempReg()
    {
        for (auto& r : int_temp_reg) {
            if (!r.second)
                return r.first;
        }
        panic("try to get more int temp reg");
    }

    Addr ArmTripleGenerator::getEmptyFloatTempReg()
    {
        for (auto& r : float_temp_reg) {
            if (!r.second)
                return r.first;
        }
        panic("try to get more float temp reg");
    }

    void ArmTripleGenerator::setTempRegState(const Addr& addr, bool state)
    {
        if (addr.base != AB.reg)
            panic("set not reg temp state");
        for (auto& r : int_temp_reg) {
            if (r.first == addr.value) {
                r.second = state;
                return;
            }
        }
        for (auto& r : float_temp_reg) {
            if (r.first == addr.value) {
                r.second = state;
                return;
            }
        }
        panic("set bad reg temp state");
    }

    ArmTripleGenerator::ArmTripleGenerator() {}

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

    void ArmTripleGenerator::generate(Triples& triples, bool O)
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

            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", // 浮点寄存器
            "s8", "s9", "s10", "s11", "s12",
            "s13", "s14", "s15",

            "imd", "dimd",
            "tag", // 是tag
            "up_tag", "low_tag" //高位读取， 低位读取
        };

        if (base == AB.null) {
            return "null";
        } else if (base == AB.reg) {
            return names[value];
        } else if (base == AB.imd) {
            return "#" + std::to_string(value);
        } else if (base == AB.tag) {
            return tag;
        } else if (base == AB.low_tag) {
            return ":lower16:" + tag;
        } else if (base == AB.up_tag) {
            return ":upper16:" + tag;
        } else {
            return "[" + names[base] + ", #" + std::to_string(value * 4) + "]";
        }
    }
}