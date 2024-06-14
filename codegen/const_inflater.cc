#include "codegen/const_inflater.hpp"

char* ConstInflater::getLabel(ASTNode* const_decl) {
    assert(ASTNode_id_is(const_decl, "Const"));
    
    const char* decl_name = nullptr;
    ASTNode_get_attr_str(const_decl, "name", &decl_name);
    assert(decl_name != nullptr);

    char* name = strdup(decl_name);

    QueryResult *scopes = ASTNode_querySelector(const_decl, "/ancestor::Scope"), *cur = NULL;
    DL_FOREACH(scopes, cur) {
        const char* scope_name = nullptr;
        ASTNode_get_attr_str(cur->node, "name", &scope_name);
        assert(scope_name != nullptr);
        // concat scope name
        asprintf(&name, "%s_%s", scope_name, name);   
    }
    return name;
}

void ConstInflater::inflateConstDecl(ASTNode* const_decl, AssemblyBuilder& asm_builder) {
    assert(ASTNode_id_is(const_decl, "Const"));
    const char* label = nullptr;
    ASTNode_get_attr_str(const_decl, "label", &label);
    assert(label != nullptr);

    asm_builder
        .line("%s:", label);

    bool is_array = ASTNode_querySelectorOne(const_decl, "/.[@array]") != nullptr;
    if (!is_array) {
        ASTNode* init_value = ASTNode_querySelectorOne(const_decl, "/ConstInitValue//Number");
        assert(init_value != nullptr);
        int value = -1;
        ASTNode_get_attr_int(init_value, "value", &value);
        asm_builder
            .line("\t.word %d", value);
    } else {
        QueryResult *init_values = ASTNode_querySelector(const_decl, "/ConstInitValue[@array]//Number"), *cur = NULL;
        DL_FOREACH(init_values, cur) {
            int value = -1, repeat = -1;
            ASTNode_get_attr_int(cur->node, "value", &value);
            ASTNode_get_attr_int(cur->node, "repeat", &repeat);
            if (value == 0) {
                asm_builder.line("\t.zero %d", repeat * this->word_size); 
            } else {
                for (int i = 0; i < repeat; i++) {
                    asm_builder.line("\t.word %d", value);
                }
            }
        }
    }

    asm_builder 
        .line("\t.section .rodata")
        .line("\t.align %d", this->word_align)
        .line();
    
}

/**
 * 这个函数用于将常量声明的AST节点转换为汇编代码，同时为每个 ConstDecl 节点生成一个唯一的标签添加到 label 属性中
 */
void ConstInflater::inflate(AssemblyBuilder& asm_builder) {
    QueryResult *consts_decls = ASTNode_querySelector(compunit, "//Const"), *cur = NULL;

    DL_FOREACH(consts_decls, cur) {
        ASTNode *const_decl = (ASTNode *) cur->node;
        // ASTNode_print(const_decl);
        char* label = getLabel(const_decl);
        ASTNode_add_attr_str(const_decl, "label", label);
        // printf("label: %s\n", label);
        inflateConstDecl(const_decl, asm_builder);
        free(label);
    }
}