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

  root.print();
	ConstNode_unfold(root);
	ArrayDecl_flatten(root);
  printf("===After flatten===\n");
  root.print();

  auto as_file = fopen(Flag::getFlag().getFlagFor("output").c_str(), "w");

  fprintf(as_file, R"(main:
        str     fp, [sp, #-4]!
        add     fp, sp, #0
        mov     r3, #0
        mov     r0, r3
        add     sp, fp, #0
        ldr     fp, [sp], #4
        bx      lr
)");

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