#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {

    Addr ArmTripleGenerator::loadInt(const Addr& addr, int stack_type)
    {
//        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
//            // 已分配给通用寄存器，直接返回
//            return addr;
//        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
//            // 已分配给浮点寄存器，移动到整形寄存器
//            Addr ftemp = getEmptyFloatTempReg();
//            Addr temp = getEmptyIntTempReg();
//
//            instrs.push_back({ ACmd.vcvtf2d, ftemp, addr });
//            instrs.push_back({ ACmd.vmov, temp, ftemp });
//
//            setTempRegState(temp, true); // 标记占用
//            return temp;
//
//        } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
//            // 在栈上（以某个寄存器为基偏移）， 读取
//            if (stack_type != 2) {
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.ldr, temp, addr });
//
//                setTempRegState(temp, true); // 标记占用
//                return temp;
//            } else {
//                Addr ftemp = getEmptyFloatTempReg();
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.vldr, ftemp, addr });
//                instrs.push_back({ ACmd.vcvtf2d, ftemp, ftemp });
//                instrs.push_back({ ACmd.vmov, temp, ftemp });
//
//                setTempRegState(temp, true); // 标记占用
//                return temp;
//            }
//
//        } else if (addr.base == AB.imd) {
//            // 是立即数，读取
//            Addr temp = getEmptyIntTempReg();
//
//            //TOnnDO 限定范围
//            instrs.push_back({ ACmd.mov, temp, addr });
//
//            setTempRegState(temp, true); // 标记占用
//            return temp;
//
//        } else if (addr.base == AB.dimd) {
//            // 是浮点立即数，转换为整形读取
//            Addr temp = getEmptyIntTempReg();
//
//            int v = (int)*(float*)(&addr.value);
//
//            //TOnnDO 限定范围
//            instrs.push_back({ ACmd.mov, temp, {v} });
//
//            setTempRegState(temp, true); // 标记占用
//            return temp;
//        } else if (addr.base == AB.tag) {
//            // 全局变量
//            if (stack_type != 2) {
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
//                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
//                instrs.push_back({ ACmd.ldr,  temp, { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });
//
//                setTempRegState(temp, true); // 标记占用
//                return temp;
//            } else {
//                Addr ftemp = getEmptyFloatTempReg();
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
//                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
//                instrs.push_back({ ACmd.vldr, ftemp, { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });
//                instrs.push_back({ ACmd.vcvtf2d, ftemp, ftemp });
//                instrs.push_back({ ACmd.vmov, temp, ftemp });
//
//                setTempRegState(temp, true); // 标记占用
//                return temp;
//            }
//
//        } else {
//            panic("load bad addr ( like tag ) to int reg!");
//        }

        Addr temp = getEmptyIntTempReg();
        loadInt(addr, temp, stack_type);
        setTempRegState(temp, true);
        return temp;
    }

    Addr ArmTripleGenerator::loadFloat(const Addr& addr, int stack_type)
    {
//        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
//            // 已分配给通用寄存器，移动到浮点寄存器
//            Addr ftemp = getEmptyFloatTempReg();
//
//            instrs.push_back({ ACmd.vmov, ftemp, addr });
//            instrs.push_back({ ACmd.vcvtd2f, ftemp, ftemp });
//
//            setTempRegState(ftemp, true); // 标记占用
//            return ftemp;
//
//        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
//            // 已分配给浮点寄存器，直接返回
//            return addr;
//
//        } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
//            // 在栈上（以某个寄存器为基偏移）， 读取
//            if (stack_type != 2) {
//                Addr ftemp = getEmptyFloatTempReg();
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.ldr, temp, addr });
//                instrs.push_back({ ACmd.vmov, ftemp, temp });
//                instrs.push_back({ ACmd.vcvtd2f, ftemp, ftemp });
//
//                setTempRegState(ftemp, true); // 标记占用
//                return ftemp;
//            } else {
//                Addr ftemp = getEmptyFloatTempReg();
//
//                instrs.push_back({ ACmd.vldr, ftemp, addr });
//
//                setTempRegState(ftemp, true); // 标记占用
//                return ftemp;
//            }
//        } else if (addr.base == AB.imd || addr.base == AB.dimd) {
//            // 是立即数，读取，整形转换为浮点读取
//            float v;
//            int d;
//            if (addr.base == AB.imd) {
//                v = addr.value;
//            } else if (addr.base == AB.dimd) {
//                v = *(float*)(&addr.value);
//            }
//            d = *(int*)(&v);
//            Addr ftemp = getEmptyFloatTempReg();
//            Addr temp = getEmptyIntTempReg();
//            //加载立即数
//
//            instrs.push_back({ ACmd.movw, temp, d & 0xFFFF });
//            instrs.push_back({ ACmd.movt, temp, d >> 16 });
//            instrs.push_back({ ACmd.vmov, ftemp, temp });
//
//            setTempRegState(ftemp, true); // 标记占用
//            return ftemp;
//
//        } else if (addr.base == AB.tag) {
//            // 全局变量
//            if (stack_type != 2) {
//                Addr ftemp = getEmptyFloatTempReg();
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
//                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
//                instrs.push_back({ ACmd.ldr, temp,  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });
//                instrs.push_back({ ACmd.vmov, ftemp, temp });
//                instrs.push_back({ ACmd.vcvtd2f, ftemp, ftemp });
//
//                setTempRegState(ftemp, true); // 标记占用
//                return ftemp;
//            } else {
//                Addr ftemp = getEmptyFloatTempReg();
//                Addr temp = getEmptyIntTempReg();
//
//                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
//                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
//                instrs.push_back({ ACmd.vldr, ftemp,  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });
//
//                setTempRegState(ftemp, true); // 标记占用
//                return ftemp;
//            }
//        } else {
//            panic("load bad addr ( like tag ) to float reg!");
//        }
        Addr ftemp = getEmptyFloatTempReg();
        loadFloat(addr, ftemp, stack_type);
        setTempRegState(ftemp, true);
        return ftemp;

    }

    void ArmTripleGenerator::loadInt(const Addr &addr, const Addr &reg, int stack_type) {
        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 已分配给通用寄存器，直接返回
            if(reg.value == addr.value)
                return;
            else{
                instrs.push_back({ACmd.mov, reg, addr});
            }
        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 已分配给浮点寄存器，移动到整形寄存器
            Addr ftemp = getEmptyFloatTempReg();

            instrs.push_back({ ACmd.vcvtf2d, ftemp, addr });
            instrs.push_back({ ACmd.vmov, reg, ftemp });

        } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
            // 在栈上（以某个寄存器为基偏移）， 读取
            if (stack_type != 2) {
                instrs.push_back({ ACmd.ldr, reg, addr });

            } else {
                Addr ftemp = getEmptyFloatTempReg();

                instrs.push_back({ ACmd.vldr, ftemp, addr });
                instrs.push_back({ ACmd.vcvtf2d, ftemp, ftemp });
                instrs.push_back({ ACmd.vmov, reg, ftemp });

            }
            setTempRegState(addr, false);

        } else if (addr.base == AB.imd) {
            // 限定范围
            unsigned int d = addr.value;
            if(d < 0xFFFF)
                instrs.push_back({ ACmd.mov, reg, addr });
            else{
                instrs.push_back({ ACmd.movw, reg, d & 0xFFFF });
                instrs.push_back({ ACmd.movt, reg, d >> 16 });
            }

        } else if (addr.base == AB.dimd) {
            // 是浮点立即数，转换为整形读取
            unsigned int d = (int)*(float*)(&addr.value);

            // 限定范围
            if(d < 0xFFFF)
                instrs.push_back({ ACmd.mov, reg, addr });
            else {
                instrs.push_back({ACmd.movw, reg, d & 0xFFFF});
                instrs.push_back({ACmd.movt, reg, d >> 16});
            }
        } else if (addr.base == AB.tag) {
            // 全局变量
            if (stack_type != 2) {

                instrs.push_back({ ACmd.movw, reg, {AB.low_tag, addr.tag} });
                instrs.push_back({ ACmd.movt, reg, {AB.up_tag, addr.tag} });
                instrs.push_back({ ACmd.ldr,  reg, { (ADDRBASE::ADDRBASEENUM)reg.value, 0 } });

            } else {
                Addr ftemp = getEmptyFloatTempReg();

                instrs.push_back({ ACmd.movw, reg, {AB.low_tag, addr.tag} });
                instrs.push_back({ ACmd.movt, reg, {AB.up_tag, addr.tag} });
                instrs.push_back({ ACmd.vldr, ftemp, { (ADDRBASE::ADDRBASEENUM)reg.value, 0 } });
                instrs.push_back({ ACmd.vcvtf2d, ftemp, ftemp });
                instrs.push_back({ ACmd.vmov, reg, ftemp });

            }

        } else {
            panic("load bad addr ( like tag ) to int reg!");
        }
    }

    void ArmTripleGenerator::loadFloat(const Addr &addr, const Addr &reg, int stack_type) {
        if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
            // 已分配给通用寄存器，移动到浮点寄存器

            instrs.push_back({ ACmd.vmov, reg, addr });
            instrs.push_back({ ACmd.vcvtd2f, reg, reg });


        } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
            // 已分配给浮点寄存器，直接返回
            if(reg.value == addr.value)
                return;
            else{
                instrs.push_back({ACmd.vmov, reg, addr});
            }

        } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
            // 在栈上（以某个寄存器为基偏移）， 读取
            if (stack_type != 2) {
                Addr temp = getEmptyIntTempReg();

                instrs.push_back({ ACmd.ldr, temp, addr });
                instrs.push_back({ ACmd.vmov, reg, temp });
                instrs.push_back({ ACmd.vcvtd2f, reg, reg });

            } else {

                instrs.push_back({ ACmd.vldr, reg, addr });
            }
            setTempRegState(addr, false);

        } else if (addr.base == AB.imd || addr.base == AB.dimd) {
            // 是立即数，读取，整形转换为浮点读取
            float v;
            unsigned int d;
            if (addr.base == AB.imd) {
                v = addr.value;
            } else if (addr.base == AB.dimd) {
                v = *(float*)(&addr.value);
            }
            d = *(int*)(&v);
            Addr temp = getEmptyIntTempReg();
            //加载立即数

            instrs.push_back({ ACmd.movw, temp, d & 0xFFFF });
            instrs.push_back({ ACmd.movt, temp, d >> 16 });
            instrs.push_back({ ACmd.vmov, reg, temp });


        } else if (addr.base == AB.tag) {
            // 全局变量
            if (stack_type != 2) {
                Addr temp = getEmptyIntTempReg();

                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
                instrs.push_back({ ACmd.ldr, temp,  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });
                instrs.push_back({ ACmd.vmov, reg, temp });
                instrs.push_back({ ACmd.vcvtd2f, reg, reg });

            } else {
                Addr temp = getEmptyIntTempReg();

                instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
                instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
                instrs.push_back({ ACmd.vldr, reg,  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 } });

            }

        } else {
            panic("load bad addr ( like tag ) to float reg!");
        }
    }

    void ArmTripleGenerator::storeInt(const Addr& addr, const Addr& reg)
    {
        //assert(reg.base == AB.reg && (reg.value >= AB.r0 && reg.value <= AB.pc));
        if (reg.base == AB.reg && (reg.value >= AB.r0 && reg.value <= AB.pc)) {
            if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
                // 存储到通用寄存器，判断位置，不相同mov，相同跳过
                if (addr.value == reg.value) // 相同，ret, 不需要释放临时寄存器
                    return;
                else {
                    instrs.push_back({ ACmd.mov, addr, reg });
                }

            } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
                // 存储到浮点寄存器，报错
                panic("can not save int into float reg!");

            } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
                // 在栈上（以某个寄存器为基偏移）， 存储
                instrs.push_back({ ACmd.str, reg, addr });
                setTempRegState(addr, false);

            } else {
                printf("%s\n", addr.toString().c_str());
                panic("save to bad addr ( like tag ) from int reg!");
            }
        } else if (reg.base == AB.reg && (reg.value >= AB.fa0 && reg.value <= AB.fa15)) {
            if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
                // 存储到通用寄存器，转换类型并移动
                Addr ftemp = getEmptyFloatTempReg();

                instrs.push_back({ ACmd.vcvtf2d, ftemp, reg });
                instrs.push_back({ ACmd.vmov, addr, ftemp });


            } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
                // 存储到浮点寄存器，报错
                panic("can not save int into float reg!");

            } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
                // 在栈上（以某个寄存器为基偏移）， 类型转换， 存储
                Addr ftemp = getEmptyFloatTempReg();

                instrs.push_back({ ACmd.vcvtf2d, ftemp, reg });
                instrs.push_back({ ACmd.vstr, ftemp, addr });
                setTempRegState(addr, false);

            } else {
                printf("%s\n", addr.toString().c_str());
                panic("save to bad addr ( like tag ) from float reg!");
            }

        } else {
            panic("ERROR: not store data from reg!");
        }
        // 寄存器是临时寄存器，释放
        setTempRegState(reg, false);
    }

    void ArmTripleGenerator::storeFloat(const Addr& addr, const Addr& reg)
    {
        //assert(reg.base == AB.reg && (reg.value >= AB.fa0 && reg.value <= AB.fa15));
        if (reg.base == AB.reg && (reg.value >= AB.r0 && reg.value <= AB.pc)) {
            if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
                // 存储到通用寄存器，报错
                panic("can not save float into int reg!");

            } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
                // 存储到浮点寄存器，转换类型并存入

                instrs.push_back({ ACmd.vmov, addr, reg });
                instrs.push_back({ ACmd.vcvtd2f, addr, addr });

            } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
                // 在栈上（以某个寄存器为基偏移）， 存储
                Addr ftemp = getEmptyFloatTempReg();

                instrs.push_back({ ACmd.vmov, ftemp, reg });
                instrs.push_back({ ACmd.vcvtd2f, ftemp, ftemp });
                instrs.push_back({ ACmd.vstr, ftemp, addr });
                setTempRegState(addr, false);

            } else {
                printf("%s\n", addr.toString().c_str());
                panic("save to bad addr ( like tag ) from int reg!");
            }
        } else if (reg.base == AB.reg && (reg.value >= AB.fa0 && reg.value <= AB.fa15)) {
            if (addr.base == AB.reg && (addr.value >= AB.r0 && addr.value <= AB.pc)) {
                // 存储到通用寄存器，报错
                panic("can not save float into int reg!");
            } else if (addr.base == AB.reg && (addr.value >= AB.fa0 && addr.value <= AB.fa15)) {
                // 存储到浮点寄存器，判断位置，不相同mov，相同跳过
                if (addr.value == reg.value) // 相同，ret, 不需要释放临时寄存器
                    return;
                else {
                    instrs.push_back({ ACmd.vmov, addr, reg });
                }

            } else if (addr.base >= AB.r0 && addr.base <= AB.pc || addr.base == AB.reglsl_stack) {
                // 在栈上（以某个寄存器为基偏移）， 存储
                instrs.push_back({ ACmd.vstr, reg, addr });
//                assert(addr.value < 0xFFFF);
                setTempRegState(addr, false);

            } else {
                printf("%s\n", addr.toString().c_str());
                panic("save to bad addr ( like tag ) from float reg!");
            }

        } else {
            panic("ERROR: not store data from reg!");
        }

        // 寄存器是临时寄存器，释放
        setTempRegState(reg, false);
    }

    Addr ArmTripleGenerator::getEmptyIntTempReg()
    {
        for (auto& r : int_temp_reg) {
            if (!r.second)
                return { AB.reg, r.first };
        }
        panic("try to get more int temp reg");
    }

    Addr ArmTripleGenerator::getEmptyFloatTempReg()
    {
        for (auto& r : float_temp_reg) {
            if (!r.second)
                return { AB.reg, r.first };
        }
        panic("try to get more float temp reg");
    }

    int ArmTripleGenerator::setTempRegState(const Addr& _addr, bool state)
    {
        if(state) {
            printf("occupancy %s \n", _addr.toString().c_str());
        }
        else {
            printf("delete %s \n",  _addr.toString().c_str());
        }

        Addr addr = _addr;
        if (addr.base != AB.reg){
            if(addr.base >= AB.r0 && addr.base <= AB.fa15){
                addr = Addr(addr.base);
            } else if(addr.base == AB.reglsl_stack){
                setTempRegState({AB.reg, addr.tag[1]}, state);
                addr = {AB.reg, addr.tag[0]};
            }
            else
                panic("set not reg temp state");

        }
        for (auto& r : int_temp_reg) {
            if (r.first == addr.value) {
                r.second = state;
                return 1;
            }
        }
        for (auto& r : float_temp_reg) {
            if (r.first == addr.value) {
                r.second = state;
                return 1;
            }
        }
        return 0;
        //panic("set bad reg temp state");
    }

    Addr TriplesArmGenerator::ArmTripleGenerator::loadTripleValueAddr(const Triples& triples, const Triples::TripleValue& triple)
    {
        if (triple.type == TTT.dimd) {
            return triple.value;
        } else if (triple.type == TTT.fimd) {
            return { AB.imd, triple.value };
        } else if (triple.type == TTT.temp) {
            Addr addr = temp_addr[triple.value];
            if(addr.value < 1024|| addr.base < AB.r0 || addr.base > AB.pc)
                return addr;
            else {
                Addr t = loadInt(addr.value * 4);
                return { (ADDRBASE::ADDRBASEENUM)t.value, 0 };
            }
        } else if (triple.type == TTT.value) {
            if (triple.added == NULL) {
                if (value_addr[triple.value].base == AB.tag) {
                    Addr addr = value_addr[triple.value];
                    Addr temp = getEmptyIntTempReg();
                    instrs.push_back({ ACmd.movw, temp, {AB.low_tag, addr.tag} });
                    instrs.push_back({ ACmd.movt, temp, {AB.up_tag, addr.tag} });
                    return { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                } else {
                    Addr addr = value_addr[triple.value];
                    if(addr.value < 1024 || addr.base < AB.r0 || addr.base > AB.pc)
                        return addr;
                    else {
                        addr.value *= 4;
                        Addr temp = getEmptyIntTempReg();
                        loadInt(addr,temp);
                        return  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                    }
                }
            } else if (triple.added->type == TTT.dimd) {
                Addr addr = value_addr[triple.value];
                addr.value += triple.added->value;

                if(addr.value < 1024)
                    return addr;
                else {
                    addr.value *= 4;
                    Addr temp = getEmptyIntTempReg();
                    loadInt(addr,temp);
                    return  { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                }
            } else if (triple.added->type == TTT.temp || triple.added->type == TTT.value ) {
                Addr temp ;
                temp = loadInt(loadTripleValueAddr(triples, *triple.added));
                Addr lst = value_addr[triple.value];
                // 数组只能在栈上或全局
                assert(lst.base >= AB.r0 && lst.base <= AB.pc || lst.base == AB.tag);

                if (lst.base >= AB.r0 && lst.base <= AB.pc) {
                    // 栈上
                    instrs.push_back({ ACmd.lsls, temp, temp, 2 });
                    instrs.push_back({ ACmd.add, temp, lst.base, temp });
                    // 别忘了释放
                    if(lst.value < 1024)
                        return { (ADDRBASE::ADDRBASEENUM)temp.value, lst.value };
                    else {
                        Addr t = loadInt(lst.value * 4 );
                        instrs.push_back({ ACmd.add, temp, temp, t });
                        setTempRegState(t, false);
                        return { (ADDRBASE::ADDRBASEENUM)temp.value, 0 };
                    }
                } else if (lst.base == AB.tag) {
                    // 全局
                    Addr temp2 = getEmptyIntTempReg();
                    instrs.push_back({ ACmd.movw, temp2, {AB.low_tag, lst.tag} });
                    instrs.push_back({ ACmd.movt, temp2, {AB.up_tag, lst.tag} });

                    // 别忘了释放

                    return  { (ADDRBASE::ADDRBASEENUM)temp2.value, (ADDRBASE::ADDRBASEENUM)temp.value };
                }
            }
        } else if (triple.type == TTT.lamb) {
            return ".l" + std::to_string(triple.value);
        } else if (triple.type == TTT.func) {
            return triples.getFuncName(triple.value);
        }
        return AB.r0;
    }

    ArmTripleGenerator::ArmTripleGenerator() {}

    void ArmTripleGenerator::printAddrs(Triples& triples)
    {

        printf("函数栈分配：\n");
        for (int i = 0; i < func_stack_size.size(); ++i) {
            string name = triples.getFuncName({ i , TTT.func });
            printf("%d@%s : %d\n", i, name.c_str(),
                func_stack_size[i] * 4);
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

    void ArmTripleGenerator::print()
    {
        for (auto& p : instrs) {
            printf(p.toASM().c_str());
        }
    }

    void ArmTripleGenerator::write(AssemblyBuilder& asm_file)
    {
        asm_file.line("\t.syntax unified")
            .line("\t.arch armv7-a")
            .line("\t.fpu vfpv4")
            .line("\t.eabi_attribute 27, 3")
            .line("\t.eabi_attribute 28, 1")
            .line("\t.eabi_attribute 23, 1")
            .line("\t.eabi_attribute 24, 1")
            .line("\t.eabi_attribute 25, 1")
            .line("\t.eabi_attribute 26, 2")
            .line("\t.eabi_attribute 30, 6")
            .line("\t.eabi_attribute 34, 0")
            .line("\t.eabi_attribute 18, 4")
            .line();


        for (auto p : instrs) {
            asm_file.raw(p.toASM().c_str());
        }
    }

    std::string Addr::toString() const
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
            "up_tag", "low_tag", //高位读取， 低位读取
            "reglist", // 寄存器列表
            "reglsl_stack"
        };

        if (base == AB.null) {
            return "null";
        } else if (base == AB.reg) {
            return names[value];
        } else if (base == AB.imd) {
            return "#" + std::to_string((unsigned int)value);
        } else if (base == AB.tag) {
            return tag;
        } else if (base == AB.low_tag) {
            return "#:lower16:" + tag;
        } else if (base == AB.up_tag) {
            return "#:upper16:" + tag;
        } else if (base == AB.reglsl_stack) {
            return "[ " + names[tag[0]] + ", " + names[tag[1]] + ", lsl #2 ]";
        } else if (base == AB.reglist) {
            std::string regl = "{ ";
            bool con_flg = false;
            for (char r : tag) {
                if (con_flg)
                    regl += ", ";
                con_flg = true;
                regl += names[r];
            }
            return regl + " }";
        } else {
            return "[" + names[base] + ", #" + std::to_string(value * 4) + "]";
        }
    }
    std::string Instr::toASM()
    {
        std::string ans;
        std::string cmds[] = {
            "nop",

            "mov",
            "movw",
            "movt",
            "movs",
            "str",
            "ldr",

            "push",
            "pop",

            "b",  // jmp
            "bl", // call
            "bx",

            "add",
            "adds",
            "sub",
            "mul",
            "div",
            "mod",

            "lsls",
            "lsrs",

            "vmov",
            "vldr.32",
            "vstr.32",

            "vadd.f32",
            "vsub.f32",
            "vmul.f32",
            "vdiv.f32",

            "vcvt.f32.s32",
            "vcvt.s32.f32",

            "cmp",
            "beq",
            "bne",
            "ble",
            "blt",
            "bge",
            "bgt",

            "vcmpe.f32",
            "vmrs",

            "tag",    // 需特判, 放置tag
            "word",   // 需特判, 放置word
            "space",
            "ascii",  // 需特判, 放置ascii字符串
        };

        if (cmd == ACmd.tag) {
            ans = e1.tag + ":";
        } else if (cmd >= ACmd.word && cmd <= ACmd.space) {
            ans = "\t." + cmds[cmd] + " " + std::to_string(e1.value);
        } else {
            ans = "\t" + cmds[cmd];
            if (e1.base != AB.null)
                ans += "\t" + e1.toString();
            if (e2.base != AB.null)
                ans += ", " + e2.toString();
            if (e3.base != AB.null)
                ans += ", " + e3.toString();
        }
        return ans + "\n";
    }
}