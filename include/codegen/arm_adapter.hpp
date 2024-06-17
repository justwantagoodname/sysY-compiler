#pragma once

#ifndef ARM_ADAPTER_H
#define ARM_ADAPTER_H

#include "sysY.h"
#include "asm_helper.hpp"
#include "codegen/adapter.hpp"

class ARMAdapter : public Adapter
{
private:
   AssemblyBuilder& asm_file;
public:
   explicit ARMAdapter(AssemblyBuilder& asm_file) : asm_file(asm_file) {}
   ~ARMAdapter() override = default;
   
   void loadImmediate(const char* reg, int x) override {
      assert(0);
   }
   void loadImmediate(const char* reg, float x) override {
      assert(0);
   }
};

#endif