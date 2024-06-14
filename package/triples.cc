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

Triples::TripleValue Triples::find(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, int len = -1) const
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

Triples::Triples(const Element& e) : root(e) {}

Triples::Triple::Triple(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, const TripleValue& to) : cmd(cmd), e1(e1), e2(e2), to(to) {}

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

	Query array_decls = root("//Decl/*[@array='true']");
	for (auto adecl : array_decls) {
		for (auto it : adecl) {
			if (!strcmp(it.id(), "Exp") && it.flag) {
				it.add_attr("value", it[0].get_attr_int("value"));
			}
			if (!strcmp(it.id(), "Dimension") && it.flag) {
				it.add_attr("value", it[0].get_attr_int("value"));
				Element replace = (ASTNode*)calloc(1, sizeof(ASTNode));
				it.move_children_to(replace);
				replace.free();
			}
			if (!strcmp(it.id(), "ArraySize") && it.flag) {
				it.add_attr("size", it.size());
			}
		}
		adecl.add_attr("size", adecl[0].get_attr_int("size"));
		Element replace = (ASTNode*)calloc(1, sizeof(ASTNode));
		replace.move_children_from(adecl);
		replace.qo("/ArraySize").move_children_to(adecl);
		replace.free();
	}

	Query not_nodes = root("//Not");
	for (auto notnode : not_nodes) {
		//for (auto it : notnode) {
		//	
		//}
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

		//printf("element: <%s%s>\n", flag ? "/" : "", id);

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
			if (element[0].get_attr("type")) {
				const char* t = element[0].get_attr_str("type");
				element.add_attr("type", t);
			}
		}
		ife("Number") {
			int val = element.get_attr_int("value");
			TripleValue lcmd = triples.find(Cmd.imdd, { val ,TVTE.dimd }, {});
			if (lcmd.type == TVTE.null) {
				element.add_attr("temp", temp_count);
				triples.add(Cmd.imdd, { val ,TVTE.dimd }, {}, { temp_count });
				++temp_count;
			}
			else {
				element.add_attr("temp", lcmd.value);
			}
		}
		ifb("Address") {
			int size = element.size();
			if (size > 0) { // is array
				element[0].add_attr("base", element.get_attr_str("base"));
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
			TripleValue lcmd = triples.find(Cmd.mov, { a, TVTE.value }, {});
			if (lcmd.type == TVTE.null) {
				triples.add(Cmd.mov, { a, TVTE.value }, {}, { temp_count });
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
			triples.add(Cmd.mov, { t }, {}, { a, TVTE.value });
		}

#define makeCond(cmd) do{\
		int t0 = element[0].get_attr_int("temp");\
		int t1 = element[1].get_attr_int("temp");\
		int idx = triples.size();\
		triples.add((cmd), {t0}, {t1}, {});\
		triples.add(Cmd.jmp, {}, {}, {});\
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
			triples[t1].to = { t1 + 2 ,TVTE.local };
			triples[f2].to = { f1 ,TVTE.local };
			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Or") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");
			triples[t2].to = { t1 ,TVTE.local };
			triples[f1].to = { f1 + 1, TVTE.local };
			element.add_attr("true", t2);
			element.add_attr("false", f2);
		}
		ife("Cond") {
			int t = element[0].get_attr_int("true");
			int f = element[0].get_attr_int("false");
			element("parent::*")[0].add_attr("false", f);
			do {
				TripleValue tmp = triples[t].to;
				triples[t].to = { triples.size() , TVTE.local };
				t = tmp.value;
			} while (t != 0);
		}
		ife("Then") {
			Element if_element = element("parent::If");
			if_element.add_attr("toEnd", triples.size());
			triples.add(Cmd.jmp, {}, {}, { triples.size(), TVTE.local });

			int f = if_element.get_attr_int("false");
			do {
				TripleValue tmp = triples[f].to;
				triples[f].to = { triples.size(), TVTE.local };
				f = tmp.value;
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
			triples.add(Cmd.jmp, {}, {}, { b, TVTE.local });
			triples[f].to = { triples.size(), TVTE.local };
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
		if(lcmd.type == TVTE.null){ \
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
			triples.add(Cmd.call, { fid, TVTE.func }, {}, { temp_count });
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

			triples.add(Cmd.tag, { fid, TVTE.func }, {}, {});
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

			triples.add(Cmd.tag, { fid, TVTE.func }, {}, {});
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
			"read",
			"reada",
			"write",
			"imdd",
			"jeq",
			"jne",
			"jgt",
			"jlt",
			"jge",
			"jle",
			"ret",
			"rev",
			"pux",
			"pus",
			"add",
			"sub",
			"mul",
			"div",
			"mod",
			"tag",
		};
		if (i.cmd == Cmd.tag) {
			printf("%s -%d:\n", function_pointer[i.e1.value].get_attr_str("name"), idx);
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
	case TVTE.null:
		snprintf(s, 20, "-");
		break;
	case TVTE.temp:
		snprintf(s, 20, "T%d", value);
		break;
	case TVTE.dimd:
		snprintf(s, 20, "#%d", value);
		break;
	case TVTE.value:
		snprintf(s, 40, "%s", triples.value_pointer[value].get_attr_str("name"));
		break;
	case TVTE.func:
		snprintf(s, 50, "%s:%d", triples.function_pointer[value].get_attr_str("name"), triples.function_pointer[value].get_attr_int("place"));
		break;
	case TVTE.local:
		snprintf(s, 20, ">%d", value);
		break;
	default:
		snprintf(s, 20, "unknow:%d", value);
		break;
	}
}
