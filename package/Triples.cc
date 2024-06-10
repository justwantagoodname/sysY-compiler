#include "Triples.h"
#include <vector>
using std::vector;
#if 0
void Triples::add(int cmd, int e1, int e2, int to)
{
	triples.push_back(IntTriple(cmd, e1, e2, to));
}

int Triples::find(const Element& e)
{
	for (int i = 0; i < value_pointer.size(); ++i) {
		if (value_pointer[i] == e)return i;
	}
	value_pointer.push_back(e);
	return value_pointer.size() - 1;

}

int Triples::find(int cmd, int e1, int e2, int len = -1) const
{
	int end = 0;
	if (len != -1)
	{
		end = triples.size() - 1 - len;
		if (end < 0)end = 0;
	}
	for (int i = triples.size() - 1; i >= end; --i) {
		const IntTriple& t = triples[i];
		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
			return t.to;
		}
	}
	return -1;
}

Triples::Triples(const Element& e) : root(e) {}

Triples::IntTriple::IntTriple(int cmd, int e1, int e2, int to) : cmd(cmd), e1(e1), e2(e2), to(to) {}

size_t Triples::size()
{
	return triples.size();
}

void Triples::pretreat()
{
	//Query conds = root("//Cond");
	//for (auto cond : conds) {
	//	Element exp = cond.qo("/Exp");
	//	cond.move_children_from(exp);
	//	ASTNode*n = cond.unwrap()->children;
	//	DL_DELETE(n, n);
	//	//exp.free();
	//}
}

void Triples::make()
{
	Triples& triples = *this;
	int temp_count = 0; // 临时变量计数器 

	//for (auto element : root) {
	for (Element::Iter iter = root.begin(); iter != root.end(); ++iter) {
		Element element = *iter;

		const char* id = element.id();
		const bool flag = element.flag;

#define beginElement(eqid) (!strcmp(id, (eqid)) && !flag)
#define endElement(eqid) (!strcmp(id, (eqid)) && flag)
#define ife(eqid) if (endElement(eqid))
#define ifb(eqid) if (beginElement(eqid))
#define cut iter.flag = true; continue

		ifb("Decl") {
			cut;
		}
		ife("Exp") {
			if (element[0].get_attr("temp")) {
				int t = element[0].get_attr_int("temp");
				element.add_attr("temp", t);
			}
			if (element[0].get_attr("true")) {
				int t = element[0].get_attr_int("true");
				element.add_attr("true", t);
			}
			if (element[0].get_attr("false")) {
				int t = element[0].get_attr_int("false");
				element.add_attr("false", t);
			}
		}
		ife("Number") {
			element.add_attr("temp", temp_count);
			triples.add(imdd, element.get_attr_int("value"), 0, temp_count);
			++temp_count;
		}
		ife("Address") {
			const char* s = element.get_attr_str("base");
			Element value = element.table(s);

			if (value.get_attr("array")) { // is array

			}

			element.add_attr("addr", triples.find(value));
		}
		ife("Dest") {
			int a = element[0].get_attr_int("addr");
			element.add_attr("addr", a);
		}
		ife("Fetch") {
			int a = element[0].get_attr_int("addr");
			int lcmd = triples.find(read, a, 0);
			if (lcmd == -1) {
				triples.add(read, a, 0, temp_count);
				element.add_attr("temp", temp_count);
				++temp_count;
			}
			else {
				element.add_attr("temp", lcmd);
			}
		}
		ife("Assign") {
			int a = element[0].get_attr_int("addr");
			int t = element[1].get_attr_int("temp");
			triples.add(write, t, 0, a);
		}

#define makeCond(cmd)do{\
		int t0 = element[0].get_attr_int("temp");\
		int t1 = element[1].get_attr_int("temp");\
		int idx = triples.size();\
		triples.add((cmd), t0, t1, 0);\
		triples.add(Cmd.jmp, 0, 0, 0);\
		element.add_attr("true", idx);\
		element.add_attr("false", idx + 1);}\
		while(0)
		ife("Equal") {
			makeCond(jeq);
		}
		ife("NotEq") {
			makeCond(jne);
		}
		ife("Less") {
			makeCond(jlt);
		}
		ife("LessEq") {
			makeCond(jle);
		}
		ife("Greater") {
			makeCond(jgt);
		}
		ife("GreaterEq") {
			makeCond(Cmd.jge);
		}
		ife("And") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");
			triples[t1].to = t1 + 2;
			triples[f2].to = f1;
			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Or") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");
			triples[t2].to = t1;
			triples[f1].to = f1 + 1;
			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Cond") {
			int t = element[0].get_attr_int("true");
			int f = element[0].get_attr_int("false");
			element("parent::*")[0].add_attr("false", f);
			do {
				int tmp = triples[t].to;
				triples[t].to = triples.size();
				t = tmp;
			} while (t != 0);
		}
		ife("Then") {
			Element if_element = element("parent::If");
			if_element.add_attr("toEnd", triples.size());
			triples.add(jmp, 0, 0, triples.size());

			int f = if_element.get_attr_int("false");
			do {
				int tmp = triples[f].to;
				triples[f].to = triples.size();
				f = tmp;
			} while (f != 0);
		}
		ife("Else") {
			Element if_element = element("parent::If");
			int toend = if_element.get_attr_int("toEnd");
			triples[toend].to = triples.size();
		}
		ifb("While") {
			element.add_attr("begin", triples.size());
		}
		ife("While") {
			int b = element.get_attr_int("begin");
			int f = element.get_attr_int("false");
			triples.add(Cmd.jmp, 0, 0, b);
			triples[f].to = triples.size();
		}
		ife("Not") {
			int t = element[0].get_attr_int("true");
			int f = element[0].get_attr_int("false");
			element.add_attr("true", f);
			element.add_attr("false", t);
		}
		ife("Return") {
			int t = element[0].get_attr_int("temp");
			triples.add(Cmd.ret, t, 0, 0);
			++temp_count;
		}
#define EopE(cmd) do{\
		int t0 = element[0].get_attr_int("temp"); \
		int t1 = element[1].get_attr_int("temp"); \
		int lcmd = triples.find(cmd, t0, t1); \
		if(lcmd == -1){ \
		triples.add(cmd, t0, t1, temp_count); \
			element.add_attr("temp", temp_count); \
			++temp_count; \
		}\
		else { \
			element.add_attr("temp", lcmd); \
		} \
	}while (0)
		ife("Plus") {
			EopE(Cmd.add);
		}
		ife("Minus") {
			EopE(Cmd.sub);
		}
		ife("Mult") {
			EopE(Cmd.mul);
		}
		ife("Div") {
			EopE(Cmd.div);
		}
		ife("Mod") {
			EopE(Cmd.mod);
		}
		ife("Call") {
			int count = 0;
			{
				ASTNode* head = element.unwrap()->children;
				ASTNode* iter = head->prev;
				do {
					int t;
					ASTNode_get_attr_int(iter, "temp", &t);
					triples.add(Cmd.pus, t, 0, 0);
					++count;
					iter = iter->prev;
				} while (iter != head->prev);
			}

			int top = triples.size() - 1;
			for (int i = 0; i < (3 > count ? count : 3); ++i) {
				triples[top - i].cmd = Cmd.pux;
				triples[top - i].to = i;
			}

			const char* name = element.get_attr_str("name");
			char* select = (char*)malloc(35 + strlen(name));
			assert(select != NULL);
			sprintf(select, "//FunctionDef/Function[@name='%s']", name);
			Element func = root % select;
			int place = func.get_attr_int("place");

			element.add_attr("temp", temp_count);
			triples.add(Cmd.call, place, 0, temp_count);
			++temp_count;
			::free(select);
		}
		ife("Param") {
			int t = element[0].get_attr_int("temp");
			element.add_attr("temp", t);
		}
		ifb("Function") {
			element.add_attr("place", triples.size());
		}
	}
}

void Triples::print() const
{
	int idx = 0;
	char* select;
	printf("\n");
	for (auto i : triples) {
		printf(">%d, ", idx);
		switch (i.cmd)
		{
		case Cmd.mov:
			printf("mov, T%d, 0, T%d\n", i.e1, i.to);
			break;
		case Cmd.jmp:
			printf("jmp, 0, 0, %d\n", i.to);
			break;
		case Cmd.call:
			select = (char*)malloc(60);
			assert(select != NULL);
			sprintf(select, "//FunctionDef/Function[@place=%d]", i.e1);
			printf("call, %s:%d, 0, T%d\n", root.qo(select).get_attr_str("name"), i.e1, i.to);
			::free(select);
			break;
		case Cmd.read:
			printf("read, %s, %d, T%d\n", value_pointer[i.e1].get_attr_str("name"), i.e2, i.to);
			break;
		case Cmd.write:
			printf("write, T%d, 0, %s\n", i.e1, value_pointer[i.to].get_attr_str("name"));
			break;
		case Cmd.imdd:
			printf("imdd, %d, 0, T%d\n", i.e1, i.to);
			break;
		case Cmd.jeq:
			printf("jeq, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.jne:
			printf("jne, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.jlt:
			printf("jlt, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.jgt:
			printf("jgt, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.jle:
			printf("jle, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.jge:
			printf("jge, T%d, T%d, %d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.ret:
			printf("ret, T%d, 0, 0\n", i.e1);
			break;
		case Cmd.add:
			printf("add, T%d, T%d, T%d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.sub:
			printf("sub, T%d, T%d, T%d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.mul:
			printf("mul, T%d, T%d, T%d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.div:
			printf("div, T%d, T%d, T%d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.mod:
			printf("mod, T%d, T%d, T%d\n", i.e1, i.e2, i.to);
			break;
		case Cmd.pus:
			printf("pus, T%d, 0, 0\n", i.e1);
			break;
		case Cmd.pux:
			printf("pux, T%d, 0, E%d\n", i.e1, i.to);
			break;
		default:
			printf("unknow:%d, %d, %d, %d", i.cmd, i.e1, i.e2, i.to);
			break;
		}
		++idx;
	}
}

#endif