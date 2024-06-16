#include "element.h"
#include "triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"
#include "codegen/ashelper.hpp"
#include "codegen/const_inflater.hpp"

#define TRIPLE_DEBUG
#define UNION_OPTIMIZTION

int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);
  
	Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());

	root.print();

#ifdef UNI_OPTIMIZTION
	ConstNode_unfold(root);
	ArrayDecl_flatten(root);
  	root.print();
#endif

#ifdef ASM_GEN
	printf("=== ASM Start ===\n");
	AssemblyBuilder asfile(Flag::getFlag().getFlagFor("output").c_str());

	asfile 	| ".global main" // export main symbol
			| ".extern putf getint putint putch getch getarray putarray"; // import libsysy.a
	asfile.line();

	GlobalDeclInflater const_inflater(root.unwrap());
    const_inflater.inflate(asfile);
	
	asfile	| ".text"	
			| "main:"
			| "	push {fp, lr}"
			| "	add fp, sp, #4" // set up frame pointer
			| "	mov r0, #0"
			| "	pop {fp, pc}"; // return 0
	
	asfile.line()
		  .raw(".section	.note.GNU-stack,\"\",%progbits")
		  .line(); // new line in the end
#endif

#ifdef TRIPLE_DEBUG
	Triples triples(root);
	triples.pretreat();
	printf("===After pretreat===\n");

	root.print();

	triples.make();
	printf("===After make===\n");
	root.print();

	triples.print();
#endif

	return 0;
}