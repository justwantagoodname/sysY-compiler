#pragma once
#include "triples.h"
namespace TriplesArmGenerator {
    class ADDRBASE {
    public:
        enum ADDRBASEENUM{
            reg, // �Ĵ�����
            r0, r1, r2, r3, // �����Ĵ���
            r4, r5, r6, r7, r8, r9, r10, r11, // ͨ�üĴ���
            s0, // r12, ip, scratch�Ĵ���, ��Ų��, ���Ҳ��ûָ�, Ųȥ��s0�ˣ�
            sp, // r13, ջ�Ĵ���
            lr, // r14, �жϼĴ���
            pc, // r15, pc�Ĵ���

        };
    } AddrBase;
    auto& AB = AddrBase;

    struct Addr {
        ADDRBASE::ADDRBASEENUM base;
        int value;
    };
    class Generator {
    private:
        Triples triples;

        // TODO
        // value -> λ��
        std::vector<Addr> value_addr;
        // temp -> λ��
        std::vector<Addr> temp_addr;
    public:
        // getplace
        void getStackPlace();
        // make
    };
}