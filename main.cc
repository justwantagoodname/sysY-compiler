#include "element.h"
#include "triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"
#include "codegen/asm_helper.hpp"
#include "codegen/const_inflater.hpp"
#include "codegen/stack_translator.hpp"
#include "codegen/arm_adapter.hpp"

int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);
  
	Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());

#ifdef UNI_OPTIMIZTION
	ConstNode_unfold(root);
	ArrayDecl_flatten(root);
  	// root.print();
#endif

#ifdef ASM_GEN
	printf("=== ASM Start ===\n");
	AssemblyBuilder asfile(Flag::getFlag().getFlagFor("output").c_str());

	asfile | ".extern putf getint putint putch getch getarray putarray"; // import libsysy.a
	asfile.line();

	GlobalDeclInflater const_inflater(root.unwrap());
    const_inflater.inflate(asfile);

    ARMAdapter arm_adapter(asfile);

    StackTranslator translator(root.unwrap(), std::make_unique<ARMAdapter>(arm_adapter));
	translator.translate();

	asfile.line()
		  .line(".section	.note.GNU-stack,\"\",%%progbits");
	root.print();
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