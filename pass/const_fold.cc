#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"

/**
 * @brief Fold all expression in the AST
 * @note this AST *Will Be Modified* after fold 此函数目前仅对保证正确常量表达式引用顺序的 AST 有效
 */
void ConstNode_unfold(ASTNode* root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "CompUnit"));

    QueryResult* const_decls = ASTNode_querySelector(root, "//Exp"),
                *cur = NULL;
    DL_FOREACH(const_decls, cur) {
//        printf("Found with ==\n");
        ASTNode* const_exp = cur->node;
//        ASTNode_print(const_exp);
        ASTNode* sim_exp = ExpNode_simplify(const_exp);
        ASTNode_replace(sim_exp, const_exp);
        ASTNode_free(const_exp);
//        ASTNode_print(sim_exp);
//        printf("====\n");
    }
}

/**
 * 计算数组大小并存在 AST 中的 size 属性中，注意此属性其实是元素个数而并不是字节数
 */
void ArrayNode_calculate_size(ASTNode* decl) {
    assert(decl);
    assert(ASTNode_id_is(decl, "Var") || ASTNode_id_is(decl, "Const"));
    assert(ASTNode_has_attr(decl, "array"));

    if (ASTNode_has_attr(decl, "size")) {
        return;
    }

    // ArraySize 中的表达式必须在之前被化简
    QueryResult* dims = ASTNode_querySelector(decl, "/ArraySize//Number"), *cur = nullptr;
    int dim_size = 0;
    int total_size = 1;

    DL_FOREACH(dims, cur) {
        const char* type = nullptr;
        ASTNode_get_attr_str(cur->node, "type", &type);
        assert(strcmp(type, "Int") == 0);

        int dim;
        ASTNode_get_attr_int(cur->node, "value", &dim);
        dim_size++;
        total_size *= dim;
    }
    assert(dim_size == ASTNode_children_size(ASTNode_querySelectorOne(decl, "/ArraySize")));

    ASTNode_add_attr_int(decl, "size", total_size);
}


/**
 * 鉴于有些变量和常量数组可能未被引用，所以再扫描一遍
 * @param root
 */
void ArrayDecl_flatten(ASTNode* root) {
    assert(root);
    assert(ASTNode_id_is(root, "CompUnit"));

    QueryResult* var_array_decls = ASTNode_querySelector(root, "//Var[@array='true']"),
            *const_array_decls = ASTNode_querySelector(root, "//Const[@array='true']"),
            *iter = nullptr;

    DL_CONCAT(const_array_decls, var_array_decls);

    DL_FOREACH(const_array_decls, iter) {
        ArrayNode_calculate_size(iter->node);
        if (ArrayInitNode_need_flatten(iter->node)) {
            ArrayInitNode_flatten(iter->node);
        }
    }
}