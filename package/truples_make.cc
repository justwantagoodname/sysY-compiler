#include "triples.h"
#include <vector>
using std::vector;


#define DFS_Element(node) 	for (Element::Iter iter = (node).dfsbegin();\
		 iter != (node).dfsend(); iter.next()) 
#define DFS_Element_init Element element = *iter
#define beginElement(eqid) ((*iter).id_is(eqid) && !(*iter).flag)
#define endElement(eqid) ((*iter).id_is(eqid) && (*iter).flag)
#define ife(eqid) if (endElement(eqid))
#define ifb(eqid) if (beginElement(eqid))
#define cut {iter.flag = true; continue;}

void Triples::pretreat()
{
	Query array_decls_Unknown = root("//Decl/*[@array='true']/Dimension[@size='Unknown']");
	for (auto adecl : array_decls_Unknown) {
		adecl.add_attr("unknown-size", -1);
	}

	Query array_decls = root("//Decl/*[@array='true']");
	for (auto adecl : array_decls) {
		DFS_Element(adecl) {
			DFS_Element_init;
			ife("Exp") {
				element.add_attr("value", element[0].get_attr_int("value"));
			}
			ife("Dimension") {
				if (!element.get_attr("unknown-size")) {
					element.add_attr("size", element[0].get_attr_int("value"));

					Element("")
						.move_children_from(element)
						.free();
				}
			}
			//ife("ArraySize") {
			//	element.add_attr("size", element.size());
			//	ASTNode* chilen = element.children();
			//	ASTNode* cur = NULL;
			//	int size = 1;
			//	DL_FOREACH(chilen, cur) {
			//		int i;
			//		ASTNode_get_attr_int(cur, "value", &i);
			//		size *= i;
			//	}
			//	element.add_attr("value", size);
			//}
		}
		//adecl.add_attr("value", adecl[0].get_attr_int("value"));
	}

	//Query ex_functions = root("//Call[@name='printf'"
	//	"|@name='getint'|@name='getfloat'|@name='getarray'"
	//	"|@name='starttime'|@name='stoptime'"
	//	"|@name='putint'|@name='putfloat'|@name='putch'|@name='putarray']");
	//for (auto ex_fun : ex_functions) {
	//	ex_fun.add_attr("ex_fun", 1);
	//}

	Element decls = root("//Scope/Decl");
	decls.add_attr("name", "Global");

	Query cond_node = root("//And");
	cond_node += root("//Or");
	cond_node += root("//Cond/Exp");

	for (auto cond : cond_node) {
		Element copy_cond_node = cond.clone();
		copy_cond_node.unwrap()->children = NULL;

		ASTNode* head = cond.children();
		ASTNode* el = NULL;
		for ((el) = (head); el; (el) = (el)->next) {
			Element item = el;

			if (!item.id_is("Equal") &&
				!item.id_is("NotEq") &&
				!item.id_is("Less") &&
				!item.id_is("LessEq") &&
				!item.id_is("Greater") &&
				!item.id_is("GreaterEq") &&
				!item.id_is("And") &&
				!item.id_is("Or") &&
				!item.id_is("Not") &&
				!item.id_is("NotZero"))

			{
				Element cutnode = Element("NotZero");
				cutnode.unwrap()->next = el->next;
				cutnode.unwrap()->prev = el->prev;
				if (el->next)
					el->next->prev = cutnode.unwrap();
				if (el->prev->next)
					el->prev->next = cutnode.unwrap();

				cutnode.unwrap()->parent = el->parent;
				cutnode.unwrap()->children = el;
				el->next = NULL;
				el->prev = el;

				if (el == head) {
					el->parent->children = cutnode.unwrap();
				}

				el->parent = cutnode.unwrap();
				el = cutnode.unwrap();
			}
		}
	}

	Query call_time_node = root("//Call[@name='starttime'|@name='stoptime']");
	for (auto call : call_time_node) {
		int line = call.get_attr_int("line");
		call.add_child(
			Element("Param")
			.add_child(
				Element("Number")
				.add_attr("value", call.get_attr_int("line"))
				.add_attr("type", "Int")
			)
		);
	}
}

void Triples::make()
{
	Triples& triples = *this;
	int temp_count = 0; // ��ʱ���������� 
	int tag_count = 0; // ��ʱ��ǩ������
	int block_count = 0; // block������

	//for (auto element : root) {
	DFS_Element(root) {
		Element element = *iter;

		//printf("--%p", element.unwrap());
		printf("--%s\n", element.id());

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

			element.add_attr("temp", temp_count);
			if (strcmp(element.get_attr_str("type"), "Int") == 0) {
				triples.add(Cmd.mov, { val ,TT.dimd }, {}, { temp_count });
			}
			else {
				triples.add(Cmd.mov, { val ,TT.fimd }, {}, { temp_count });
			}
			++temp_count;

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

			element.add_attr("addr", triples.find(value));
			element.add_attr("type", value.get_attr_str("type"));

			if (value.get_attr("array")) { // is array
				if (element.size() == 0) {
					element.add_attr("array", 1);
					element.add_attr("is_addr", 1);
					element.add_attr("temp", temp_count);
					triples.add(Cmd.mov, { 0, TT.dimd }, {}, { temp_count++ });
				}
				else if (element[0].get_attr("is_addr")) {
					element.add_attr("array", 1);
					element.add_attr("is_addr", 1);
					element.add_attr("temp", element[0].get_attr_int("temp"));
				}
				else {
					element.add_attr("array", 1);
					element.add_attr("temp", element[0].get_attr_int("temp"));
				}
			}
		}
		ife("Locator") {
			const char* s = element.get_attr_str("base");
			Element value = element.table(s);

			if (!value.id_is("ParamDecl")) {
				value = value[0];
			}

			int temp = temp_count++;

			int count = 0;
			ASTNode* cur = NULL;
			int cr = 1;
			triples.add(Cmd.mov, { 0, TT.dimd }, {}, { temp });
			DL_FOREACH(element.unwrap()->children, cur) {
				int t;
				ASTNode_get_attr_int(cur, "temp", &t);
				if (count > 0) {
					cr *= value[count].get_attr_int("size");
					triples.add(Cmd.mul, { t }, { cr, TT.dimd }, { t });
				}
				++count;
				triples.add(Cmd.add, { temp }, { t }, { temp });
			}

			if (element.size() < value.size()) {
				element.add_attr("is_addr", 1);
				element.add_attr("addr_base", triples.find(value));
				element.add_attr("temp", temp);
			}
			else {
				element.add_attr("temp", temp);
			}
		}
		ife("Dimension") {
			int t = element[0].get_attr_int("temp");
			element.add_attr("temp", t);
		}
		ife("Dest") {
			int a = element[0].get_attr_int("addr");
			element.add_attr("addr", a);
			element.add_attr("type", element[0].get_attr_str("type"));

			if (element[0].get_attr("array"))
			{
				element.add_attr("array", 1);
				element.add_attr("temp", element[0].get_attr_int("temp"));
			}
		}
		ife("Fetch") {
			int a = element[0].get_attr_int("addr");
			TripleValue d = {};
			bool flg = false;

			if (element[0].get_attr("array")) {
				d = element[0].get_attr_int("temp");
				if (element[0].get_attr("is_addr")) {
					flg = true;
				}
			}
			triples.add(flg ? Cmd.mov : Cmd.load, { a, flg ? TT.addr : TT.value, d }, {}, { temp_count });
			element.add_attr("temp", temp_count);
			++temp_count;
			element.add_attr("type", element[0].get_attr_str("type"));
		}

#define TypeTras(ty0, ty1, t) do{\
		if (strcmp(ty0, "Int") == 0 && strcmp(ty1, "Float") == 0) {		\
			triples.add(Cmd.f2d, t, {}, temp_count);					\
			t = temp_count++;											\
		}																\
		else if (strcmp(ty0, "Float") == 0 && strcmp(ty1, "Int") == 0) {\
			triples.add(Cmd.d2f, t, {}, temp_count);					\
			t = temp_count++;											\
		}																\
		}while(0)

		ife("Assign") {
			int a = element[0].get_attr_int("addr");
			int t = element[1].get_attr_int("temp");
			TripleValue d = {};
			if (element[0].get_attr("array"))
				d = { element[0].get_attr_int("temp") };

			const char* at = element[0].get_attr_str("type");
			const char* tt = element[1].get_attr_str("type");
			//TypeTras(at, tt, t);

			triples.add(Cmd.store, { t }, {}, { a, TT.value, d });
		}

#define EopETypeTras(node, t0, t1, isfloat) do{		   \
		const char* ts0 = (node)[0].get_attr_str("type");\
		const char* ts1 = (node)[1].get_attr_str("type");\
		if(strcmp(ts0, "Float") == 0 || strcmp(ts1, "Float") == 0){\
			if(strcmp(ts0, "Int") == 0 ){\
			triples.add(Cmd.d2f, t0, {}, temp_count);\
			t0 = temp_count++; \
			}	\
			if(strcmp(ts1, "Int") == 0 ){\
			triples.add(Cmd.d2f, t1, {}, temp_count);\
			t1 = temp_count++; \
			}	\
			(node).add_attr("type", "Float");\
			isfloat = true;\
		}\
		else{\
			(node).add_attr("type", "Int");\
			isfloat = false;\
		}\
		}while(0)

		/*
	#define makeCond(cmd, fcmd) do{\
			bool isfloat = false;\
			int t0 = element[0].get_attr_int("temp");\
			int t1 = element[1].get_attr_int("temp");\
			int idx = triples.size();\
			EopETypeTras(element, t0, t1, isfloat);\
			CMD::CMD_ENUM tcmd = isfloat ? (fcmd) : (cmd);\
			triples.add(tcmd, {t0}, {t1}, {});\
			triples.add(Cmd.jmp, {}, {}, {});\
			triples.add(Cmd.tag, { tag_count++, TT.lamb}, {}, {});\
			element.add_attr("true", idx);\
			element.add_attr("false", idx + 1);\
			} while(0)

			ife("Equal") {
				makeCond(Cmd.jeq, Cmd.jeqf);
			}
			ife("NotEq") {
				makeCond(Cmd.jne, Cmd.jnef);
			}
			ife("Less") {
				makeCond(Cmd.jlt, Cmd.jltf);
			}
			ife("LessEq") {
				makeCond(Cmd.jle, Cmd.jlef);
			}
			ife("Greater") {
				makeCond(Cmd.jgt, Cmd.jgtf);
			}
			ife("GreaterEq") {
				makeCond(Cmd.jge, Cmd.jgef);
			}
	*/
#define makeCond(cmd) do{\
		bool isfloat = false;\
		int t0 = element[0].get_attr_int("temp");\
		int t1 = element[1].get_attr_int("temp");\
		int idx = triples.size();\
		CMD::CMD_ENUM tcmd = (cmd);\
		triples.add(tcmd, {t0}, {t1}, {});\
		triples.add(Cmd.jmp, {}, {}, {});\
		triples.add(Cmd.tag, { tag_count++, TT.lamb}, {}, {});\
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
		ife("NotZero") {
			int temp = element[0].get_attr_int("temp");
			triples.add(Cmd.jn0, { temp }, {}, {});
			triples.add(Cmd.jmp, {}, {}, {});

			element.add_attr("true", triples.size() - 2);
			element.add_attr("false", triples.size() - 1);
		}
		ife("And") {
			int t1 = element[0].get_attr_int("true"),
				t2 = element[1].get_attr_int("true");
			int f1 = element[0].get_attr_int("false"),
				f2 = element[1].get_attr_int("false");

			int t = t1;
			do {
				TripleValue tmp = triples[t].to;
				if (triples[t1 + 1].cmd == Cmd.tag)
					triples[t].to = { triples[t1 + 1].e1.value, TT.lamb };
				else
					triples[t].to = { triples[t1 + 2].e1.value, TT.lamb };
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
				if (triples[f1 + 1].cmd == Cmd.tag)
					triples[f].to = { triples[f1 + 1].e1.value, TT.lamb };
				else
					triples[f].to = { triples[f1 + 2].e1.value, TT.lamb };
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
				triples[t].to = { tag_count, TT.lamb };
				t = tmp.value;
			} while (t != 0);
			triples.add(Cmd.tag, { tag_count++, TT.lamb }, {}, {});
		}
		ife("Then") {
			Element if_element = element("parent::If");
			int b = triples.size();
			if_element.add_attr("toEnd", b);
			triples.add(Cmd.jmp, {}, {}, {});

			b = triples.size();
			int f = if_element.get_attr_int("false");
			do {
				TripleValue tmp = triples[f].to;
				triples[f].to = { tag_count, TT.lamb };
				f = tmp.value;
			} while (f != 0);
			triples.add(Cmd.tag, { tag_count++, TT.lamb }, {}, {});
		}
		ife("If") {
			int toend = element.get_attr_int("toEnd");
			triples[toend].to = { tag_count, TT.lamb };
			triples.add(Cmd.tag, { tag_count++, TT.lamb }, {}, {});
		}
		ifb("While") {
			int b = tag_count++;
			element.add_attr("begin", b);
			triples.add(Cmd.tag, { b, TT.lamb }, {}, {});
		}
		ife("While") {
			int b = element.get_attr_int("begin");
			int f = element.get_attr_int("false");
			triples.add(Cmd.jmp, {}, {}, { b, TT.lamb });
			triples[f].to = { tag_count, TT.lamb };
			triples.add(Cmd.tag, { tag_count++, TT.lamb }, {}, {});
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
		/*
		#define EopE(cmd, fcmd) do{\
				bool isfloat = false;\
				int t0 = element[0].get_attr_int("temp"); \
				int t1 = element[1].get_attr_int("temp"); \
				EopETypeTras(element, t0, t1, isfloat);\
				CMD::CMD_ENUM cmdt = isfloat? fcmd : cmd;\
				triples.add(cmdt, {t0}, {t1}, {temp_count}); \
					element.add_attr("temp", temp_count); \
					++temp_count; \
				}while (0)
				ife("Plus") {
					EopE(Cmd.add, Cmd.fadd);
				}
				ife("Minus") {
					EopE(Cmd.sub, Cmd.fsub);
				}
				ife("Mult") {
					EopE(Cmd.mul, Cmd.fmul);
				}
				ife("Div") {
					EopE(Cmd.div, Cmd.fdiv);
				}
				ife("Mod") {
					EopE(Cmd.mod, Cmd.mod);
					assert(strcmp("Int", element.get_attr_str("type")) == 0);
				}
		*/
#define EopE(cmd) do{\
			bool isfloat = false;\
			int t0 = element[0].get_attr_int("temp"); \
			int t1 = element[1].get_attr_int("temp"); \
			CMD::CMD_ENUM cmdt = (cmd);\
			triples.add(cmdt, {t0}, {t1}, {temp_count}); \
			element.add_attr("temp", temp_count); \
			if(strcmp(element[0].get_attr_str("type"), "Float") == 0 \
				 || strcmp(element[0].get_attr_str("type"), "Float") == 0){\
				element.add_attr("type", "Float");\
			}\
			else{\
				element.add_attr("type", "Int");\
			}\
			++temp_count; \
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
		ife("UnMinus") {
			triples.add(Cmd.sub, { 0, TT.dimd }, element[0].get_attr_int("temp"), temp_count);
			element.add_attr("temp", temp_count);
			temp_count++;
			element.add_attr("type", element[0].get_attr_str("type"));
		}
		ife("Call") {
			TripleValue parms = { 0, TT.parms };

			// ���ǵ���pus�Ĵ��룬 ���ǻᵹ��ÿһ������ͨ��pusָ�����ջ
			// �뱣����Щ���룬�Է�ĳһ��Ҫ���л���
			//int count = 0;
			//{
			//	ASTNode* head = element.unwrap()->children;
			//	ASTNode* iter = head->prev;
			//	do {
			//		const char* type;
			//		ASTNode_get_attr_str(iter, "type", &type);
			//		if (strcmp(type, "StringConst") != 0) {
			//			int t;
			//			ASTNode_get_attr_int(iter, "temp", &t);
			//			triples.add(Cmd.pus, { t }, {}, {});
			//			++count;
			//			iter = iter->prev;
			//		}
			//		else {
			//			const char* str;
			//			ASTNode_get_attr_str(iter, "value", &str);
			//			triples.add(Cmd.pus, { str, this }, {}, {});
			//			iter = iter->prev;
			//		}
			//	} while (iter != head->prev);
			//}
			// ���⣨ > w < )

			// ��Щ�ǽ�parm���Ϊһ��triple value�Ĵ���
			{
				int count = 0;
				ASTNode* head = element.unwrap()->children;
				ASTNode* iter = NULL;
				TripleValue* last_triple = &parms;
				DL_FOREACH(head, iter) {
					const char* type;
					ASTNode_get_attr_str(iter, "type", &type);
					if (strcmp(type, "StringConst") == 0) {
						const char* str;
						ASTNode_get_attr_str(iter, "value", &str);
						last_triple->added = new TripleValue{ str, this };
						last_triple = last_triple->added;
						last_triple->added = nullptr;
						//printf("parms %d input str by %p, type: %s\n", count, iter, type);
						//ASTNode_print(iter);
					}
					else {
						int t;
						ASTNode_get_attr_int(iter, "temp", &t);
						last_triple->added = new TripleValue{ t };
						last_triple = last_triple->added;
						last_triple->added = nullptr;
						//printf("parms %d input temp\n", count);

					}
					count++;
				}
				parms.value = count;
			}
			// ���⣨ > w < )

			const char* name = element.get_attr_str("name");
			element.add_attr("temp", temp_count);

			int fid = triples.findf(name);

			if (fid != -1) {
				triples.add(Cmd.call, { fid, TT.func }, parms, { temp_count });
				element.add_attr("type", function_pointer[fid].get_attr_str("return"));

			}
			else {
				triples.add(Cmd.call, { element.get_attr_str("name"), this }, parms, { temp_count });

				// TODO -> 在这里加载内置函数的类型
				element.add_attr("type", "Unknown");

			}
			++temp_count;
		}
		ife("Param") {
			if (element.size() > 0) {
				element.add_attr("type", element[0].get_attr_str("type"));
			}
			if (strcmp(element.get_attr_str("type"), "StringConst") != 0) {
				int t = element[0].get_attr_int("temp");
				element.add_attr("temp", t);
			}
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
		ifb("ParamDecl") {
			const char* s = element.get_attr_str("name");
			Element value = element.qo("ancestor::Function/Scope/Decl").table(s);
			int a = triples.find(value);
			triples.add(Cmd.pop, {}, {}, { a, TT.value });
			cut;
		}
		ifb("Var") {
			Element init = element("/InitValue");
			if (init) {
				init.add_attr("name", element.get_attr_str("name"));
				init.add_attr("type", element.get_attr_str("type"));
			}
		}
		ife("Var") {
			int size = 1;

			const char* s = element.get_attr_str("name");
			Element value = element.table(s);
			if (value.get_attr("size"))
				size = value.get_attr_int("size");

			int a = triples.find(value);

			int type = 0;
			if (strcmp("Float", value.get_attr_str("type")) == 0)
				type = 1;

			triples.add(Cmd.var, { a , TT.value }, { size , TT.dimd }, { type, TT.typetag });
		}
		ife("InitValue") {
			const char* et = element.get_attr_str("type");

			if (!element.get_attr("array")) {
				const char* s = element.get_attr_str("name");
				const char* vt = element[0].get_attr_str("type");
				Element value = element.table(s);
				int a = triples.find(value);
				int t = element[0].get_attr_int("temp");
				//TypeTras(et, vt, t);
				triples.add(Cmd.mov, { t }, {}, { a, TT.value });
			}
			else {
				int count = 0;
				int value_idx = triples.find(element.table(element.get_attr_str("name")));
				for (auto e : element) {
					e.add_attr("start", count);
					count += e.get_attr_int("repeat");
				}
				element.add_attr("size", count);
				triples.add(Cmd.mset,
					{ value_idx, TT.value },
					{ count , TT.dimd }, {});
				for (auto e : element) {
					int len = e.get_attr_int("repeat");
					int base = e.get_attr_int("start");
					const char* vt = e.get_attr_str("type");

					TripleValue t;
					if (e.get_attr("value")) {
						t = { e.get_attr_int("value"), TT.dimd };
					}
					else if (e.get_attr("temp")) {
						t = { e.get_attr_int("temp"), TT.temp };
					}
					if (t.value != 0 || t.type != TT.dimd) {
						for (int i = 0; i < len; ++i) {
							//TypeTras(et, vt, t);
							triples.add(Cmd.mov, t, {},
								{ value_idx, TT.value, {base + i, TT.dimd} });
						}
					}
				}
			}
		}
		ifb("Block") {
			triples.add(Cmd.blkb, { block_count, TT.blockno }, {}, {});
			element.add_attr("block", block_count++);
		}
		ife("Block") {
			if (element.get_attr("block"))
				triples.add(Cmd.blke, { element.get_attr_int("block"), TT.blockno }, {}, {});
		}
	}
	this->temp_count = temp_count;
	setValueTable();
	setFuncParams();
}

void Triples::setValueTable() {
	for (auto& e : value_pointer) {
		assert(e);

		ValueTableElement value;

		value.name = e.get_attr_str("name");
		value.type = strcmp(e.get_attr_str("type"), "Float") == 0; // set 0 int and 1 float;

		value.type |= (e.get_attr("array") ? 1 : 0) << 1; // for array, set 2 int[] and 3 float[]

		value.type += 1; // for void 0, type add 1.

		Element block = e % ("ancestor::Scope/Block");
		if (block) {
			value.block = block.get_attr_int("block");
		}
		else {
			value.block = -1;
		}
		printf("%s, %d, %d\n", value.name.c_str(), value.type, value.block);
		value_table.push_back(value);

	}
}

void Triples::setFuncParams()
{
	/*
	0 = void
	1 = int
	2 = float
	3 = int[]
	4 = float[]
	*/
	// set lib functions
	func_params = {
		//name,   return , params...
		{"putch", {0,	1} },
		{"putint", {0,	1} },
		{"putfloat", {0,	2} },
	};

	for (auto& e : function_pointer) {
		assert(e);
		std::vector<int> param_types;

		// float: -(-1) + 1 = 2; int: (0) + 1 = 1; void: -(1) + 1 = 1 
		int ret = -strcmp(e.get_attr_str("return"), "Int") + 1;
		param_types.push_back(ret);

		auto params = e("/Params/*");
		for (auto param : params) {
			// 同setValueTable
			int type = strcmp(param.get_attr_str("type"), "Float") == 0;
			type |= (param.get_attr("array") ? 1 : 0) << 1;
			type += 1;

			param_types.push_back(type);
		}

		func_params.emplace(e.get_attr_str("name"), param_types);
	}
}
