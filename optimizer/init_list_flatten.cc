#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"

#include <vector>

// 在Const下寻找子ConstInitValue 如果存在则需要展开 后面需要支持 Var
bool ArrayInitNode_need_flatten(ASTNode* root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "Const") || ASTNode_id_is(root, "Var"));

    if (ASTNode_id_is(root, "Var") && ASTNode_querySelectorOne(root, "/ConstInitValue") == NULL) return false;

    ASTNode* init_list = ASTNode_querySelectorOne(root, "/ConstInitValue");
    assert(init_list != NULL);

    return !ASTNode_has_attr(init_list, "flatten");
}

ASTNode* ArrayInitItem_create(int value, int repeat) {
    ASTNode* ret = ASTNode_create("Number");
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
 * @return 返回剩余未初始化的值数量
 */
void ArrayInitNode_flattener(const std::vector<int>& dim_sizes, ASTNode * cur_node, ASTNode* linear_init, int depth, int space) {
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
            // 这个是一个变量，因为语法生成的AST 是 <ConstInitValue><Exp><Number></Number></Exp></ConstInitValue>
            // 由于常量折叠和化简同步进行，在确保依赖关系正确的情况下，Exp 内总是化为Number字面量的
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
            ArrayInitNode_flattener(dim_sizes, iter->node, linear_init, depth + 1, dim_sizes[depth + 1]);
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
 * 将多维数组的初始化表达式展开为一维数组
 * 需要使用数组的维度信息，将会直接修改Const ConstInitValue节点后续会支持Var节点
 */
void ArrayInitNode_flatten(ASTNode* decl) {
    assert(decl != NULL);
    assert(ASTNode_id_is(decl, "Const") || ASTNode_id_is(decl, "Var"));

    // 首先获取数组的维度信息, 由于是在常量折叠过程中进行的，因此在保证初始化顺序的情况下，可以确保之前的数组声明每一维都被化简到了 Number 节点

    QueryResult* dims = ASTNode_querySelector(decl, "/ArraySize//Number"), *iter = NULL;
    std::vector<int> dim_sizes;

    DL_FOREACH(dims, iter) {
        int dim_size = -1;
        ASTNode_get_attr_int(iter->node, "value", &dim_size);
        dim_sizes.push_back(dim_size);
    }

    // 倒序将每一维乘起来，就是每一位维度大小
    dim_sizes.push_back(1);

    for (auto i = dim_sizes.size() - 1;i > 0; i--) {
        dim_sizes[i - 1] = dim_sizes[i - 1] * dim_sizes[i];
    }

    ASTNode* init_value = ASTNode_querySelectorOne(decl, "/ConstInitValue");

    if (init_value == NULL) {
        assert(ASTNode_id_is(decl, "Var")); // 如果没有声明初始化，说明这是一个变量数组
        return;
    }

    ASTNode* linear_init = ASTNode_create_attr("ConstInitValue", 2, "array", "true", "flatten", "true");

    ASTNode_print(init_value);

    ArrayInitNode_flattener(dim_sizes, init_value, linear_init, 0, dim_sizes[0]);

    ASTNode_replace(linear_init, init_value);
    ASTNode_free(init_value);
    ASTNode_print(linear_init);
    assert(0);
}