#pragma once

#ifndef ADAPTER_H
#define ADAPTER_H

/*
 * 用于平台特定汇编生成的 Interface ，此类应为抽象类，不应该加入实际实现
 */
class Adapter
{
public:
   Adapter() = default;
   virtual ~Adapter() = default;

   virtual const int getWordSize() = 0;
   
   virtual const std::string getRegName(int reg) = 0;
   virtual const std::string getStackPointerName() = 0;
   virtual const std::string getFramePointerName() = 0;
   virtual const std::string getReturnAddressName() = 0;
   virtual const std::string getPCName() = 0;

   virtual const void emitComment() = 0; // 仅插入空白行
   virtual const void emitComment(const std::string& content) = 0; // 写入注释行

   virtual void emitFunctionLabel(const std::string& funcName) = 0;
   virtual void emitLabel(const std::string& reg) = 0;
   virtual void loadImmediate(const std::string& reg, int x) = 0;
   virtual void loadImmediate(const std::string& reg, float x) = 0;
   virtual void loadLabelAddress(const std::string& reg, const std::string& labelName) = 0;
   virtual void pushStack(std::initializer_list<std::string> regs) = 0;
   virtual void popStack(std::initializer_list<std::string> regs) = 0;
   virtual void add(const std::string& dst, const std::string& src, int imm) = 0;
   virtual void sub(const std::string& dst, const std::string& src, int imm) = 0;
   virtual void mov(const std::string& dst, const std::string& src) = 0;
   virtual void call(const std::string& labelName) = 0;
};

#endif