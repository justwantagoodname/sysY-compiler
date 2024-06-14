#ifndef TRIPLE_H
#define TRIPLE_H
#include "element.h"
#include <vector>

class Triples {
private:
	Element root;

public:
	constexpr static class CMD {
	public:
		enum CMD_ENUM {
			mov,
			jmp,
			call,
			jeq,
			jne,
			jgt,
			jlt,
			jge,
			jle,
			ret,
			rev,
			pus,
			pop,
			add,
			sub,
			mul,
			div,
			mod,
			tag,
			lab,
		};
	}Cmd = CMD();

	constexpr static class TRIPLEVALUE {
	public:
		enum TRIPLEVALUE_ENUM {
			null,
			dimd, // int立即数
			fimd, // float立即数
			temp, // 临时变量
			value,// 变量编号
			func, // 函数编号
			lamb,// 标签
			
		};
	}TT = TRIPLEVALUE(); // Triple Value Type Enum
	
	typedef TRIPLEVALUE::TRIPLEVALUE_ENUM TripleType;

	struct TripleValue
	{
		int value;
		TripleType type;
		
		TripleValue() : value(0), type(TT.null) {}
		TripleValue(int t) :value(t), type(TT.temp) {}
		TripleValue(int v, TripleType ty) :value(v), type(ty) {}

		bool operator==(const TripleValue& t) const;
		//inline operator int() { return value; }

		void toString(char[], const Triples& triples);
	};

private:

	struct Triple {
		int cmd;
		TripleValue e1, e2, to;

		Triple(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, const TripleValue&);
	};

	std::vector<Triple> triples;
	std::vector<Element> value_pointer;
	std::vector<Element> function_pointer;
	//std::vector<Element> value_table;
	//std::vector<int> page_stack;

	void add(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, const TripleValue&);

	TripleValue find(CMD::CMD_ENUM, const TripleValue&, const TripleValue&) const;
	TripleValue find(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, int) const;
	int find(const Element& e);
	int pushf(const Element& e);
	int findf(const Element& e);
	int findf(const char* name);

public:
	Triples(const Element&);
	Triples(const Triples&) = delete;

	void pretreat();
	void make();

	Triple& operator[](int idx) {
		return triples[idx];
	};
	size_t size();

	std::vector<Triple>::iterator begin() { return triples.begin(); }
	std::vector<Triple>::iterator end() { return triples.end(); }

	void print() const;

};
#endif //TRIPLE_H