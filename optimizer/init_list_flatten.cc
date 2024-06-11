#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"
#include "utils.h"

#include <vector>

//#define ARRAY_FLAT_LOG printf
#define ARRAY_FLAT_LOG(...) (0)

// 在Const下寻找子ConstInitValue 如果存在则需要展开 后面需要支持 Var
bool ArrayInitNode_need_flatten(const ASTNode *root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "Const") || ASTNode_id_is(root, "Var"));

    if (ASTNode_id_is(root, "Var")) {
        return ASTNode_has_attr(root, "array")
               && ASTNode_querySelectorOne(root, "/InitValue") != nullptr; // 如果是变量数组并且有初始化列表
    } else if (ASTNode_id_is(root, "Const")) {

        if (!ASTNode_has_attr(root, "array")) {
            return false;
        }

        // 是常量数组
        ASTNode *const_init_list = ASTNode_querySelectorOne(root, "/ConstInitValue");

        assert(ASTNode_id_is(root, "Const") && const_init_list != NULL); // 如果是常量数组这里必须有初始化列表

        return !ASTNode_has_attr(const_init_list, "flatten");
    }
    assert(false); // 不应该到达这里
}

ASTNode *ArrayInitItem_create(int value, int repeat) {
    ASTNode *ret = ASTNode_create("Number");
    ASTNode_add_attr_int(ret, "value", value);
    ASTNode_add_attr_int(ret, "repeat", repeat);
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
void ConstInitNode_flattener(const std::vector<int> &dim_sizes, ASTNode *cur_node, ASTNode *linear_init, int depth,
                             int space) {
    assert(cur_node != NULL);
    assert(linear_init != NULL);
    assert(space > 0);
    assert(depth <= dim_sizes.size());

    if (ASTNode_children_size(cur_node) == 0) {
        ASTNode_add_child(linear_init, ArrayInitItem_create(0, space));
        return;
    }

    QueryResult *sub_list = ASTNode_querySelector(cur_node, "/ConstInitValue"), *iter = NULL;

    DL_FOREACH(sub_list, iter) {

        ASTNode *child = ASTNode_querySelectorOne(iter->node, "/Exp[0]");
        if (child != NULL) {
            // 这个是一个表达式，因为语法生成的AST 是 <ConstInitValue><Exp><Number></Number></Exp></ConstInitValue>
            // 由于常量折叠和化简同步进行，在确保依赖关系正确的情况下，Exp 内总是化为Number字面量的或者引用之前的变量的
            assert(ASTNode_id_is(child, "Exp"));
            ASTNode *number = ASTNode_querySelectorOne(child, "/*");
            assert(ASTNode_id_is(number, "Number"));
            int value = -1;
            ASTNode_get_attr_int(number, "value", &value);

            ASTNode_add_child(linear_init, ArrayInitItem_create(value, 1));
            --space;

            continue;
        }

        child = ASTNode_querySelectorOne(iter->node, "/ConstInitValue");
        if (child != NULL || ASTNode_children_size(iter->node) == 0) {
            ConstInitNode_flattener(dim_sizes, iter->node, linear_init, depth + 1, dim_sizes[depth + 1]);
            space -= dim_sizes[depth + 1];

            continue;
        }

        // Shouldn't reach there.
        assert(false);
    }
    if (space > 0) {
        ASTNode_add_child(linear_init, ArrayInitItem_create(0, space));
    }
}

/**
 * 只展开常量数组
 * @param decl
 * @return 返回展开到线性的节点，注意此时还未修改原节点
 */
void ConstInitNode_flatten(ASTNode* decl, const std::vector<int>& dim_sizes) {
    ASTNode *init_value = ASTNode_querySelectorOne(decl, "/ConstInitValue");

    assert(init_value != nullptr);

    ASTNode *linear_init = ASTNode_create_attr("ConstInitValue", 2, "array", "true", "flatten", "true");

    ConstInitNode_flattener(dim_sizes, init_value, linear_init, 0, dim_sizes[0]);

    ASTNode_replace(linear_init, init_value);
    ASTNode_free(init_value);
    ASTNode_print(linear_init);
}

/**
 * 展开变量数组的逻辑，因为变量数组可以引用常量或者变量作为初始化列表的项目，所以需要特别处理，可以看到目前两块逻辑有很大重叠，等待重构到一起
 * @param dim_sizes 数组每一维度的大小，0代表整体大小
 * @param cur_node 当前初始化列表
 * @param linear_init 生成线性的一维表示结果存储到linear_init中
 * @param depth 当前的维度
 * @param space 将要初始化的值的数量
 */
void VarInitNode_flattener(const std::vector<int> &dim_sizes, ASTNode *cur_node, ASTNode *linear_init, int depth,
                           int space) {
    assert(cur_node != NULL);
    assert(linear_init != NULL);
    assert(space > 0);
    assert(depth <= dim_sizes.size());

    if (ASTNode_children_size(cur_node) == 0) {
        ASTNode_add_child(linear_init, ArrayInitItem_create(0, space));
        return;
    }

    QueryResult *sub_list = ASTNode_querySelector(cur_node, "/InitValue"), *iter = NULL;

    DL_FOREACH(sub_list, iter) {

        ASTNode *child = ASTNode_querySelectorOne(iter->node, "/Exp[0]");
        if (child != NULL) {
            // 这个是一个初始化值，因为语法生成的AST 是由于变量数组可以引用变量作为初始化值，因此需要特别处理，
            // 转为<Exp repeat="1"></Exp>的形式
            // 或者在Exp内部为字面量的前提下转为Number
            // 由于常量折叠已经做完，在确保依赖关系正确的情况下，Exp 内总是化为最简化的形式
            // 注意：这里还是假设输入的程序全部为正确的，不会出现变量引用未定义的情况 可以考虑在后续翻译中检查错误
            assert(ASTNode_id_is(child, "Exp"));
            ASTNode *inner = ASTNode_querySelectorOne(child, "/*");
            if (ASTNode_id_is(inner, "Number")) {
                int value = -1;
                ASTNode_get_attr_int(inner, "value", &value);
                ASTNode_add_child(linear_init, ArrayInitItem_create(value, 1));
            } else {
                // 不是字面量，整体clone
                ASTNode* cloned_exp = ASTNode_clone(child);
                ASTNode_clone(child);
                ASTNode_add_attr_int(cloned_exp, "repeat", 1);
                ASTNode_add_child(linear_init, cloned_exp);
            }
            --space;

            continue;
        }

        child = ASTNode_querySelectorOne(iter->node, "/InitValue");
        if (child != NULL || ASTNode_children_size(iter->node) == 0) {
            VarInitNode_flattener(dim_sizes, iter->node, linear_init, depth + 1, dim_sizes[depth + 1]);
            space -= dim_sizes[depth + 1];

            continue;
        }

        assert(false);
    }
    if (space > 0) {
        ASTNode_add_child(linear_init, ArrayInitItem_create(0, space));
    }
}

/**
 * 只展开变量数组
 */
void VarInitNode_flatten(ASTNode* decl, const std::vector<int>& dim_sizes) {
    ASTNode *init_value = ASTNode_querySelectorOne(decl, "/InitValue");

    assert(init_value != nullptr);

    ASTNode *linear_init = ASTNode_create_attr("InitValue", 2, "array", "true", "flatten", "true");

    VarInitNode_flattener(dim_sizes, init_value, linear_init, 0, dim_sizes[0]);

    ASTNode_replace(linear_init, init_value);
    ASTNode_free(init_value);
    ASTNode_print(linear_init);
}

/**
 * 将多维数组的初始化表达式展开为一维数组
 * 需要使用数组的维度信息，将会直接修改Const ConstInitValue节点后续会支持Var节点
 */
void ArrayInitNode_flatten(ASTNode *decl) {
    assert(decl != NULL);
    assert(ASTNode_id_is(decl, "Const") || ASTNode_id_is(decl, "Var"));
    assert(ASTNode_has_attr(decl, "array") && !ASTNode_has_attr(decl, "flatten")); // 是数组并且没有展平过

    // 首先获取数组的维度信息, 由于是在常量折叠过程中进行的，因此在保证初始化顺序的情况下，可以确保之前的数组声明每一维都被化简到了 Number 节点

    QueryResult *dims = ASTNode_querySelector(decl, "/ArraySize//Number"), *iter = NULL;
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

    if (ASTNode_id_is(decl, "Const")) {
        ConstInitNode_flatten(decl, dim_sizes);
    } else if (ASTNode_id_is(decl, "Var")) {
        VarInitNode_flatten(decl, dim_sizes);
    }
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