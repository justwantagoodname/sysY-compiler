#include "Triples.h"
#include <vector>
using std::vector;

void Triples::add(int cmd, int e1, int e2, int to)
{
	triples.push_back(IntTriple(cmd, e1, e2, to));
}

int Triples::find(const Element e)
{
	for (int i = 0; i < value_pointer.size(); ++i) {
		if (value_pointer[i] == e)return i;
	}
	value_pointer.push_back(e);
	return value_pointer.size() - 1;
}

int Triples::find(int cmd, int e1, int e2, int len = -1) const
{
	for (int i = triples.size() - 1; i >= len == -1 ? 0 : triples.size() - 1 - len; --i) {
		const IntTriple& t = triples[i];
		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
			return t.to;
		}
	}
	return -1;
}

Triples::Triples(const Element& e) : root(e) {}

Triples::IntTriple::IntTriple(int cmd, int e1, int e2, int to) : cmd(cmd), e1(e1), e2(e2), to(to) {}

void Triples::make()
{
	Triples& triples = *this;
	int temp_count = 0; // 临时变量计数器 
	printf("makeing...\n");

	for (auto element : root) {
		const char* id = element.id();
		const bool flag = element.flag;

#define beginElement(eqid) (!strcmp(id, (eqid)) && !flag)
#define endElement(eqid) (!strcmp(id, (eqid)) && flag)
#define ife(eqid) else if (endElement(eqid))
#define ifb(eqid) else if (beginElement(eqid))

		//printf("loading ");
		if (0);
		//ife("Exp") {
		//	printf("Exp\n");
		//	int t = element[0].get_attr_int("temp");
		//	//triples.add(Cmd.mov, t, 0, temp_count);
		//	element.add_attr("temp", t);
		//	//++temp_count;
		//}
		//ife("Dest") {
		//	printf("Dest\n");
		//	int t = element[0].get_attr_int("temp");
		//}
		//ife("Address") {
		//	printf("Address\n");
		//	element.print();
		//	const char* s = element.get_attr_str("base");
		//	Element value = element.table(s);
		//	element.add_attr("addr", triples.find(value));
		//}
		//ife("Dest") {
		//	int a = element[0].get_attr_int("addr");
		//	element.add_attr("addr", a);
		//}
		//ife("Fetch") {
		//	int a = element[0].get_attr_int("addr");
		//	triples.add(Cmd.read, a, 0, temp_count);
		//	element.add_attr("temp", temp_count);
		//	++temp_count;
		//}
		//ife("Plus") {
		//	int t0 = element[0].get_attr_int("temp");
		//	int t1 = element[1].get_attr_int("temp");
		//	triples.add(Cmd.add, t0, t1, temp_count);
		//	element.add_attr("temp", temp_count);
		//	++temp_count;
		//}
		//ife("Assign") {
		//	int a = element[0].get_attr_int("addr");
		//	int t = element[1].get_attr_int("temp");
		//	triples.add(Cmd.write, t, 0, a);
		//}
	}
}

