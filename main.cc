#include "Element.h"
#include "Triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"

int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);
  
	Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());
	//ConstNode_unfold(root);
	//ArrayDecl_flatten(root);


	Triples triples(root);
	//triples.pretreat();
	root.print();

	triples.make();
	root.print();

	triples.print();

	return 0;
}