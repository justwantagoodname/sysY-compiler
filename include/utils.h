#pragma once
#ifndef UTILS_H
#define UTILS_H

#include "sysY.h"
#include <vector>

#ifdef __cplusplus

/**
 * 将多维数组的索引转换为线性索引
 * @param dim_sizes 数组的声明大小
 * @param locator_access_size 访问的大小
 * @return 线性索引
 */
size_t multi_dimensional_index_to_linear_index(const std::vector<int> &dim_sizes, const std::vector<int> &locator_access_size);

#endif

/**
 * 根据函数名判断是否为 sysY 库函数
 * @param func_name 传入函数名
 * @return
 */
bool is_lib_function(const char* func_name);

bool is_integer(double x);

void print_version();

void print_help(const char* program_name);

#endif //UTILS_H
