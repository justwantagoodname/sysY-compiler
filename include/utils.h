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

#endif //UTILS_H
