#include "Element.h"
#include "Triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"
#include "codegen/ashelper.hpp"
#include "codegen/const_inflater.hpp"

int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);
  
	Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());

	// root.print();
	ConstNode_unfold(root);
	ArrayDecl_flatten(root);
  	// printf("===After flatten===\n");
  	// root.print();
	printf("=== ASM Start ===\n");
	AssemblyBuilder asfile(Flag::getFlag().getFlagFor("output").c_str());

	asfile 	| ".global main" // export main symbol
			| ".extern putf getint putint putch getch getarray putarray"; // import libsysy.a
	asfile.line();

	ConstInflater const_inflater(root.unwrap());
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