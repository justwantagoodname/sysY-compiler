#include "arm_triple_gnerator.h"
namespace TriplesArmGenerator {
    auto TTT = Triples::TT;
    auto TCmd = Triples::Cmd;

    void TriplesArmGenerator::Generator::getStackPlace()
    {
        for (int i = 0; i < triples.size(); ++i) {
            Triples::Triple& triple = triples[i];

            // �Ǻ����� ����ջ����
            if (triple.cmd == TCmd.tag && triple.e1.type == TTT.func) {
                // ���亯��������ַ

            }
        }
    }
}