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

	void make();
	IntTriple& operator[](int idx) {
		return triples[idx];
	};

	std::vector<IntTriple>::iterator begin() { return triples.begin(); }
	std::vector<IntTriple>::iterator end() { return triples.end(); }

};
#endif //TRIPLE_H