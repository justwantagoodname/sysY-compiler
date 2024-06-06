#include "sysY.h"
#include "ast.h"

#include <stdarg.h>


ASTAttribute* _ASTNode_get_attr_or_null(ASTNode* node, const char* key);
/// <summary>
/// token½ÚµãÔªËØ
/// </summary>
class Element {
private:
	ASTNode* node;
public:

	//ASTNode* ASTNode_create(const char* id);
	//ASTNode* ASTNode_create_attr(const char* id, int attr_count, ...);
	///* Children */
	//void ASTNode_add_nchild(ASTNode* parent, int n, ...);
	//void ASTNode_lpush_child(ASTNode* parent, ASTNode* child);
	//void ASTNode_add_child(ASTNode* parent, ASTNode* child);
	///* Attributes */
	//void ASTNode_add_attr_int(ASTNode* node, const char* key, int value);
	//void ASTNode_add_attr_str(ASTNode* node, const char* key, const char* value);
	//void ASTNode_add_attr_float(ASTNode* node, const char* key, float value);
	//bool ASTNode_get_attr_int(ASTNode* node, const char* key, int* value);
	//bool ASTNode_get_attr_str(ASTNode* node, const char* key, const char** value);
	//bool ASTNode_get_attr_float(ASTNode* node, const char* key, float* value);
	//bool ASTNode_attr_eq_int(ASTNode* node, const char* key, int value);
	//bool ASTNode_attr_eq_str(ASTNode* node, const char* key, const char* value);
	//bool ASTNode_attr_eq_float(ASTNode* node, const char* key, float value);
	///* Utils */
	//void ASTNode_print(struct ASTNode* node);
	//void ASTNode_move_children(ASTNode* from, ASTNode* to);
	//void ASTNode_copy_children(ASTNode* from, ASTNode* to);
	//ASTNode* ASTNode_clone(ASTNode* node);
	//void ASTNode_free(ASTNode* node);
	///* Operators */
	//bool ASTNode_id_is(ASTNode* node, const char* id);

	//QueryResult* QueryResult_create(ASTNode* node);
	//QueryResult* ASTNode_querySelector(ASTNode* node, const char* selector);
	//ASTNode* ASTNode_querySelectorOne(ASTNode* node, const char* selector);

	Element(ASTNode* n) {
		node = n;
	}

	Element(const Element&& e) {
		node = e.node;
	}

	Element(const char* id) {
		node = ASTNode_create(id);
	}

	Element(const char* id, int attr_count, ...) {
		assert(id != NULL);
		ASTNode* root = ASTNode_create(id);
		va_list args;
		va_start(args, attr_count);
		for (int i = 0; i < attr_count; i++) {
			const char* key = va_arg(args, const char*);
			const char* value = va_arg(args, const char*);
			ASTNode_add_attr_str(root, key, value);
		}
		node = root;
	}

	operator ASTNode* () const {
		return node;
	}

	Element& operator+=(const Element&& e) {
		ASTNode_add_child(node, e.node);
		return *this;
	}

	Element& operator+=(ASTNode* e) {
		ASTNode_add_child(node, e);
		return *this;
	}

	ASTAttribute& operator[](const char* key) const {
		return *_ASTNode_get_attr_or_null(node, key);
	}

	Element operator[](int index) const {
		ASTNode* child = node->children;
		int i = 0;
		while (i < index) {
			if (child == NULL)
				return Element(child);
			child = child->next;
			++i;
		}
		return child;
	}

	QueryResult* operator()(const char* key) const {
		return ASTNode_querySelector(node, key);
	}

	Element operator%(const char* key)const {
		return ASTNode_querySelectorOne(node, key);
	}

	QueryResult* q(const char* key) const{
		return ASTNode_querySelector(node, key);
	}

	Element qo(const char* key) const {
		return ASTNode_querySelectorOne(node, key);
	}

	Element at(int index) const {
		ASTNode* child = node->children;
		int i = 0;
		while (i < index) {
			if (child == NULL)
				throw "Error";
			child = child->next;
			++i;
		}
		return child;
	}

	void add_child(int n, ...) {
		assert(node != NULL);

		va_list args;
		va_start(args, n);

		for (int i = 0; i < n; i++) {
			ASTNode* child = va_arg(args, ASTNode*);
			assert(child != NULL);
			ASTNode_add_child(node, child);
		}

		va_end(args);
	}

	void add_child(ASTNode* child) {
		ASTNode_add_child(node, child);
	}

	void add_child(const Element&& child) {
		ASTNode_add_child(node, child.node);
	}

	void lpush_child(ASTNode* child) {
		ASTNode_lpush_child(node, child);
	}

	void lpush_child(const Element&& child) {
		ASTNode_lpush_child(node, child.node);
	}

	void add_attr(const char* key, int value) {
		ASTNode_add_attr_int(node, key, value);
	}

	void add_attr(const char* key, float value) {
		ASTNode_add_attr_float(node, key, value);
	}

	void add_attr(const char* key, double value) {
		ASTNode_add_attr_float(node, key, value);
	}

	void add_attr(const char* key, const char* value) {
		ASTNode_add_attr_str(node, key, value);
	}

	bool get_attr(const char* key, int* value) {
		return ASTNode_get_attr_int(node, key, value);
	}

	bool get_attr(const char* key, float* value) {
		return ASTNode_get_attr_float(node, key, value);
	}

	bool get_attr(const char* key, const char** value) {
		return ASTNode_get_attr_str(node, key, value);
	}

	ASTAttribute* get_attr(const char* key) {
		return _ASTNode_get_attr_or_null(node, key);
	}

	int get_attr_int(const char* key) {
		int* value;
		ASTNode_get_attr_int(node, key, value);
		return *value;
	}

	float get_attr_float(const char* key) {
		float* value;
		ASTNode_get_attr_float(node, key, value);
		return *value;
	}

	const char* get_attr_str(const char* key) {
		const char** value;
		ASTNode_get_attr_str(node, key, value);
		return *value;
	}

	bool attr_eq(const char* key, int value) {
		return ASTNode_attr_eq_int(node, key, value);
	}

	bool attr_eq(const char* key, float value) {
		return ASTNode_attr_eq_float(node, key, value);
	}

	bool attr_eq(const char* key, bool value) {
		return ASTNode_attr_eq_float(node, key, value);
	}
	bool attr_eq(const char* key, const char* value) {
		return ASTNode_attr_eq_str(node, key, value);
	}

	void print() {
		ASTNode_print(node);
	}

	void move_children_to(ASTNode* to) {
		ASTNode_move_children(node, to);
	}

	void move_children_from(ASTNode* from) {
		ASTNode_move_children(from, node);
	}

	void copy_children_by(ASTNode* from) {
		ASTNode_copy_children(from, node);
	}

	Element clone() {
		return ASTNode_clone(node);
	}

	bool id_is(const char* id) {
		return ASTNode_id_is(node, id);
	}

	void free() {
		ASTNode_free(node);
	}

	QueryResult* createQueryResult() {
		return QueryResult_create(node);
	}

	QueryResult* querySelector(const char* selector) {
		return ASTNode_querySelector(node, selector);
	}

	Element querySelectorOne(const char* selector) {
		return ASTNode_querySelectorOne(node, selector);
	}

};