﻿#ifndef TRIPLE_H
#define TRIPLE_H
#include "element.h"

#include <map>
#include <vector>
#include <memory>

struct ValueTableElement {
	std::string name;
	int type;
	int scope;
};

class Triples {
private:
	Element root;

public:
	constexpr static class CMD {
	public:
		enum CMD_ENUM {
			mov,
			call,
			jmp,
			jn0,
			jeq,
			jne,
			jgt,
			jlt,
			jge,
			jle,
			//jeqf,
			//jnef,
			//jgtf,
			//jltf,
			//jgef,
			//jlef,
			ret,
			rev,
			pus,
			pop,
			add,
			sub,
			mul,
			div,
			mod,
			//fadd,
			//fsub,
			//fmul,
			//fdiv,
			tag,
			//d2f,
			//f2d,
			mset,
			load,
			store,
			blkb,
			blke,
			var,
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
			str, // 格式化字符串常量
			parms, // 参数组
			blockno, // 块编号
			typetag, // 类型标记，0为int， 1为float
			addr, // 地址类型，用于向函数传递数组
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
		TripleValue& operator=(const TripleValue& t);
		//inline operator int() { return value; }

		void toString(char[], const Triples& triples);
	};

	struct Triple {
		int cmd;
		TripleValue e1, e2, to;

		Triple(CMD::CMD_ENUM, const TripleValue&, const TripleValue&, const TripleValue&);
	};

	std::vector<ValueTableElement> value_table;
	
	// func name -> [<param name, type>, ...]
	// 0,    1,   2,     3,     4
	// void, int, float, int[], float[]
	std::map<std::string, std::vector<std::pair<int, int>>> func_params;

private:
	void setValueTable();
	void setFuncParams();

	int temp_count;
public:

	std::vector<std::shared_ptr<Triple>> triples;
	std::vector<Element> value_pointer;
	std::vector<Element> function_pointer;
	std::vector<std::string> string_pointer;
	std::vector<int> temp_type;
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
	~Triples();

	/// <summary>
	/// 构建前预处理
	/// </summary>
	void pretreat();

	/// <summary>
	/// 构建
	/// </summary>
	void make();

	/// <summary>
	/// 极小化临时变量占用
	/// </summary>
	void minTempVar();

	/// <summary>
	/// 去除无用中间变量
	/// </summary>
	void eliUnnecVar();

	/// <summary>
	/// 重新排序临时变量
	/// </summary>
	void resortTemp();

	/// <summary>
	/// 分析临时变量类型
	/// </summary>
	void listTempType();
	
	/// <summary>
	/// 获得临时变量类型
	/// </summary>
	int getTempType(int n) {
		return temp_type[n];
	}

	/// <summary>
	/// 获取类型
	/// </summary>
	int getValueType(const TripleValue& e);

	Triple& operator[](int idx) {
		return *triples[idx];
	};

	size_t size();

	std::vector<std::shared_ptr<Triple>>::iterator begin() { return triples.begin(); }
	std::vector<std::shared_ptr<Triple>>::iterator end() { return triples.end(); }

	void print() const;

	std::string getValueString(const TripleValue& triple_value) const;
	std::string getFuncName(const TripleValue& triple_value) const;
	std::string getLabelName(const TripleValue& triple_value) const;
	std::string getVarName(const TripleValue& triple_value) const;
};

#endif //TRIPLE_H