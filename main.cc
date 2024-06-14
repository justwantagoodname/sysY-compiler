#include "Element.h"
#include "Triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"
#include "codegen/ashelper.hpp"

int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);
  
	// Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());

  // root.print();
	// ConstNode_unfold(root);
	// ArrayDecl_flatten(root);
  // printf("===After flatten===\n");
  // root.print();

  AssemblyBuilder asfile("output.s");
  
  asfile| ".data" 
        | "test"
        | "test2"
        | "test3"
        | 123;

#ifdef TRIPLE_DEBUG
	Triples triples(root);
	//triples.pretreat();
	root.print();

	triples.make();
	root.print();

	triples.print();
#endif

	return 0;
}