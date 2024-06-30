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
			jeqf,
			jnef,
			jgtf,
			jltf,
			jgef,
			jlef,
			ret,
			rev,
			pus,
			pop,
			add,
			sub,
			mul,
			div,
			mod,
			fadd,
			fsub,
			fmul,
			fdiv,
			tag,
			d2f,
			f2d,
			mset,
		};
	}Cmd = CMD();

	constexpr static class TRIPLEVALUE {
	public:
		enum TRIPLEVALUE_ENUM {
			null,
			imd, // int立即数
			temp, // 临时变量
			value,// 变量编号
			func, // 函数编号
			lamb,// 标签
			str, // 格式化字符串常量
		};
	}TT = TRIPLEVALUE(); // Triple Value Type Enum

	typedef TRIPLEVALUE::TRIPLEVALUE_ENUM TripleType;

	struct TripleValue
	{
		int value;
		TripleType type;
		TripleValue* added = nullptr;

		TripleValue() : value(0), type(TT.null), added(nullptr) {}
		TripleValue(int t) :value(t), type(TT.temp), added(nullptr) {}
		TripleValue(int v, TripleType ty) :value(v), type(ty), added(nullptr) {}
		TripleValue(int v, TripleType ty, const TripleValue& at);
		TripleValue(const char* str, Triples* triple);
		TripleValue(const TripleValue& at);

		~TripleValue();

		bool operator==(const TripleValue& t) const;
		bool operator!=(const TripleValue& t) const;
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
	std::vector<std::string> string_pointer;
	//std::vector<Element> value_table;
	//std::vector<int> page_stack;

	void add(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, const TripleValue&);

	//TripleValue find(CMD::CMD_ENUM, const TripleValue&, const TripleValue&) const;
	//TripleValue find(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, int) const;
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