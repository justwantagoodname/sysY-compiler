#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"
#include "utils.h"

#include <vector>

//#define ARRAY_FLAT_LOG printf
#define ARRAY_FLAT_LOG(...) (0)

// 在Const下寻找子ConstInitValue 如果存在则需要展开 后面需要支持 Var
bool ArrayInitNode_need_flatten(const ASTNode *root) {
    assert(root != nullptr);
    assert(ASTNode_id_is(root, "Const") || ASTNode_id_is(root, "Var"));

    auto init_list = ASTNode_querySelectorOne(root, "/InitValue");
    bool has_init_list = init_list != nullptr;

    return When<bool>(root, {
        TagMatch<bool>("Const", [&]() {
            panic_on(ASTNode_id_is(root, "Const") && !has_init_list, "常量数组这里必须有初始化列表");
            return ASTNode_has_attr(root, "array") && !ASTNode_has_attr(init_list, "flatten");
        }),
        TagMatch<bool>("Var", [&]() {
            return ASTNode_has_attr(root, "array") && has_init_list; // 如果是变量数组并且有初始化列表
        })
    });
}

ASTNode *ArrayInitItem_create(double value, int repeat, const std::string& decl_type) {
    ASTNode *ret = ASTNode_create("Number");
    ASTNode_add_attr_int(ret, "value", (int) value);
    ASTNode_add_attr_int(ret, "repeat", repeat);
    ASTNode_add_attr_str(ret, "type", decl_type.c_str());
    return ret;
}

/**
 * 遍历初始化列表声明，
 * @param dim_sizes 数组每一维度的大小，0代表整体大小
 * @param cur_node 当前初始化列表
 * @param linear_init 生成线性的一维表示结果存储到linear_init中
 * @param depth 当前的维度
 * @param space 将要初始化的值的数量
 */
// Update: 修改为递归填充
void ArrayInitNode_flattener(const std::string decl_type, const std::vector<int> &dim_sizes, ASTNode *cur_node, ASTNode *linear_init, int depth) {
    assert(cur_node != nullptr);
    assert(linear_init != nullptr);
    assert(depth < dim_sizes.size());

    auto current_array_size = dim_sizes[depth]; // 当前类型(维度)下剩余填充需要的元素数量

    QueryResult *sub_list = ASTNode_querySelector(cur_node, "*"), *item;
    DL_FOREACH(sub_list, item) {
        if (ASTNode_has_attr(item->node, "visited")) continue;
        if (current_array_size == 0) return; // 当前维度已经填充完毕，直接返回填充下一个维度

        When(item->node, {
            TagMatch<void>("Exp", [&]() {
                // 内部是一个表达式，这个表达式应该是一个常量表达式，因为常量折叠已经完成
                // 那么填充最内层的维度
                if (dim_sizes[depth + 1] != 1) {
                    // 进入下一个维度，但是语法节点不变因为实际的节点是被省略了
                    ArrayInitNode_flattener(decl_type, dim_sizes, cur_node, linear_init, depth + 1);
                    current_array_size -= dim_sizes[depth + 1]; // 已经填充了一部分
                } else {
                    // 否则，那么已经到达需要填充的维度了，直接填充即可
                    auto number = ASTNode_querySelectorOne(item->node, "/Number");
                    int value;
                    string type;
                    ASTNode_get_attr_int(number, "value", &value);
                    ASTNode_get_attr_str(number, "type", type);
                    ASTNode_add_child(linear_init, ArrayInitItem_create(value, 1, type));
                    current_array_size--; // 已经填充了一个
                }
            }),
            TagMatch<void>("InitValue", [&]() {
                // 内部是另一个初始化列表，递归填充
                panic_on(depth + 1 > dim_sizes.size(), "初始化列表和实际声明层级不匹配");
                ArrayInitNode_flattener(decl_type, dim_sizes, item->node, linear_init, depth + 1);
                current_array_size -= dim_sizes[depth + 1]; // 已经填充了一部分
            })
        });

        if (!ASTNode_has_attr(item->node, "visited")) ASTNode_add_attr_str(item->node, "visited", "true");
    }

    // 当前填充完后如果还有剩余的，或者是空的 {}，那么应该填充全零 0，数量为当前类型维度的大小
    if (current_array_size > 0) {
        ASTNode_add_child(linear_init, ArrayInitItem_create(0, current_array_size, decl_type));
    }
}

/**
 * 将多维数组的初始化表达式展开为一维数组
 * 需要使用数组的维度信息，将会直接修改 InitValue 节点
 */
void ArrayInitNode_flatten(ASTNode *decl) {
    assert(decl != nullptr);
    assert(ASTNode_id_is(decl, "Const") || ASTNode_id_is(decl, "Var"));
    assert(ASTNode_has_attr(decl, "array") && !ASTNode_has_attr(decl, "flatten")); // 是数组并且没有展平过

    // 首先获取数组的维度信息, 由于是在常量折叠过程中进行的，因此在保证初始化顺序的情况下，可以确保之前的数组声明每一维都被化简到了 Number 节点

    QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize//Number"), *iter = nullptr;
    std::vector<int> dim_sizes;

    DL_FOREACH(dims, iter) {
        int dim_size = -1;
        ASTNode_get_attr_int(iter->node, "value", &dim_size);
        dim_sizes.push_back(dim_size);
    }

    // 倒序将每一维乘起来，就是每一位维度大小
    dim_sizes.push_back(1);

    for (auto i = dim_sizes.size() - 1; i > 0; i--) {
        dim_sizes[i - 1] = dim_sizes[i - 1] * dim_sizes[i];
    }

    const char *decl_type_str;
    ASTNode_get_attr_str(decl, "type", &decl_type_str);
    std::string decl_type(decl_type_str);

    ASTNode *init_value = ASTNode_querySelectorOne(decl, "/InitValue");

    assert(init_value != nullptr);

    ASTNode *linear_init = ASTNode_create_attr("InitValue", 2, "array", "true", "flatten", "true");

    ArrayInitNode_flattener(decl_type, dim_sizes, init_value, linear_init, 0);

    ASTNode_replace(linear_init, init_value);
    ASTNode_free(init_value);
}

/**
 * 用一个数组locator来获取常量数组的值 
 * @note locator 中的每一个维度都必须是一个Number节点, decl中的ArraySize节点必须是Number节点, ConstInitValue 都必须展平过
 * @return 成功取值时返回一个复制过的Number节点，否则返回NULL
 */

ASTNode *ArrayInitNode_get_value_by_linear_index(const ASTNode *decl, const ASTNode *locator) {
    assert(decl != NULL);
    assert(ASTNode_id_is(decl, "Const"));
    assert(ASTNode_id_is(locator, "Locator"));

    // 检查locator 引用的常量数组是否在范围内
    QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize//Number"), *iter = NULL;
    std::vector<int> dim_sizes; // 申明大小

    DL_FOREACH(dims, iter) {
        int dim_size = -1;
        ASTNode_get_attr_int(iter->node, "value", &dim_size);
        dim_sizes.push_back(dim_size);
    }

    std::vector<int> locator_access_size;
    QueryResult *locator_dims = ASTNode_querySelector(locator, "/Dimension/Number");
    iter = NULL;
    DL_FOREACH(locator_dims, iter) {
        int dim_size = -1;
        ASTNode_get_attr_int(iter->node, "value", &dim_size);
        locator_access_size.push_back(dim_size);
    }

    assert(locator_access_size.size() == dim_sizes.size());

    for (int i = 0; i < dim_sizes.size(); i++) {
        assert(0 <= locator_access_size[i] && locator_access_size[i] < dim_sizes[i]);
    }

    // 计算线性索引
    size_t linear_index = multi_dimensional_index_to_linear_index(dim_sizes, locator_access_size);

    // 获取初始化列表
    QueryResult *init_list = ASTNode_querySelector(decl, "/ConstInitValue[@array,@flatten]/Number");
    iter = NULL;

    assert(init_list != NULL);

    int current_index = 0;
    DL_FOREACH(init_list, iter) {
        int value = -1, repeat = -1;
        ASTNode_get_attr_int(iter->node, "value", &value);
        ASTNode_get_attr_int(iter->node, "repeat", &repeat);

        if (current_index + repeat > linear_index) {
            const char *type;
            ASTNode_get_attr_str(decl, "type", &type);
            ASTNode *ret = ASTNode_create_attr("Number", 1, "type", type);
            ASTNode_add_attr_int(ret, "value", value);
            return ret;
        }
        current_index += repeat;
    }
    return NULL;
}