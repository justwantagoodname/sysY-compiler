#include "triples.h"
#include <vector>
using std::vector;

void Triples::add(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, const TripleValue& to)
{
	assert(cmd != Cmd.mov || (cmd == Cmd.mov && e1.value < 20000) || e1.type != TT.temp);
	triples.push_back(std::shared_ptr<Triple>(new Triple(cmd, e1, e2, to)));
}

int Triples::find(const Element& e)
{
	assert(e);
	assert(!e.id_is("ArraySize"));

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

//Triples::TripleValue Triples::find(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, int len) const
//{
//	int end = 0;
//	if (len != -1)
//	{
//		end = triples.size() - 1 - len;
//		if (end < 0)end = 0;
//	}
//	for (int i = triples.size() - 1; i >= end; --i) {
//		const Triple& t = triples[i];
//		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
//			return t.to;
//		}
//	}
//	return {};
//}
//
//Triples::TripleValue Triples::find(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2) const
//{
//	int end = 0;
//	for (int i = triples.size() - 1; i >= end; --i) {
//		const Triple& t = triples[i];
//		if (t.cmd == Cmd.tag)
//			break;
//		if (t.cmd == cmd && t.e1 == e1 && t.e2 == e2) {
//			return t.to;
//		}
//	}
//	return {};
//}

Triples::Triples(const Element& e) : root(e), temp_count(-1) {
	triples.clear();
	value_pointer.clear();
	function_pointer.clear();
	string_pointer.clear();
	temp_type.clear();
}

Triples::Triple::Triple(CMD::CMD_ENUM cmd, const TripleValue& e1, const TripleValue& e2, const TripleValue& to) : cmd(cmd), e1(e1), e2(e2), to(to) {}

Triples::~Triples()
{
	//root.free();
}

size_t Triples::size()
{
	return triples.size();
}

void Triples::print() const
{

	char ts1[550], ts2[550], ts3[550];
	int idx = 0;
	printf("\n");
	const char cmd_text[][10] = {
		"mov",
		"call",
		"jmp",
		"j!0",
		"jeq",
		"jne",
		"jgt",
		"jlt",
		"jge",
		"jle",
		//"jeqf",
		//"jnef",
		//"jgtf",
		//"jltf",
		//"jgef",
		//"jlef",
		"ret",
		"rev",
		"pus",
		"pop",
		"add",
		"sub",
		"mul",
		"div",
		"mod",
		//"fadd",
		//"fsub",
		//"fmul",
		//"fdiv",
		"tag",
		//"d2f",
		//"f2d",
		"mset",
		"load",
		"store",
		"{",
		"}",
		"var"
	};
	for (auto i : triples) {

		if (i->cmd == Cmd.tag) {
			i->e1.toString(ts1, *this);
			printf("%d- %s :\n", idx, ts1);
		}
		else if (i->cmd == Cmd.blkb || i->cmd == Cmd.blke) {
			i->e1.toString(ts1, *this);
			printf("%d>\t%s(%s)\n", idx, cmd_text[i->cmd], ts1);
		}
		else {
			i->e1.toString(ts1, *this);
			i->e2.toString(ts2, *this);
			i->to.toString(ts3, *this);
			printf("%d> \t%s, %s, %s, %s\n", idx, cmd_text[i->cmd], ts1, ts2, ts3);
		}
		++idx;
	}
}

bool Triples::TripleValue::operator==(const Triples::TripleValue& t) const
{
	return value == t.value && type == t.type
		&& ((added != nullptr && t.added != nullptr && (*added) == (*t.added))
			|| (added == nullptr && t.added == nullptr));
}

bool Triples::TripleValue::operator!=(const Triples::TripleValue& t) const
{
	return value != t.value || type != t.type
		|| ((added != nullptr && t.added != nullptr && (*added) != (*t.added))
			|| ((added == nullptr) ^ (t.added == nullptr)));
}

Triples::TripleValue::TripleValue(int v,
	Triples::TripleType ty, const Triples::TripleValue& at)
	: value(v), type(ty), added(nullptr)
{
	if (at.type != TT.null)
		added = new TripleValue(at);
	else
		added = nullptr;
}

Triples::TripleValue::TripleValue(const char* str, Triples* triple)
	: type(TT.str), added(nullptr)
{
	int i = 0;
	for (; i < triple->string_pointer.size(); ++i) {
		if (triple->string_pointer[i] == str) {
			value = i;
			return;
		}
	}
	value = i;
	triple->string_pointer.push_back(str);
}

Triples::TripleValue::TripleValue(const TripleValue& at)
	: value(at.value), type(at.type)
{
	if (at.added != nullptr)
		added = new TripleValue(*(at.added));
	else
		added = nullptr;
}

Triples::TripleValue& Triples::TripleValue::operator=(const Triples::TripleValue& at) {
	value = at.value;
	type = at.type;
	if (at.added != nullptr)
		added = new TripleValue(*(at.added));
	else
		added = nullptr;
	return *this;
}

Triples::TripleValue::~TripleValue()
{
	if (added)
		delete added;
}

void Triples::TripleValue::toString(char s[], const Triples& triples)
{
	char ts[500], ts2[500];
	switch (type)
	{
	case TT.null:
		snprintf(s, 20, "-");
		break;
	case TT.temp:
		if (triples.temp_type[value] == 2)
			snprintf(s, 20, "fT%d", value);
		else
			snprintf(s, 20, "T%d", value);
		break;
	case TT.dimd:
		snprintf(s, 40, "#%d", value);
		break;
	case TT.fimd:
		snprintf(s, 40, "#%f", *(float*)(&value));
		break;
	case TT.value:
		if (added == nullptr) {
			snprintf(s, 40, "%s(%d)", triples.value_pointer[value].get_attr_str("name"), value);
		}
		else {
			added->toString(ts, triples);
			snprintf(s, 60, "%s[%s]", triples.value_pointer[value].get_attr_str("name"), ts);
		}
		break;
	case TT.func:
		if (triples.function_pointer[value].get_attr("ex_func"))
			snprintf(s, 500, "lib@%s",
				triples.function_pointer[value].get_attr_str("name")
			);
		else
			snprintf(s, 500, "%d@%s",
				triples.function_pointer[value].get_attr_int("place"),
				triples.function_pointer[value].get_attr_str("name")
			);
		break;
	case TT.lamb:
		snprintf(s, 20, ".l%d", value);
		break;
	case TT.str:
		snprintf(s, 100, "\"%s\"", triples.string_pointer[value].c_str());
		break;
	case TT.parms:
		ts[0] = '\0';
		for (auto t = added; t; t = t->added) {
			t->toString(ts2, triples);
			if (t != added)
				strcat(ts, ", ");
			strcat(ts, ts2);
		}
		snprintf(s, 500, "{%s}", ts);
		break;
	case TT.typetag:
		snprintf(s, 20, value ? "Float" : "Int");
		break;
	case TT.blockno:
		snprintf(s, 20, "b%d", value);
		break;
	case TT.addr:
		added->toString(ts, triples);
		snprintf(s, 20, "&%s + %s", triples.value_pointer[value].get_attr_str("name"), ts);
		break;
	default:
		snprintf(s, 20, "unknow:%d", value);
		break;
	}
}

std::string Triples::getFuncName(const TripleValue& tv) const {
	assert(tv.type == TT.func);
	return function_pointer[tv.value].get_attr_str("name");
}

std::string Triples::getValueString(const TripleValue& tv) const {
	assert(tv.type == TT.str);
	return string_pointer[tv.value];
}

std::string Triples::getLabelName(const TripleValue& tv) const {
	assert(tv.type == TT.lamb);
	return ".l" + std::to_string(tv.value);
}

std::string Triples::getVarName(const TripleValue& tv) const {
	assert(tv.type == TT.value);
	return value_pointer[tv.value].get_attr_str("name");
}

int Triples::getValueType(const TripleValue& e) {
	int t;
	switch (e.type)
	{
	case TT.temp:
		return temp_type[e.value];
		break;
	case TT.dimd:
		return 1;
		break;
	case TT.fimd:
		return 2;
		break;
	case TT.str:
		return 6;
		break;
	case TT.value:
		return (strcmp(value_pointer[e.value].get_attr_str("type"), "Float") == 0) + 1;
		break;
	case TT.func:
		t = strcmp(function_pointer[e.value].get_attr_str("return"), "Int");
		t = t > 0 ? 1 : t < 0 ? -1 : 0;
		return -t + 1;
		break;
	case TT.addr:
		return (strcmp(value_pointer[e.value].get_attr_str("type"), "Float") == 0) + 3;
		break;
	case TT.null:
		return 0;
		break;
	default:
		panic("gettype error");
		break;
	}
}