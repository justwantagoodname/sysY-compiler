#ifndef ELEMENT_H
#define ELEMENT_H

#include "sysY.h"
#include "ast.h"

#include "Query.h"

class Element {
private:
	ASTNode* node;

public:
	Element();
	static Element CreateByFile(const char* filename);
	Element(ASTNode* n);
	Element(const Element&& e);
	Element(const char* id);
	Element(const char* id, int attr_count, ...);
	operator ASTNode* () const;
	Element& operator+=(const Element&& e);
	Element& operator+=(ASTNode* e);
	ASTAttribute& operator[](const char* key) const;
	Element operator[](int index) const;
	Query operator()(const char* key) const;
	Element operator%(const char* key) const;
	Query q(const char* key) const;
	Element qo(const char* key) const;
	Element at(int index) const;
	void add_child(int n, ...);
	void add_child(ASTNode* child);
	void add_child(const Element&& child);
	void lpush_child(ASTNode* child);
	void lpush_child(const Element&& child);
	void add_attr(const char* key, int value);
	void add_attr(const char* key, float value);
	void add_attr(const char* key, double value);
	void add_attr(const char* key, const char* value);
	bool get_attr(const char* key, int* value);
	bool get_attr(const char* key, float* value);
	bool get_attr(const char* key, const char** value);
	ASTAttribute* get_attr(const char* key);
	int get_attr_int(const char* key);
	float get_attr_float(const char* key);
	const char* get_attr_str(const char* key);
	bool attr_eq(const char* key, int value);
	bool attr_eq(const char* key, float value);
	bool attr_eq(const char* key, bool value);
	bool attr_eq(const char* key, const char* value);
	void print();
	void move_children_to(ASTNode* to);
	void move_children_from(ASTNode* from);
	void copy_children_by(ASTNode* from);
	Element clone();
	bool id_is(const char* id);
	void free();
	Query createQueryResult();
	Query querySelector(const char* selector);
	Element querySelectorOne(const char* selector);
    ASTNode *unwrap();
};

#endif // ELEMENT_H