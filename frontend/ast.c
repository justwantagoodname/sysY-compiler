#include "sysY.h"
#include <stdarg.h>
#include <unistd.h>
#include "ast.h"
#include <sys/types.h>

struct ASTNode *ASTNode_create(const char* id) {
    assert(id != NULL);

    struct ASTNode *node = (struct ASTNode *)calloc(1, sizeof(struct ASTNode));
    node->id = strdup(id);

    return node;
}

ASTAttribute *ASTAttribute_clone(ASTAttribute *attr) {
    assert(attr != NULL);

    ASTAttribute *clone = (ASTAttribute *)calloc(1, sizeof(ASTAttribute));
    clone->key = strdup(attr->key);
    clone->type = attr->type;
    switch (attr->type) {
        case ATTR_TYPE_INT:
            clone->value.int_value = attr->value.int_value;
            break;
        case ATTR_TYPE_FLOAT:
            clone->value.float_value = attr->value.float_value;
            break;
        case ATTR_TYPE_STR:
            clone->value.str_value = strdup(attr->value.str_value);
            break;
        default:
            break;
    }

    return clone;
}

ASTNode *ASTNode_create_attr(const char* id, int attr_count, ...) {
    assert(id != NULL);
    ASTNode* root = ASTNode_create(id);
    va_list args;
    va_start(args, attr_count);
    for (int i = 0; i < attr_count; i++) {
        const char *key = va_arg(args, const char *);
        const char *value = va_arg(args, const char *);
        ASTNode_add_attr_str(root, key, value);
    }
    return root;
}

void ASTNode_add_nchild(ASTNode *parent, int n, ...) {
    assert(parent != NULL);

    va_list args;
    va_start(args, n);

    for (int i = 0; i < n; i++) {
        ASTNode *child = va_arg(args, ASTNode *);
        assert(child != NULL);
        ASTNode_add_child(parent, child);
    }

    va_end(args);
}

// I Love XML. XD
void ASTNode_print_impl(struct ASTNode *node, FILE *stream) {
    assert(node != NULL);

    fprintf(stream, "<%s", node->id);

    // print attributes
    ASTAttribute *attr = NULL;
    for (attr = node->attrs; attr != NULL; attr = attr->hh.next) {
        switch (attr->type) {
            case ATTR_TYPE_INT:
                fprintf(stream, " %s=\"%d\"", attr->key, attr->value.int_value);
                break;
            case ATTR_TYPE_FLOAT:
                fprintf(stream, " %s=\"%f\"", attr->key, attr->value.float_value);
                break;
            case ATTR_TYPE_STR:
                fprintf(stream, " %s=\"%s\"", attr->key, attr->value.str_value);
                break;
            default:
                fprintf(stream, " %s=\"UNKNOWN\" ", attr->key);
                break;
        }
    }

    if (node->children == NULL) {
        fprintf(stream, " />\n");
        return;
    } else {
        fprintf(stream, ">\n");
    }

    // print children
    struct ASTNode *cur = NULL;
    DL_FOREACH(node->children, cur) {
        ASTNode_print_impl(cur, stream);
    }

    fprintf(stream, "</%s>\n", node->id);
}
#ifdef XML_PP
void ASTNode_pretty_print(ASTNode* node) {
    assert(node != NULL);
    
    int fd[2];
    pipe(fd);
    // create a process and call xmllint --format -
    // pip the fd[1] to stdin after fork and exec
    pid_t pid = fork();
    if (pid == 0) {
        // child
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        execlp("xmllint", "xmllint", "--format", "-", NULL);
        perror("execlp");
        exit(1);
    } else {
        close(fd[0]);
        FILE *stream = fdopen(fd[1], "w");
        ASTNode_print_impl(node, stream);
        fclose(stream);
        waitpid(pid, NULL, 0);
    }
}
#endif

void ASTNode_print(struct ASTNode *node) {
    assert(node != NULL);
#ifndef XML_PP
    ASTNode_print_impl(node, stdout);
#else
    ASTNode_pretty_print(node);
#endif
}

void ASTNode_lpush_child(ASTNode *parent, ASTNode *child) {
    assert(parent != NULL && child != NULL);

    child->parent = parent;

    DL_PREPEND(parent->children, child); 
}

void ASTNode_add_child(ASTNode *parent, ASTNode *child) {
    assert(parent != NULL && child != NULL);

    child->parent = parent;

    DL_APPEND(parent->children, child);
}

void _ASTNode_add_attr(ASTNode *node, ASTAttribute *attr) {
    assert(node != NULL && attr != NULL);

    HASH_ADD_STR(node->attrs, key, attr);
}

void ASTNode_add_attr_int(ASTNode *node, const char* key, int value) {
    assert(node != NULL && key != NULL);

    struct ASTAttribute *attr = (struct ASTAttribute *)calloc(1, sizeof(struct ASTAttribute));
    attr->key = strdup(key);
    attr->value.int_value = value;
    attr->type = ATTR_TYPE_INT;

    _ASTNode_add_attr(node, attr);
}

void ASTNode_add_attr_str(ASTNode *node, const char* key, const char* value) {
    assert(node != NULL && key != NULL && value != NULL);

    struct ASTAttribute *attr = (struct ASTAttribute *)calloc(1, sizeof(struct ASTAttribute));
    attr->key = strdup(key);
    attr->value.str_value = strdup(value);
    attr->type = ATTR_TYPE_STR;

    _ASTNode_add_attr(node, attr);
}

void ASTNode_add_attr_float(ASTNode *node, const char* key, float value) {
    assert(node != NULL && key != NULL);

    struct ASTAttribute *attr = (struct ASTAttribute *)calloc(1, sizeof(struct ASTAttribute));
    attr->key = strdup(key);
    attr->value.float_value = value;
    attr->type = ATTR_TYPE_FLOAT;

    _ASTNode_add_attr(node, attr);
}

ASTAttribute *_ASTNode_get_attr_or_null(ASTNode *node, const char* key) {
    assert(node != NULL && key != NULL);

    struct ASTAttribute *attr = NULL;
    HASH_FIND_STR(node->attrs, key, attr);
    return attr;
}

/**
 * Retrieves an integer attribute value from the given ASTNode.
 *
 * @param node The ASTNode from which to retrieve the attribute.
 * @param key The key of the attribute to retrieve.
 * @param value A pointer to store the retrieved integer value.
 * @return true if the attribute was found and retrieved successfully, false otherwise.
 */
bool ASTNode_get_attr_int(ASTNode *node, const char* key, int *value) {
    assert(node != NULL && key != NULL && value != NULL);

    struct ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL) {
        *value = attr->value.int_value;
        return true;
    } else {
        return false;
    }
}

/**
 * Retrieves a string attribute value from the given ASTNode.
 *
 * @param node The ASTNode from which to retrieve the attribute.
 * @param key The key of the attribute to retrieve.
 * @param value A pointer to store the retrieved string value.
 * @return true if the attribute was found and retrieved successfully, false otherwise.
 */
bool ASTNode_get_attr_str(ASTNode *node, const char* key, const char **value) {
    assert(node != NULL && key != NULL && value != NULL);

    struct ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL) {
        *value = attr->value.str_value;
        return true;
    } else {
        return false;
    }
}

/**
 * Retrieves a float attribute value from the given ASTNode.
 *
 * @param node The ASTNode from which to retrieve the attribute.
 * @param key The key of the attribute to retrieve.
 * @param value A pointer to store the retrieved float value.
 * @return true if the attribute was found and retrieved successfully, false otherwise.
 */

bool ASTNode_get_attr_float(ASTNode *node, const char* key, float *value) {
    assert(node != NULL && key != NULL && value != NULL);

    struct ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL) {
        *value = attr->value.float_value;
        return true;
    } else {
        return false;
    }
}

bool ASTNode_id_is(ASTNode *node, const char* id) {
    assert(node != NULL && id != NULL);

    return strcmp(node->id, id) == 0;
}

/**
 * @brief Move children from -> to, No node construction in procedure.
 * 
 * @param from From Node children will be empty after move.
 * @param to To Node
 */
void ASTNode_move_children(ASTNode *from, ASTNode *to) {
    assert(from != NULL && to != NULL);

    struct ASTNode *cur = NULL;
    DL_FOREACH(from->children, cur) {
        cur->parent = to;
    }

    DL_CONCAT(to->children, from->children);
    from->children = NULL;
}

void ASTNode_copy_children(ASTNode *from, ASTNode *to) {
    assert(from != NULL && to != NULL);
    ASTNode *cur = NULL;
    DL_FOREACH(from->children, cur) {
        ASTNode* cloned = ASTNode_clone(cur);
        DL_APPEND(to->children, cloned);
    }
}

ASTNode *ASTNode_clone(ASTNode *node) {
    assert(node != NULL);

    ASTNode* ret = ASTNode_create(node->id);

    ASTAttribute *attr = NULL, *tmp = NULL;
    HASH_ITER(hh, node->attrs, attr, tmp) {
        ASTAttribute *clone_attr = ASTAttribute_clone(attr);
        HASH_ADD_STR(ret->attrs, key, clone_attr);    
    }

    ASTNode* cur = NULL;
    DL_FOREACH(node->children, cur) {
        ASTNode* cloned_child = ASTNode_clone(cur);
        ASTNode_add_child(ret, cloned_child);
    }
    return ret;
}

void ASTNode_free(ASTNode *node) {
    assert(node != NULL);

    struct ASTAttribute *attr = NULL, *tmp = NULL;
    HASH_ITER(hh, node->attrs, attr, tmp) {
        free((char *)attr->key);
        if (attr->type == ATTR_TYPE_STR) {
            free((char *)attr->value.str_value);
        }
        free(attr);
        HASH_DEL(node->attrs, attr);
    }

    struct ASTNode *cur, *tmp2;
    DL_FOREACH_SAFE(node->children, cur, tmp2) {
        ASTNode_free(cur);
    }

    free((char *)node->id);
    free(node);
}

bool ASTNode_has_attr(ASTNode *node, const char* key) {
    assert(node != NULL && key != NULL);

    ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    return attr != NULL;
}

bool ASTNode_attr_eq_int(ASTNode *node, const char* key, int value) {
    assert(node != NULL && key != NULL);

    ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL && attr->type == ATTR_TYPE_INT) {
        return attr->value.int_value == value;
    } else {
        return false;
    } 
}

bool ASTNode_attr_eq_str(ASTNode *node, const char* key, const char* value) {
    assert(node != NULL && key != NULL && value != NULL);

    ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL && attr->type == ATTR_TYPE_STR) {
        return strcmp(attr->value.str_value, value) == 0;
    } else {
        return false;
    }
}
bool ASTNode_attr_eq_float(ASTNode *node, const char* key, float value) {
    assert(node != NULL && key != NULL);

    ASTAttribute *attr = _ASTNode_get_attr_or_null(node, key);
    if (attr != NULL && attr->type == ATTR_TYPE_FLOAT) {
        return attr->value.float_value == value;
    } else {
        return false;
    }
}