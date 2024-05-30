#include "lib/uthash.h"
#include "lib/utlist.h"
#include "action.h"

struct ValueSymbol *addVSArray(struct ValueSymbol *array, struct ValueSymbol *vSymbol) {
    assert(vSymbol != NULL);

    LL_PREPEND(array, vSymbol);
    return array;
}

void modifyVSType(struct ValueSymbol *array, enum ValueType type) {
    assert(array != NULL);

    struct ValueSymbol *cur;
    LL_FOREACH(array, cur) {
        cur->type = type;
    }
}

ValueSymbol *appendVSList(ValueSymbol *array, ValueSymbol *array2) {
    assert(array2 != NULL);

    struct ValueSymbol *cur;
    LL_CONCAT(array, array2);

    return array;
}