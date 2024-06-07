#ifndef TRIPLE_H
#define TRIPLE_H
#include "Element.h"

class Triples {
private:
	Element root;
	
private:
	class {
		enum {
			mov,
			jmp,
			call,
		};
	} Cmd;

	struct IntTriple {
		int cmd;
		int e1, e2, to;
	};

public:
	Triples(const Element&&);
	Triples(const Triples&) = delete;

	void make();
};
#endif //TRIPLE_H