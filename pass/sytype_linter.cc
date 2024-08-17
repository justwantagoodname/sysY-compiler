#include "sysY.h"
#include "pass.h"

std::string linter_get_array_decl_type(const ASTNode* array_decl) {
    assert(ASTNode_has_attr(array_decl, "array"));
    int dim_size = 0;
    if (ASTNode_id_is(array_decl, "ParamDecl")) {
        dim_size = ASTNode_children_size(array_decl);
    } else {
        dim_size = ASTNode_children_size(ASTNode_querySelectorOne(array_decl, "/ArraySize"));
    }
    const char* base_type;
    bool hasBaseType = ASTNode_get_attr_str(array_decl, "type", &base_type);
    assert(hasBaseType);

    std::string type = base_type;
    for (int i = 0; i < dim_size; i++) {
        type.insert(0, "[");
    }
    return type;
}

void SyType_linter(ASTNode* root) {
    auto paramdecls = ASTNode_querySelector(root, "//Params/ParamDecl[@array]");
    paramdecls->foreach([](auto param_decl) {
        ASTNode_set_attr_str(param_decl, "type", linter_get_array_decl_type(param_decl));
    });
}