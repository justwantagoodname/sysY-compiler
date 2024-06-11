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
        printf("Found with ==\n");
        ASTNode* const_exp = cur->node;
        ASTNode_print(const_exp);
        ASTNode* sim_exp = ExpNode_simplify(const_exp);
        ASTNode_replace(sim_exp, const_exp);
        ASTNode_free(const_exp);
        ASTNode_print(sim_exp);
        printf("====\n");
    }
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
            *iter = NULL;

    DL_CONCAT(const_array_decls, var_array_decls);

    DL_FOREACH(const_array_decls, iter) {
        if (ArrayInitNode_need_flatten(iter->node)) {
            ArrayInitNode_flatten(iter->node);
        }
    }
}