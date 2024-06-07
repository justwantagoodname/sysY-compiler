#include "ast.h"
#include "pass.h"

bool ArrayInitNode_need_flatten(ASTNode* root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "ConstInitValue"));

    return ASTNode_querySelectorOne(root, "//ConstInitValue[0]") != NULL;
}
/**
 * @brief Expand the nested array initialization list to linear list keep repeat parameter to save space.
 * @example {1, {2, 3}, 4} => {1, 2, 3, 4}
 * @example {{}, {}, {1}} => {<ConstInitValue repeat="dim * 2">0</ConstInitValue>, <>1</>, <>0</>}
 */
void ArrayInitNode_expand(ASTNode* root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "CompUnit"));

    QueryResult* const_array_init_list = ASTNode_querySelector(root, "//Const/ConstInitValue"),
                *iter = NULL;
    DL_FOREACH(const_array_init_list, iter) {
        ASTNode* array_init_list = iter->node;

        if (ArrayInitNode_need_flatten(array_init_list)) {
            ASTNode* const_decl = ASTNode_querySelectorOne(array_init_list, "/..");
            assert(const_decl != NULL);
            assert(ASTNode_id_is(const_decl, "Const"));

            /* TODO */
            ASTNode* flatten_decl = ArrayInitNode_flatten(const_decl);
        }
    }
}