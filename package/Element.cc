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

Element::Element(ASTNode* n, bool f): node(n), flag(f) {}

Element::Element(const Element& e) : node(e.node) {}

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

Element& Element::add_child(int n, ...) {
	assert(node != nullptr);
	va_list args;
	va_start(args, n);
	for (int i = 0; i < n; i++) {
		ASTNode* child = va_arg(args, ASTNode*);
		assert(child != nullptr);
		ASTNode_add_child(node, child);
	}
	va_end(args);
	return *this;
}

Element& Element::add_child(ASTNode* child) {
	ASTNode_add_child(node, child);
	return *this;
}

Element& Element::add_child(const Element&& child) {
	ASTNode_add_child(node, child.node);
	return *this;
}

Element& Element::lpush_child(ASTNode* child) {
	ASTNode_lpush_child(node, child);
	return *this;
}

Element& Element::lpush_child(const Element&& child) {
	ASTNode_lpush_child(node, child.node);
	return *this;
}

Element& Element::add_attr(const char* key, int value) {
	ASTNode_add_attr_int(node, key, value);
	return *this;
}

Element& Element::add_attr(const char* key, float value) {
	ASTNode_add_attr_float(node, key, value);
	return *this;
}

Element& Element::add_attr(const char* key, double value) {
	ASTNode_add_attr_float(node, key, value);
	return *this;
}

Element& Element::add_attr(const char* key, const char* value) {
	ASTNode_add_attr_str(node, key, value);
	return *this;
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

Element& Element::print() {
	ASTNode_print(node);
	return *this;
}

Element& Element::move_children_to(ASTNode* to) {
	ASTNode_move_children(node, to);
	return *this;
}

Element& Element::move_children_from(ASTNode* from) {
	ASTNode_move_children(from, node);
	return *this;
}

Element& Element::copy_children_by(ASTNode* from) {
	ASTNode_copy_children(from, node);
	return *this;
}

Element Element::clone() {
	return ASTNode_clone(node);
}

const char* Element::id()
{
	return node->id;
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

Element Element::table(const char* key)
{
	char* selector = (char*)malloc(strlen(key) + 25);
	if (!selector)return (ASTNode*)(NULL);
	sprintf(selector, "ancestor::Scope/Decl/*[@name='%s']", key);
	Element e = this->qo(selector);
	::free(selector);
	return e;
}

Element::iter Element::begin()
{
	return Element::iter(node);
}

Element::iter Element::end()
{
	return Element::iter(node -> parent);
}

Element::iter::iter(ASTNode* q) : it(q) {}

Element::iter& Element::iter::operator++() {

	if (it->children && !flag) { //如果有叶子节点并且为进入
		it = it->children;
		flag = false;
	}
	else if (it->next) { //如果有下一项兄弟节点
		it = it->next;
		flag = false;
	}
	else if (it->parent) { //如果有父节点
		it = it->parent;
		flag = true;
	}
	else {
		it = NULL;
		flag = true;
	}
	return *this;

}

bool Element::iter::operator!=(Element::iter& other) {
	return it != other.it;
}

Element Element::iter::operator*() {
	return *this;
}

Element::iter::operator Element()
{
	return Element(it, flag);
}
