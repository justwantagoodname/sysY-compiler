#include <stdarg.h>

#include "element.h"
#include "query.h"

extern int yyparse(struct ASTNode** root);
extern FILE* yyin;

Element::Element() : node(nullptr) {}

Element Element::CreateByFile(const char* filename) {
	yyin = fopen(filename, "r");
	assert(yyin != nullptr);
	ASTNode* node = nullptr;
	int status = yyparse(&node);

	
	assert(status == 0);

	if (status != 0) {
		node = nullptr;
	}
	
	return Element(node);
}

Element::Element(ASTNode* n) : node(n) {}

Element::Element(ASTNode* n, bool f) : node(n), flag(f) {}

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

Element::operator bool() const
{
	return node;
}

bool Element::operator==(const Element& e) const
{
	return node == e.node;
}

Element& Element::operator+=(const Element& e) {
	ASTNode_add_child(node, e.node);
	return *this;
}

Element& Element::operator+=(ASTNode* e) {
	ASTNode_add_child(node, e);
	return *this;
}

ASTAttribute& Element::operator[](const char* key) const {
	return *ASTNode_get_attr_or_null(node, key);
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

//Query Element::operator()(const char* key) const {
//	return ASTNode_querySelector(node, key);
//}

Query Element::operator()(const char* fmt, ...) const {
	va_list args;
	va_start(args, fmt);
	char* selector = NULL;
	int ret = vasprintf(&selector, fmt, args);
	va_end(args);
	QueryResult* result = ASTNode_querySelector(node, selector);
	::free(selector);
	return result;
}

Element Element::operator%(const char* key) const {
	return ASTNode_querySelectorOne(node, key);
}

Query Element::q(const char* fmt, ...) const {
	va_list args;
	va_start(args, fmt);
	char* selector = NULL;
	int ret = vasprintf(&selector, fmt, args);
	va_end(args);
	QueryResult* result = ASTNode_querySelector(node, selector);
	::free(selector);
	return result;
}

//Query Element::q(const char* key) const {
//	return ASTNode_querySelector(node, key);
//}
//
//Element Element::qo(const char* key) const {
//	return ASTNode_querySelectorOne(node, key);
//}

Element Element::qo(const char* fmt, ...) const {
	va_list args;
	va_start(args, fmt);
	char* selector = NULL;
	int ret = vasprintf(&selector, fmt, args);
	va_end(args);
	ASTNode* result = ASTNode_querySelectorOne(node, selector);
	::free(selector);
	return result;
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

Element& Element::add_childn(int n, ...) {
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

Element& Element::lpush_child(ASTNode* child) {
	ASTNode_lpush_child(node, child);
	return *this;
}

Element& Element::lpush_child(const Element&& child) {
	ASTNode_lpush_child(node, child.node);
	return *this;
}

Element& Element::add_attr(const char* key, size_t value)
{
	ASTNode_add_attr_int(node, key, value);
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

bool Element::get_attr(const char* key, int* value) const {
	return ASTNode_get_attr_int(node, key, value);
}

bool Element::get_attr(const char* key, float* value) const {
	return ASTNode_get_attr_float(node, key, value);
}

bool Element::get_attr(const char* key, const char** value) const {
	return ASTNode_get_attr_str(node, key, value);
}

ASTAttribute* Element::get_attr(const char* key) const {
	return ASTNode_get_attr_or_null(node, key);
}

int Element::get_attr_int(const char* key) const {
	int value;
	bool flg = ASTNode_get_attr_int(node, key, &value);
	assert(flg);
	return value;
}

float Element::get_attr_float(const char* key) const {
	float value;
	bool flg = ASTNode_get_attr_float(node, key, &value);
	assert(flg);
	return value;
}

const char* Element::get_attr_str(const char* key) const {
	const char* value;
	bool flg = ASTNode_get_attr_str(node, key, &value);
	assert(flg);
	return value;
}

bool Element::attr_eq(const char* key, int value) const {
	return ASTNode_attr_eq_int(node, key, value);
}

bool Element::attr_eq(const char* key, float value) const {
	return ASTNode_attr_eq_float(node, key, value);
}

bool Element::attr_eq(const char* key, bool value) const {
	return ASTNode_attr_eq_float(node, key, value);
}

bool Element::attr_eq(const char* key, const char* value) const {
	return ASTNode_attr_eq_str(node, key, value);
}

void Element::print() const {
	ASTNode_print(node);
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

Element Element::clone() const {
	return ASTNode_clone(node);
}

const char* Element::id() const
{
	return node->id;
}

bool Element::id_is(const char* id) const {
	return ASTNode_id_is(node, id);
}

void Element::free() {
	ASTNode_free(node);
}

Query Element::createQueryResult() const {
	return QueryResult_create(node);
}

Query Element::querySelector(const char* selector) const {
	return ASTNode_querySelector(node, selector);
}

Element Element::querySelectorOne(const char* selector) const {
	return ASTNode_querySelectorOne(node, selector);
}

Query Element::querySelectorf(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	char* selector = NULL;
	int ret = vasprintf(&selector, fmt, args);
	va_end(args);
	QueryResult* result = ASTNode_querySelector(node, selector);
	::free(selector);
	return result;
}

Element Element::querySelectorOnef(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	char* selector = NULL;
	int ret = vasprintf(&selector, fmt, args);
	va_end(args);
	ASTNode* result = ASTNode_querySelectorOne(node, selector);
	::free(selector);
	return result;
}

Element Element::table(const char* key) const
{
	Element e = this->qo("ancestor::Scope/Decl/*[@name='%s',@define='true']", key);
	return e;
}

size_t Element::size() const
{
	size_t count = 0;
	if (node->children == NULL)
		return 0;
	DL_COUNT(node->children, node->children->prev, count);
	return count;
}

Element::Iter Element::dfsbegin() const
{
	return Element::Iter(node);
}

Element::Iter Element::dfsend() const
{
	return Element::Iter(node, true);
}

Element::Iter Element::begin() const
{
	return Element::Iter(node->children);
}

Element::Iter Element::end() const
{
	return nullptr;
}

Element::Iter::Iter(ASTNode* q) : it(q) {}

Element::Iter::Iter(ASTNode* q, bool f) : it(q), flag(f) {}

Element::Iter& Element::Iter::next() {

	if (it->children && !flag) { 
		it = it->children;
		flag = false;
	}
	else if (it->next) {
		it = it->next;
		flag = false;
	}
	else if (it->parent) {
		it = it->parent;
		flag = true;
	}
	else {
		it = NULL;
		flag = true;
	}

	if (it && !(it->children))
		flag = true;

	return *this;

}

Element::Iter& Element::Iter::operator++() {
	it = it->next;
	return *this;
}

bool Element::Iter::operator!=(const Element::Iter& other)const {
	return it != other.it || flag != other.flag;
}

Element Element::Iter::operator*() {
	return *this;
}

Element::Iter::operator Element()
{
	return Element(it, flag);
}

ASTNode* Element::unwrap() const {
	return node;
}

ASTNode* Element::children() const
{
	return node->children;
}
