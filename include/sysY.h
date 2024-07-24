#pragma once

#ifndef SYSY_H
#define SYSY_H

#ifdef __cplusplus

#include <cstdint>

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <variant>
#include <memory>
#include <functional>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "utlist.h"
#include "uthash.h"

#ifdef __cplusplus
}
#endif

// 数组类型的以[开头表示一个数组
// 左值类型的以L开头
const auto SyInt = "Int";
const auto SyVoid = "Void";
const auto SyFloat = "Float";

#ifndef NO_PANIC

#define panic(str) do { \
    printf(str "\n"); \
    assert(false); \
} while (0)

#endif

#endif
