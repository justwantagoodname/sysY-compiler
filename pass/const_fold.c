#include "sysY.h"
#include "ast.h"
#include "exp_sim.h"

/**
 * @brief Fold all expression in the AST
 * @note this AST *Will Be Modified* after fold 
 */
void ConstNode_unfold(ASTNode* root) {
    assert(root != NULL);
    assert(ASTNode_id_is(root, "CompUnit"));

    QueryResult* const_decls = ASTNode_querySelector(root, "//Exp"),
                *cur = NULL;
    DL_FOREACH(const_decls, cur) {
        printf("Found with ==\n");
        ASTNode* const_exp = cur->node;
        ASTNode* sim_exp = ExpNode_simplify(const_exp);
        ASTNode_replace(sim_exp, const_exp);
        ASTNode_free(const_exp);
        // ASTNode_print(sim_exp);
        printf("====\n");
    }
}