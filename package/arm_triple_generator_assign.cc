#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    void TriplesArmGenerator::ArmTripleGenerator::getStackPlace(Triples& triples)
    {
        value_addr.clear();
        value_addr.resize(triples.value_pointer.size());

        temp_addr.clear();
        temp_addr.resize(triples.temp_count);

        func_stack_size.clear();
        func_stack_size.resize(triples.funcid_params.size() - 14);

        func_reg.clear();
        func_reg.resize(triples.funcid_params.size() - 14);

        func_params_load.clear();
        func_params_load.resize(triples.funcid_params.size() - 14);


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
                //int n = 1;
                //func_reg[now_func_id] = { AB.s0 };
                func_reg[now_func_id] = {};
                //for (int j = 1; j < params.size(); ++j) {
                //    value_addr[params[j].first] = Addr(AB.s0, j - 1 + n);// j - 1: 第j个参数（param第一位是返回值类型），+n：寄存器保存位置
                //}

                //stack_size = params.size() - 1;
                //stack_size += n; // 栈模式下仅保存上次栈顶指针
                stack_size = 0;
                // 获得参数数目
            }

            // 结束函数，结束栈分析
            if (triple.cmd == TCmd.blke && triple.e1.value == now_func_block_id) {

                // 处理函数栈情况
                auto& params = triples.funcid_params[now_func_id];
                int param_size = params.size() - 1;

                for (int j = 0; j < param_size; ++j) {
                    value_addr[params[j + 1].first] = Addr(AB.sp, stack_size);
                    ++stack_size;
                }

                // 依据函数栈大小存储函数参数应当存储位置
                
                // 整型计数器，浮点计数器
                int int_count = 0, float_count = 0;
                // 参数整形寄存器数， 浮点寄存器数
                int int_reg_number = 4, float_reg_number = 1;

                // 初始化函数加载地址列表
                func_params_load[now_func_id].clear();
                func_params_load[now_func_id].resize(param_size);

                for (int j = 0; j < param_size; ++j) {

                    bool flg = false;

                    int ptype = params[j + 1].second;
                    if (ptype != 2) {
                        // 整形
                        if (int_count >= int_reg_number) {
                            flg = true;
                        } else {
                            func_params_load[now_func_id][j] = { AB.reg, AB.r0 + int_count };
                        }
                        ++int_count;
                    } else {
                        // 浮点
                        if (float_count >= float_reg_number) {
                            flg = true;
                        } else {
                            func_params_load[now_func_id][j] = { AB.reg, AB.fa0 + float_count };
                        }
                        ++float_count;
                    }

                    if(flg){
                        // 存入栈的情况
                        Addr r1 = value_addr[params[j + 1].first];
                        r1.value -= stack_size;
                        func_params_load[now_func_id][j] = r1;
                    }
                }

                // 存储函数栈大小
                func_stack_size[now_func_id] = stack_size;

                //printf("out func\n");
                // 退出当前函数分析
                now_func_block_id = -1;
                now_func_id = -1;
                stack_size = 0;
            }

            // 为了（只是为了）方便，混合存储temp和value
            // 如果是var，分配栈
            if (triple.cmd == TCmd.var) {
                //printf("var def\n");
                value_addr[triple.e1.value] = Addr(AB.sp, stack_size);
                stack_size += triple.e2.value;
            }

            // 如果是新的temp，分配栈
            if (triple.to.type == TTT.temp
                && temp_addr[triple.to.value].base == AB.null) {
                //printf("temp def\n");
                temp_addr[triple.to.value] = Addr(AB.sp, stack_size);
                stack_size += 1;
            }

        }
    }
}