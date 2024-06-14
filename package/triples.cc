#include "triples.h"
#include <vector>
using std::vector;

void Triples::add(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, const TripleValue& to)
{
	triples.push_back(Triple(cmd, e1, e2, to));
}

int Triples::find(const Element& e)
{
	for (int i = 0; i < value_pointer.size(); ++i) {
		if (value_pointer[i] == e)return i;
	}
	value_pointer.push_back(e);
	return value_pointer.size() - 1;

}
int Triples::pushf(const Element& e) {
	function_pointer.push_back(e);
	return function_pointer.size() - 1;
}

int Triples::findf(const Element& e)
{
	for (int i = 0; i < function_pointer.size(); ++i) {
		if (function_pointer[i] == e)return i;
	}
	return -1;
}

int Triples::findf(const char* name)
{
	for (int i = 0; i < function_pointer.size(); ++i) {
		if (strcmp(function_pointer[i].get_attr_str("name"), name) == 0)
			return i;
	}
	return -1;
}

Triples::TripleValue Triples::find(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, int len) const
{
	int end = 0;
	if (len != -1)
	{
		end = triples.size() - 1 - len;
		if (end < 0)end = 0;
	}
	for (int i = triples.size() - 1; i >= end; --i) {
		const Triple& t = triples[i];
		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
			return t.to;
		}
	}
	return {};
}

Triples::TripleValue Triples::find(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2) const
{
	int end = 0;
	for (int i = triples.size() - 1; i >= end; --i) {
		const Triple& t = triples[i];
		if (t.cmd == Cmd.lab || t.cmd == Cmd.tag)
			break;
		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
			return t.to;
		}
	}
	return {};
}

Triples::Triples(const Element& e) : root(e) {}

Triples::Triple::Triple(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, const TripleValue& to) : cmd(cmd), e1(e1), e2(e2), to(to) {}

size_t Triples::size()
{
	return triples.size();
}

#define DFS_Element(root) 	for (Element::Iter iter = root.begin();\
		 iter != root.end(); ++iter) 
#define DFS_Element_init\
		Element element = *iter; \
		const char* id = element.id(); \
		const bool flag = element.flag
#define beginElement(eqid) (!strcmp(id, (eqid)) && !flag)
#define endElement(eqid) (!strcmp(id, (eqid)) && flag)
#define ife(eqid) if (endElement(eqid))
#define ifb(eqid) if (beginElement(eqid))
#define cut iter.flag = true; continue

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

	Query array_decls = root("//Decl/*[@array='true']");
	for (auto adecl : array_decls) {
		DFS_Element(adecl) {
			DFS_Element_init;
			ife("Exp") {
				element.add_attr("value", element[0].get_attr_int("value"));
			}
			ife("Dimension") {
				element.add_attr("value", element[0].get_attr_int("value"));
				Element replace = (ASTNode*)calloc(1, sizeof(ASTNode));
				element.move_children_to(replace);
				replace.free();
			}
			ife("ArraySize") {
				element.add_attr("size", element.size());
			}
		}
		adecl.add_attr("size", adecl[0].get_attr_int("size"));
		Element replace = (ASTNode*)calloc(1, sizeof(ASTNode));
		replace.move_children_from(adecl);
		replace.qo("/ArraySize").move_children_to(adecl);
		replace.free();
	}

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
			if (element[0].get_attr("type")) {
				const char* t = element[0].get_attr_str("type");
				element.add_attr("type", t);
			}
		}
		ife("Number") {
			int val = element.get_attr_int("value");
			TripleValue lcmd = triples.find(Cmd.mov, { val ,TT.dimd }, {});
			if (lcmd.type == TT.null) {
				element.add_attr("temp", temp_count);
				triples.add(Cmd.mov, { val ,TT.dimd }, {}, { temp_count });
				++temp_count;
			}
			else {
				element.add_attr("temp", lcmd.value);
			}
		}
		ifb("Address") {
			int size = element.size();
			if (size > 0) { // is array
				//element[0].add_attr("base", element.get_attr_str("base"));
			}
		}
		ife("Address") {
			const char* s = element.get_attr_str("base");
			Element value = element.table(s);

			if (value.get_attr("array")) { // is array
				element.add_attr("array", 1);
				element.add_attr("temp", element[0].get_attr_int("temp"));
			}

			element.add_attr("addr", triples.find(value));
			element.add_attr("type", value.get_attr_str("type"));
		}
		ife("Locator") {
			const char* s = element.get_attr_str("base");
			Element value = element.table(s);

			int size = element.size();
			element.add_attr("size", size);
			int count = 0;
			ASTNode* cur = NULL;
			DL_FOREACH(element.unwrap()->children, cur) {
				int t;
				++count;
				ASTNode_get_attr_int(cur, "temp", &t);
			}
		}
		ife("Dimension") {
			int t = element[0].get_attr_int("temp");
			element.add_attr("temp", t);
		}
		ife("Dest") {
			int a = element[0].get_attr_int("addr");
			element.add_attr("addr", a);
		}
		ife("Fetch") {
			int a = element[0].get_attr_int("addr");
			TripleValue lcmd = triples.find(Cmd.mov, { a, TT.value }, {});
			if (lcmd.type == TT.null) {
				triples.add(Cmd.mov, { a, TT.value }, {}, { temp_count });
				element.add_attr("temp", temp_count);
				++temp_count;
			}
			else {
				element.add_attr("temp", lcmd.value);
			}

			element.add_attr("type", element[0].get_attr_str("type"));
		}
		ife("Assign") {
			int a = element[0].get_attr_int("addr");
			int t = element[1].get_attr_int("temp");
			triples.add(Cmd.mov, { t }, {}, { a, TT.value });
		}

#define makeCond(cmd) do{\
		int t0 = element[0].get_attr_int("temp");\
		int t1 = element[1].get_attr_int("temp");\
		int idx = triples.size();\
		triples.add((cmd), {t0}, {t1}, {});\
		triples.add(Cmd.jmp, {}, {}, {});\
		triples.add(Cmd.lab, {triples.size(), TT.lamb}, {}, {});\
		element.add_attr("true", idx);\
		element.add_attr("false", idx + 1);\
		} while(0)

		ife("Equal") {
			makeCond(Cmd.jeq);
		}
		ife("NotEq") {
			makeCond(Cmd.jne);
		}
		ife("Less") {
			makeCond(Cmd.jlt);
		}
		ife("LessEq") {
			makeCond(Cmd.jle);
		}
		ife("Greater") {
			makeCond(Cmd.jgt);
		}
		ife("GreaterEq") {
			makeCond(Cmd.jge);
		}
		ife("And") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");

			int t = t1;
			do {
				TripleValue tmp = triples[t].to;
				if (triples[t1 + 1].cmd == Cmd.lab)
					triples[t].to = { t1 + 1, TT.lamb };
				else
					triples[t].to = { t1 + 2, TT.lamb };
				t = tmp.value;
			} while (t != 0);

			triples[f2].to = { f1 ,TT.lamb };

			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Or") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");

			int f = f1;
			do {
				TripleValue tmp = triples[f].to;
				if(triples[f1 + 1].cmd == Cmd.lab)
					triples[f].to = { f1 + 1, TT.lamb };
				else
					triples[f].to = { f1 + 2, TT.lamb };
				f = tmp.value;
			} while (f != 0);

			triples[t2].to = { t1 ,TT.lamb };
			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Cond") {
			int t = element[0].get_attr_int("true");
			int f = element[0].get_attr_int("false");
			element("parent::*")[0].add_attr("false", f);
			do {
				TripleValue tmp = triples[t].to;
				triples[t].to = { triples.size() - 1, TT.lamb };
				t = tmp.value;
			} while (t != 0);
			//triples.add(Cmd.lab, { triples.size(), TT.lamb }, {}, {});
		}
		ife("Then") {
			Element if_element = element("parent::If");
			int b = triples.size();
			if_element.add_attr("toEnd", b);
			triples.add(Cmd.jmp, {}, {}, { b, TT.lamb });

			b = triples.size();
			int f = if_element.get_attr_int("false");
			do {
				TripleValue tmp = triples[f].to;
				triples[f].to = { b, TT.lamb };
				f = tmp.value;
			} while (f != 0);
			triples.add(Cmd.lab, { b, TT.lamb }, {}, {});
		}
		ife("Else") {
			Element if_element = element("parent::If");
			int toend = if_element.get_attr_int("toEnd");
			triples[toend].to = triples.size();
		}
		ifb("While") {
			int b = triples.size();
			element.add_attr("begin", b);
			triples.add(Cmd.lab, { b, TT.lamb }, {}, {});
		}
		ife("While") {
			int b = element.get_attr_int("begin");
			int f = element.get_attr_int("false");
			triples.add(Cmd.jmp, {}, {}, { b, TT.lamb });
			triples[f].to = { triples.size(), TT.lamb };
			triples.add(Cmd.lab, { triples.size(), TT.lamb }, {}, {});
		}
		ife("Not") {
			int t = element[0].get_attr_int("true");
			int f = element[0].get_attr_int("false");
			element.add_attr("true", f);
			element.add_attr("false", t);
		}
		ife("Return") {
			if (element.size() == 0) {
				triples.add(Cmd.ret, {}, {}, {});
			}
			else {
				int t = element[0].get_attr_int("temp");
				triples.add(Cmd.ret, { t }, {}, {});
				++temp_count;
			}
		}
#define EopE(cmd) do{\
		int t0 = element[0].get_attr_int("temp"); \
		int t1 = element[1].get_attr_int("temp"); \
		TripleValue lcmd = triples.find(cmd, t0, t1); \
		if(lcmd.type == TT.null){ \
		triples.add(cmd, {t0}, {t1}, {temp_count}); \
			element.add_attr("temp", temp_count); \
			++temp_count; \
		}\
		else { \
			element.add_attr("temp", lcmd.value); \
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
					triples.add(Cmd.pus, { t }, {}, {});
					++count;
					iter = iter->prev;
				} while (iter != head->prev);
			}

			//int top = triples.size() - 1;
			//for (int i = 0; i < (3 > count ? count : 3); ++i) {
			//	triples[top - i].cmd = Cmd.pux;
			//	triples[top - i].to = i;
			//}

			const char* name = element.get_attr_str("name");

			int fid = triples.findf(name);

			element.add_attr("temp", temp_count);
			triples.add(Cmd.call, { fid, TT.func }, {}, { temp_count });
			++temp_count;
		}
		ife("Param") {
			int t = element[0].get_attr_int("temp");
			element.add_attr("temp", t);
		}
		ifb("Function") {
			int fid = triples.pushf(element);
			element.add_attr("fid", fid);
			element.add_attr("place", triples.size());

			triples.add(Cmd.tag, { fid, TT.func }, {}, {});
		}
		ife("Function") {
			if (strcmp(element.get_attr_str("return"), "Void") == 0) {
				triples.add(Cmd.rev, {}, {}, {});
			}
		}
		ifb("Main") {
			int fid = triples.pushf(element);
			element.add_attr("place", triples.size());
			element.add_attr("name", "main");
			element.add_attr("fid", fid);

			triples.add(Cmd.tag, { fid, TT.func }, {}, {});
		}
		ife("ParamDecl") {
			const char* s = element.get_attr_str("name");
			Element value = element.qo("ancestor::Function/Scope/Decl").table(s);
			int a = triples.find(value);
			triples.add(Cmd.pop, {}, {}, { a, TT.value });
		}
	}
}

void Triples::print() const
{
	char ts1[55], ts2[55], ts3[55];
	int idx = 0;
	printf("\n");
	for (auto i : triples) {
		const char cmd_text[][10] = {
			"mov",
			"jmp",
			"call",
			"jeq",
			"jne",
			"jgt",
			"jlt",
			"jge",
			"jle",
			"ret",
			"rev",
			"pus",
			"pop",
			"add",
			"sub",
			"mul",
			"div",
			"mod",
			"tag",
			"lab",
		};
		if (i.cmd == Cmd.tag) {
			printf("%s -%d:\n", function_pointer[i.e1.value].get_attr_str("name"), idx);
		}
		else if (i.cmd == Cmd.lab) {
			printf("%d>.l%d:\n", idx, i.e1.value);
		}
		else {
			i.e1.toString(ts1, *this);
			i.e2.toString(ts2, *this);
			i.to.toString(ts3, *this);
			printf("%d> \t%s, %s, %s, %s\n", idx, cmd_text[i.cmd], ts1, ts2, ts3);
		}
		++idx;
	}
}

bool Triples::TripleValue::operator==(const Triples::TripleValue& t) const
{
	return value == t.value && type == t.type;
}

void Triples::TripleValue::toString(char s[], const Triples& triples)
{
	switch (type)
	{
	case TT.null:
		snprintf(s, 20, "-");
		break;
	case TT.temp:
		snprintf(s, 20, "T%d", value);
		break;
	case TT.dimd:
		snprintf(s, 20, "#%d", value);
		break;
	case TT.value:
		snprintf(s, 40, "%s", triples.value_pointer[value].get_attr_str("name"));
		break;
	case TT.func:
		snprintf(s, 50, "%s:%d", triples.function_pointer[value].get_attr_str("name"), triples.function_pointer[value].get_attr_int("place"));
		break;
	case TT.lamb:
		snprintf(s, 20, ">%d", value);
		break;
	default:
		snprintf(s, 20, "unknow:%d", value);
		break;
	}
}
