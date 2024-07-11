#pragma once

#ifndef ADAPTER_H
#define ADAPTER_H

/*
 * 用于平台特定汇编生成的 Interface ，此类应为抽象类，不应该加入实际实现
 */
class StackTranslator;
class Adapter
{
public:
   Adapter() = default;
   virtual ~Adapter() = default;

   virtual std::string platformName() const = 0;

   /**
    * 生成汇编代码后的准备工作
    */
   virtual void postGenerate() {}

   /**
    * 生成汇编代码前的准备工作
    */
   virtual void preGenerate() {}

   virtual bool isExternalFunction(const std::string& funcName) = 0;

   // 后面可以改成 Translator
   virtual void emitExternFunction(const std::string& funcName, ASTNode* callContext, StackTranslator* translator) = 0;

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
   virtual void loadRegister(const std::string& dst, const std::string& src, int offset) = 0;
   virtual void storeRegister(const std::string& src, const std::string& dst, int offset) = 0;

   virtual void pushStack(std::initializer_list<std::string> regs) = 0;
   virtual void pushStack(const std::vector<std::string>& regs) = 0;
   virtual void popStack(std::initializer_list<std::string> regs) = 0;
   virtual void popStack(const std::vector<std::string>& regs) = 0;

   virtual void neg(const std::string& dst, const std::string& src) = 0;

   virtual void add(const std::string& dst, const std::string& src, int imm) = 0;
   virtual void add(const std::string& dst, const std::string& src1, const std::string& src2) = 0;

   virtual void sub(const std::string& dst, const std::string& src, int imm) = 0;
   virtual void sub(const std::string& dst, const std::string& src1, const std::string& src2) = 0;

   virtual void mul(const std::string& dst, const std::string& src1, const int imm) = 0;
   virtual void mul(const std::string& dst, const std::string& src1, const std::string& src2) = 0;

   virtual void div(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void mod(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void mov(const std::string& dst, const std::string& src) = 0;

   virtual void call(const std::string& labelName) = 0;
   virtual void jump(const std::string& labelName) = 0;

   virtual void nop() = 0;

   virtual void jumpEqual(const std::string& src1, const std::string& src2, const std::string& labelName) = 0;
   virtual void jumpEqual(const std::string& src1, int imm, const std::string& labelName) = 0;
   virtual void jumpEqual(const std::string& src1, int imm, const std::string& eqLabel, const std::string& neLabel) = 0;

   virtual void jumpNotEqual(const std::string& src1, const std::string& src2, const std::string& labelName) = 0;
   virtual void jumpNotEqual(const std::string& src1, int imm, const std::string& labelName) = 0;

   // sysY中规定取反的真值是非0，这里为了方便起见，规定非0值为1和条件逻辑运算一致
   virtual void notReg(const std::string& dst, const std::string& src) = 0;

   virtual void cmpEqual(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void cmpNotEqual(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void cmpLess(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void cmpLessEqual(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void cmpGreater(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
   virtual void cmpGreaterEqual(const std::string& dst, const std::string& src1, const std::string& src2) = 0;

   // 浮点操作相关 考虑到一般都用 fpu 所以按照抽象 fpu 操作实现

   virtual std::string getFRegisterName(int reg) = 0;

   // 在浮点和通用寄存器间传递数据
   virtual void fmov(const std::string& dst, const std::string& src) = 0;

   virtual void fpushStack(std::initializer_list<std::string> regs) = 0;
   virtual void fpushStack(const std::vector<std::string>& regs) = 0;
   virtual void fpopStack(std::initializer_list<std::string> regs) = 0;
   virtual void fpopStack(const std::vector<std::string>& regs) = 0;

   virtual void floadRegister(const std::string& dst, const std::string& src, int offset) = 0;
   virtual void fstoreRegister(const std::string& src, const std::string& dst, int offset) = 0;
   
   virtual void i2f(const std::string& dst, const std::string& src) = 0;

   virtual void f2i(const std::string& dst, const std::string& src) = 0;

   virtual void f2d(const std::string& dst, const std::string& src) = 0;

   virtual void d2f(const std::string& dst, const std::string& src) = 0;

   // 加法
   virtual void fadd(const std::string& dst, const std::string& src1, const std::string& src2) = 0;

//   // 减法
//   virtual void fsub(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
//
//   // 乘法
//   virtual void fmul(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
//
//   // 除法
//   virtual void fdiv(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
//
//   // 取模
//   virtual void fmod(const std::string& dst, const std::string& src1, const std::string& src2) = 0;
};

#endif