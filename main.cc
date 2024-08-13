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
#define UNI_OPTIMIZTION

 //#define ASM_GEN
//#define TRIPLE_DEBUG
#define ARM_TRIPLE_DEBUG
// #define RV_ASM_GEN

#ifdef TRIPLE_DEBUG
#include "codegen/stack_rv_generator.h"
#endif
#ifdef RV_ASM_GEN
#include "codegen/stack_rv_generator.h"
#endif


int main(int argc, const char** argv) {
	/* 解析命令行选项 */
	Flag::getFlag().init(argc, argv);

	Element root = Element::CreateByFile(Flag::getFlag().by<std::string>("input").c_str());

	if (Flag::getFlag().by<bool>("dump-raw")) {
  		 root.print();
	}

#ifdef UNI_OPTIMIZTION
	ConstNode_fold(root);
	ArrayDecl_flatten(root);
	if (Flag::getFlag().by<bool>("dump-optimized-tree")) {
  		 root.print();
	}
#endif
	
#ifdef ASM_GEN
	AssemblyBuilder asm_file(Flag::getFlag().by<std::string>("output").c_str());

	GlobalDeclInflater const_inflater(root.unwrap());
    const_inflater.inflate(asm_file);

    ARMAdapter arm_adapter(asm_file);

    StackTranslator translator(root.unwrap(), std::make_unique<ARMAdapter>(arm_adapter));
	translator.translate();

	if (Flag::getFlag().by<bool>("dump-generated-tree")) {
  		root.print();
	}
#endif

#ifdef TRIPLE_DEBUG
	// root.print();

	Triples triples(root);
	triples.pretreat();
	printf("===After pretreat===\n");

	root.print();

	printf("===After make===\n");
	triples.make();
	root.print();
	// triples.print();

	printf("===After eliUnnecVar===\n");
	triples.eliUnnecVar();
	// triples.print();

	printf("===After MinTemp===\n");
	triples.minTempVar();
	// triples.print();

	printf("===After ResortTemp===\n");
	triples.resortTemp();
	triples.print();

	StackRiscVGenerator g;
	g.generate(triples, false);

	AssemblyBuilder asm_file(Flag::getFlag().by<std::string>("output").c_str());
	asm_file.raw(".global main\n.text\n.align 2\n.type main, %function\n");
	for (auto p : g.instrs) {
		asm_file.raw(p->toASM().c_str());
	}
	asm_file.raw(".section	.note.GNU-stack,\"\",%progbits\n.ident	\"SysY-Compiler\"\n");
#endif

#ifdef ARM_TRIPLE_DEBUG
	// root.print();

	Triples triples(root);
	triples.pretreat();
	printf("===After pretreat===\n");

	root.print();

	printf("===After make===\n");
	triples.make();
	root.print();
	// triples.print();

	printf("===After eliUnnecVar===\n");
	triples.eliUnnecVar();
	// triples.print();

	printf("===After MinTemp===\n");
	triples.minTempVar();
	// triples.print();

	printf("===After ResortTemp===\n");
	triples.resortTemp();
	triples.print();

	panic("DOING……");

	AssemblyBuilder asm_file(Flag::getFlag().by<std::string>("output").c_str());
	asm_file.raw(".global main\n.text\n.align 2\n.type main, %function\n");
	//for (auto p : g.instrs) {
	//	asm_file.raw(p->toASM().c_str());
	//}
	asm_file.raw(".section	.note.GNU-stack,\"\",%progbits\n.ident	\"SysY-Compiler\"\n");
#endif

#ifdef RV_ASM_GEN
	AssemblyBuilder asm_file(Flag::getFlag().by<std::string>("output").c_str());

	// GlobalDeclInflater const_inflater(root.unwrap());
    // const_inflater.inflate(asm_file);

	asm_file.raw(".global main\n.text\n.align 2\n.type main, %function\n");

	Triples triples(root);
	triples.pretreat();
	triples.make();
	triples.eliUnnecVar();
	triples.minTempVar();
	triples.resortTemp();

	StackRiscVGenerator g;
	g.generate(triples, false);
	for (auto p : g.instrs) {
		asm_file.raw(p->toASM().c_str());
	}
	asm_file.raw(".section	.note.GNU-stack,\"\",%progbits\n.ident	\"SysY-Compiler\"\n");
#endif

	return 0;
}