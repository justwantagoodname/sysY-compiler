#ifndef TRIPLE_H
#define TRIPLE_H
#include "Element.h"
#include <vector>

class Triples {
private:
	Element root;
	
private:
	class {
	public:
		enum {
			mov,
			jmp,
			call,
			read,
			write,
			add,
			imdd,
			jeq,
			jne,
			jgt,
			jlt,
			jge,
			jle,

		};
	} Cmd;

	struct IntTriple {
		int cmd;
		int e1, e2, to;

		IntTriple(int, int, int, int);
	};

	std::vector<IntTriple> triples;
	std::vector<Element> value_pointer;

	void add(int, int, int, int);
	int find(int, int, int, int)const;
	int find(const Element);

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