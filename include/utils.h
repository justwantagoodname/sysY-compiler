#pragma once
#ifndef UTILS_H
#define UTILS_H

#include "sysY.h"
#include <vector>

/**
 * 将多维数组的索引转换为线性索引
 * @param dim_sizes 数组的声明大小
 * @param locator_access_size 访问的大小
 * @return 线性索引
 */
size_t multi_dimensional_index_to_linear_index(const std::vector<int> &dim_sizes, const std::vector<int> &locator_access_size);


bool is_integer(double x);

void print_version();

void print_help(const char* program_name);

/**
 * 生成一个唯一的标签, 仅保证在当前编译单元内唯一！
 * @return
 */
std::string generateLabel();

// 类型计算相关的函数

bool is_array_type(const std::string &x);

bool is_primitive_type(const std::string &x);

// 数组类型解引用
std::string deref_array(const std::string &x);

std::string deref_lval(const std::string &x);

#endif //UTILS_H
