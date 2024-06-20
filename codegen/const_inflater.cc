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
        .line("\t.type %s, %%object", label)
        .line("\t.section .rodata")
        .line("\t.align %d", this->word_align)
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

    bool is_array = ASTNode_querySelectorOne(static_var_decl, "/.[@array]") != nullptr;
    bool is_inited = ASTNode_querySelectorOne(static_var_decl, "//InitValue[0]") != nullptr;

    asm_builder
        .line("\t.type %s, %%object", label)
        .line("\t.section .%s", is_inited ? "data" : "bss")
        .line("\t.align %d", this->word_align)
        .line("%s:", label);

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
                .line("\t.space %d", array_size * this->word_size);
        }
    } else {
        // 不是数组
        if (is_inited) {
            ASTNode* init_value = ASTNode_querySelectorOne(static_var_decl, "/InitValue//Number");
            assert(init_value != nullptr);
            int value = -1;
            ASTNode_get_attr_int(init_value, "value", &value);
            asm_builder
                .line("\t.word %d", value);
        } else {
            asm_builder
                .line("\t.space %d", this->word_size);
        }
    }
    asm_builder
        .line();
}

void GlobalDeclInflater::stringInflate(AssemblyBuilder &asm_builder) {
    QueryResult *strings = ASTNode_querySelector(compunit, "//Param[@type='StringConst']"),
            *cur = nullptr;

    DL_FOREACH(strings, cur) {
        auto *str_decl = cur->node;
        char* label = getStrLabel(str_decl);
        ASTNode_add_attr_str(str_decl, "label", label);
        inflateStringConst(str_decl, asm_builder);
//        ASTNode_print(var_decl);
    }
}

void GlobalDeclInflater::inflateStringConst(ASTNode *string_const, AssemblyBuilder &asm_builder) {
    assert(ASTNode_id_is(string_const, "Param"));
    assert(ASTNode_has_attr(string_const, "type"));
    const char* type_name;
    ASTNode_get_attr_str(string_const, "type", &type_name);
    assert(strcmp(type_name, "StringConst") == 0);

    assert(ASTNode_has_attr(string_const, "label"));

    const char* label_name;
    ASTNode_get_attr_str(string_const, "label", &label_name);

    assert(ASTNode_has_attr(string_const, "value"));

    const char* str_literal;
    ASTNode_get_attr_str(string_const, "value", &str_literal);

    asm_builder
        .line("\t.type %s, %%object", label_name)
        .line("\t.section .rodata")
        .line("\t.align %d", this->word_align)
        .line("%s:", label_name)
        .line("\t.asciz \"%s\"", str_literal)
        .line();
}

char *GlobalDeclInflater::getStrLabel([[maybe_unused]] ASTNode *decl) {
    static int str_count = 0;
    char *ret = NULL;
    asprintf(&ret, "STR%d", str_count++);
    return ret;
}
