#pragma once

#ifndef SYSY_H
#define SYSY_H

#ifdef __cplusplus

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <variant>
#include <memory>

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

// SysY 类型

const auto SyInt = "Int";
const auto SyVoid = "Void";
const auto SyFloat = "Float";

// 数组类型的以[开头表示一个数组
// 左值类型的以L开头

#endif
