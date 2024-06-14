#ifndef ELEMENT_H
#define ELEMENT_H

#include "sysY.h"
#include "ast.h"

#include "Query.h"

class Element {
private:
	ASTNode* node;

public:
	bool flag = false;

public:
	Element();
	static Element CreateByFile(const char* filename);
	Element(ASTNode* n);
	Element(ASTNode* n, bool f);
	Element(const Element& e);
	Element(const char* id);
	Element(const char* id, int attr_count, ...);
	operator ASTNode* () const;
	operator bool() const;
	bool operator==(const Element& e) const;
	Element& operator+=(const Element& e);
	Element& operator+=(ASTNode* e);
	ASTAttribute& operator[](const char* key) const;
	Element operator[](int index) const;
	Query operator()(const char* key) const;
	Element operator%(const char* key) const;
	Query q(const char* key) const;
	Element qo(const char* key) const;
	Element at(int index) const;
	Element& add_child(int n, ...);
	Element& add_child(ASTNode* child);
	Element& add_child(const Element&& child);
	Element& lpush_child(ASTNode* child);
	Element& lpush_child(const Element&& child);
	Element& add_attr(const char* key, size_t value);
	Element& add_attr(const char* key, int value);
	Element& add_attr(const char* key, float value);
	Element& add_attr(const char* key, double value);
	Element& add_attr(const char* key, const char* value);
	bool get_attr(const char* key, int* value) const;
	bool get_attr(const char* key, float* value) const;
	bool get_attr(const char* key, const char** value) const;
	ASTAttribute* get_attr(const char* key) const;
	int get_attr_int(const char* key) const;
	float get_attr_float(const char* key) const;
	const char* get_attr_str(const char* key) const;
	bool attr_eq(const char* key, int value) const;
	bool attr_eq(const char* key, float value) const;
	bool attr_eq(const char* key, bool value) const;
	bool attr_eq(const char* key, const char* value) const;
	void print() const;
	Element& move_children_to(ASTNode* to);
	Element& move_children_from(ASTNode* from);
	Element& copy_children_by(ASTNode* from);
	Element clone() const;
	const char* id() const;
	bool id_is(const char* id) const;
	void free();
	Query createQueryResult() const;
	Query querySelector(const char* selector) const;
	Element querySelectorOne(const char* selector) const;

	Element table(const char* key) const;
	size_t size() const;

	struct Iter {
		ASTNode* it;
		bool flag = false;
		Iter(ASTNode* q);
		Iter& operator ++();
		bool operator!=(const Iter& other)const;
		Element operator*();
		operator Element();
	};

	Iter begin() const;
	Iter end() const;

	ASTNode* unwrap() const;
};

#endif // ELEMENT_H