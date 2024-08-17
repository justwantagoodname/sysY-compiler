#pragma once

#ifndef SYSY_H
#define SYSY_H

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
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "utlist.h"
#include "uthash.h"

// 需要可以直接添加
using std::string;
using std::vector;
using std::map;
using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::cerr;
using std::endl;

// 数组类型的以[开头表示一个数组
// 左值类型的以L开头
const auto SyInt = "Int";
const auto SyVoid = "Void";
const auto SyFloat = "Float";

#ifndef NO_PANIC

#define panic(str) do { \
    fprintf(stderr, str "\n"); \
    assert(false); \
} while (0)

#ifdef DEBUG
#define panic_on(cond, str) do { \
    if (cond) { \
        fprintf(stderr, str "\n"); \
        assert(false); \
    } \
} while (0)
#else
#define panic_on(cond, str) do { } while (0)
#endif

#endif

#endif
