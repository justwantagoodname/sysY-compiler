﻿#ifndef TRIPLE_H
#define TRIPLE_H
#include "Element.h"
#include <vector>

class Triples {
private:
	Element root;

private:
	constexpr static class CMD {
	public:
		enum CMD_ENUM {
			mov,
			jmp,
			call,
			read,
			reada,
			write,
			imdd,
			jeq,
			jne,
			jgt,
			jlt,
			jge,
			jle,
			ret,
			rev,
			pux,
			pus,
			add,
			sub,
			mul,
			div,
			mod,
			tag,
		};
	}Cmd = CMD();

private:

	struct IntTriple {
		int cmd;
		int e1, e2, to;

		IntTriple(int, int, int, int);
	};

	std::vector<IntTriple> triples;
	std::vector<Element> value_pointer;
	std::vector<Element> function_pointer;
	//std::vector<Element> value_table;
	//std::vector<int> page_stack;

	void add(int, int, int, int);
	int find(int, int, int, int) const;
	int find(const Element& e);
	int pushf(const Element& e);
	int findf(const Element& e);
	int findf(const char* name);

public:
	Triples(const Element&);
	Triples(const Triples&) = delete;

	void pretreat();
	void make();

	IntTriple& operator[](int idx) {
		return triples[idx];
	};
	size_t size();

	std::vector<IntTriple>::iterator begin() { return triples.begin(); }
	std::vector<IntTriple>::iterator end() { return triples.end(); }

	void print() const;

};
#endif //TRIPLE_H