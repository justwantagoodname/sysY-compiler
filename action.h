#pragma once

#ifndef ACTION_H
#define ACTION_H

#include "sym.h"

struct ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol);

void modifyVSType(struct ValueSymbol *array, enum ValueType type);

ValueSymbol *appendVSList(ValueSymbol *array, ValueSymbol *array2);

#endif