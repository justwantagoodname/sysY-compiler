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


   virtual void loadImmediate(const char* reg, int x) = 0;
   virtual void loadImmediate(const char* reg, float x) = 0;
};

#endif