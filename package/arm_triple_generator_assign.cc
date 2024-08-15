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
            // �Ǻ����� ����ջ����
            if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
                //printf("into func\n");
                // ���func ��� �� block ���
                now_func_block_id = triples[i + 1].e1.value;
                now_func_id = triple.e1.value;

                // ���亯��������ַ��ջ��
                //int n = 1;
                //func_reg[now_func_id] = { AB.s0 };
                func_reg[now_func_id] = {};
                //for (int j = 1; j < params.size(); ++j) {
                //    value_addr[params[j].first] = Addr(AB.s0, j - 1 + n);// j - 1: ��j��������param��һλ�Ƿ���ֵ���ͣ���+n���Ĵ�������λ��
                //}

                //stack_size = params.size() - 1;
                //stack_size += n; // ջģʽ�½������ϴ�ջ��ָ��
                stack_size = 0;
                // ��ò�����Ŀ
            }

            // ��������������ջ����
            if (triple.cmd == TCmd.blke && triple.e1.value == now_func_block_id) {

                // ������ջ���
                auto& params = triples.funcid_params[now_func_id];
                int param_size = params.size() - 1;

                for (int j = 0; j < param_size; ++j) {
                    value_addr[params[j + 1].first] = Addr(AB.sp, stack_size);
                    ++stack_size;
                }

                // ���ݺ���ջ��С�洢��������Ӧ���洢λ��
                
                // ���ͼ����������������
                int int_count = 0, float_count = 0;
                // �������μĴ������� ����Ĵ�����
                int int_reg_number = 4, float_reg_number = 1;

                // ��ʼ���������ص�ַ�б�
                func_params_load[now_func_id].clear();
                func_params_load[now_func_id].resize(param_size);

                for (int j = 0; j < param_size; ++j) {

                    bool flg = false;

                    int ptype = params[j + 1].second;
                    if (ptype != 2) {
                        // ����
                        if (int_count >= int_reg_number) {
                            flg = true;
                        } else {
                            func_params_load[now_func_id][j] = { AB.reg, AB.r0 + int_count };
                        }
                        ++int_count;
                    } else {
                        // ����
                        if (float_count >= float_reg_number) {
                            flg = true;
                        } else {
                            func_params_load[now_func_id][j] = { AB.reg, AB.fa0 + float_count };
                        }
                        ++float_count;
                    }

                    if(flg){
                        // ����ջ�����
                        Addr r1 = value_addr[params[j + 1].first];
                        r1.value -= stack_size;
                        func_params_load[now_func_id][j] = r1;
                    }
                }

                // �洢����ջ��С
                func_stack_size[now_func_id] = stack_size;

                //printf("out func\n");
                // �˳���ǰ��������
                now_func_block_id = -1;
                now_func_id = -1;
                stack_size = 0;
            }

            // Ϊ�ˣ�ֻ��Ϊ�ˣ����㣬��ϴ洢temp��value
            // �����var������ջ
            if (triple.cmd == TCmd.var) {
                //printf("var def\n");
                value_addr[triple.e1.value] = Addr(AB.sp, stack_size);
                stack_size += triple.e2.value;
            }

            // ������µ�temp������ջ
            if (triple.to.type == TTT.temp
                && temp_addr[triple.to.value].base == AB.null) {
                //printf("temp def\n");
                temp_addr[triple.to.value] = Addr(AB.sp, stack_size);
                stack_size += 1;
            }

        }
    }
}