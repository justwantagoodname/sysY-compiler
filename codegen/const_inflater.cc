#include "codegen/const_inflater.hpp"

char* GlobalDeclInflater::getLabel(ASTNode* decl) {
    assert(ASTNode_id_is(decl, "Const") || ASTNode_id_is(decl, "Var"));
    
    const char* decl_name = nullptr;
    ASTNode_get_attr_str(decl, "name", &decl_name);
    assert(decl_name != nullptr);

    char* name = strdup(decl_name);

    QueryResult *scopes = ASTNode_querySelector(decl, "/ancestor::Scope"), *cur = NULL;
    DL_FOREACH(scopes, cur) {
        const char* scope_name = nullptr;
        ASTNode_get_attr_str(cur->node, "name", &scope_name);
        assert(scope_name != nullptr);
        // concat scope name
        asprintf(&name, "%s_%s", scope_name, name);   
    }
    return name;
}

void GlobalDeclInflater::inflateConstDecl(ASTNode* const_decl, AssemblyBuilder& asm_builder) {
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
void GlobalDeclInflater::constInflate(AssemblyBuilder& asm_builder) {
    QueryResult *consts_decls = ASTNode_querySelector(compunit, "//Const"), *cur = nullptr;

    DL_FOREACH(consts_decls, cur) {
        auto *const_decl = cur->node;
        // ASTNode_print(const_decl);
        char* label = getLabel(const_decl);
        ASTNode_add_attr_str(const_decl, "label", label);
        // printf("label: %s\n", label);
        inflateConstDecl(const_decl, asm_builder);
        free(label);
    }
}

void GlobalDeclInflater::staticVarInflate(AssemblyBuilder &asm_builder) {
    QueryResult *static_vars = ASTNode_querySelector(compunit, "/Scope/Decl/Var"),
                *cur = nullptr;

    DL_FOREACH(static_vars, cur) {
        auto *var_decl = cur->node;
        char* label = getLabel(var_decl);
        ASTNode_add_attr_str(var_decl, "label", label);
        inflateStaticVarDecl(var_decl, asm_builder);
//        ASTNode_print(var_decl);
    }
}

void GlobalDeclInflater::inflateStaticVarDecl(ASTNode *static_var_decl, AssemblyBuilder &asm_builder) {
    assert(ASTNode_id_is(static_var_decl, "Var"));
    const char* label = nullptr;
    ASTNode_get_attr_str(static_var_decl, "label", &label);
    assert(label != nullptr);

    asm_builder
        .line("%s:", label);
    bool is_array = ASTNode_querySelectorOne(static_var_decl, "/.[@array]") != nullptr;
    bool is_inited = ASTNode_querySelectorOne(static_var_decl, "//InitValue[0]") != nullptr;

    if (is_array) {
        if (is_inited) {
            QueryResult *init_values = ASTNode_querySelector(static_var_decl, "/InitValue[@array]//Number"), *cur = NULL;
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
            asm_builder
                .line("\t.section .data");
        } else {
            int array_size = 1;
            QueryResult *array_sizes = ASTNode_querySelector(static_var_decl, "/ArraySize/Dimension//Number"), *cur = nullptr;
            DL_FOREACH(array_sizes, cur) {
                int size = -1;
                ASTNode_get_attr_int(cur->node, "value", &size);
                assert(size != -1);
                array_size *= size;
            }
            asm_builder
                .line("\t.space %d", array_size * this->word_size)
                .line("\t.section .bss");
        }
    } else {
        // 不是数组
        if (is_inited) {
            ASTNode* init_value = ASTNode_querySelectorOne(static_var_decl, "/InitValue//Number");
            assert(init_value != nullptr);
            int value = -1;
            ASTNode_get_attr_int(init_value, "value", &value);
            asm_builder
                .line("\t.word %d", value)
                .line("\t.section .data");
        } else {
            asm_builder
                .line("\t.space %d", this->word_size)
                .line("\t.section .bss");
        }
    }
    asm_builder
        .line("\t.align %d", this->word_align)
        .line();
}
