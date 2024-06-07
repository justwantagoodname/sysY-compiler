#include <stdarg.h>

#include "Element.h"
#include "Query.h"

extern "C" {
    ASTAttribute* _ASTNode_get_attr_or_null(ASTNode* node, const char* key);
}
extern "C" {
	extern int yyparse(struct ASTNode** root);
	extern FILE* yyin;
}

Element::Element() : node(nullptr) {}

Element Element::CreateByFile(const char* filename) {
    yyin = fopen(filename, "r");
    ASTNode* node = nullptr;
    int i = yyparse(&node);
    if (i != 0) {
        node = nullptr;
        throw "Error";
    }
    return Element(node);
}

Element::Element(ASTNode* n) : node(n) {}

Element::Element(const Element&& e) : node(e.node) {}

Element::Element(const char* id) {
    node = ASTNode_create(id);
}

Element::Element(const char* id, int attr_count, ...) {
    assert(id != nullptr);
    ASTNode* root = ASTNode_create(id);
    va_list args;
    va_start(args, attr_count);
    for (int i = 0; i < attr_count; i++) {
        const char* key = va_arg(args, const char*);
        const char* value = va_arg(args, const char*);
        ASTNode_add_attr_str(root, key, value);
    }
    va_end(args);
    node = root;
}

Element::operator ASTNode* () const {
    return node;
}

Element& Element::operator+=(const Element&& e) {
    ASTNode_add_child(node, e.node);
    return *this;
}

Element& Element::operator+=(ASTNode* e) {
    ASTNode_add_child(node, e);
    return *this;
}

ASTAttribute& Element::operator[](const char* key) const {
    return *_ASTNode_get_attr_or_null(node, key);
}

Element Element::operator[](int index) const {
    ASTNode* child = node->children;
    int i = 0;
    while (i < index) {
        if (child == nullptr)
            return Element(child);
        child = child->next;
        ++i;
    }
    return child;
}

Query Element::operator()(const char* key) const {
    return ASTNode_querySelector(node, key);
}

Element Element::operator%(const char* key) const {
    return ASTNode_querySelectorOne(node, key);
}

Query Element::q(const char* key) const {
    return ASTNode_querySelector(node, key);
}

Element Element::qo(const char* key) const {
    return ASTNode_querySelectorOne(node, key);
}

Element Element::at(int index) const {
    ASTNode* child = node->children;
    int i = 0;
    while (i < index) {
        if (child == nullptr)
            throw "Error";
        child = child->next;
        ++i;
    }
    return child;
}

void Element::add_child(int n, ...) {
    assert(node != nullptr);
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        ASTNode* child = va_arg(args, ASTNode*);
        assert(child != nullptr);
        ASTNode_add_child(node, child);
    }
    va_end(args);
}

void Element::add_child(ASTNode* child) {
    ASTNode_add_child(node, child);
}

void Element::add_child(const Element&& child) {
    ASTNode_add_child(node, child.node);
}

void Element::lpush_child(ASTNode* child) {
    ASTNode_lpush_child(node, child);
}

void Element::lpush_child(const Element&& child) {
    ASTNode_lpush_child(node, child.node);
}

void Element::add_attr(const char* key, int value) {
    ASTNode_add_attr_int(node, key, value);
}

void Element::add_attr(const char* key, float value) {
    ASTNode_add_attr_float(node, key, value);
}

void Element::add_attr(const char* key, double value) {
    ASTNode_add_attr_float(node, key, value);
}

void Element::add_attr(const char* key, const char* value) {
    ASTNode_add_attr_str(node, key, value);
}

bool Element::get_attr(const char* key, int* value) {
    return ASTNode_get_attr_int(node, key, value);
}

bool Element::get_attr(const char* key, float* value) {
    return ASTNode_get_attr_float(node, key, value);
}

bool Element::get_attr(const char* key, const char** value) {
    return ASTNode_get_attr_str(node, key, value);
}

ASTAttribute* Element::get_attr(const char* key) {
    return _ASTNode_get_attr_or_null(node, key);
}

int Element::get_attr_int(const char* key) {
    int* value;
    ASTNode_get_attr_int(node, key, value);
    return *value;
}

float Element::get_attr_float(const char* key) {
    float* value;
    ASTNode_get_attr_float(node, key, value);
    return *value;
}

const char* Element::get_attr_str(const char* key) {
    const char** value;
    ASTNode_get_attr_str(node, key, value);
    return *value;
}

bool Element::attr_eq(const char* key, int value) {
    return ASTNode_attr_eq_int(node, key, value);
}

bool Element::attr_eq(const char* key, float value) {
    return ASTNode_attr_eq_float(node, key, value);
}

bool Element::attr_eq(const char* key, bool value) {
    return ASTNode_attr_eq_float(node, key, value);
}

bool Element::attr_eq(const char* key, const char* value) {
    return ASTNode_attr_eq_str(node, key, value);
}

void Element::print() {
    ASTNode_print(node);
}

void Element::move_children_to(ASTNode* to) {
    ASTNode_move_children(node, to);
}

void Element::move_children_from(ASTNode* from) {
    ASTNode_move_children(from, node);
}

void Element::copy_children_by(ASTNode* from) {
    ASTNode_copy_children(from, node);
}

Element Element::clone() {
    return ASTNode_clone(node);
}

bool Element::id_is(const char* id) {
    return ASTNode_id_is(node, id);
}

void Element::free() {
    ASTNode_free(node);
}

Query Element::createQueryResult() {
    return QueryResult_create(node);
}

Query Element::querySelector(const char* selector) {
    return ASTNode_querySelector(node, selector);
}

Element Element::querySelectorOne(const char* selector) {
    return ASTNode_querySelectorOne(node, selector);
}