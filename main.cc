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

	return 100;

	Element root = Element::CreateByFile(Flag::getFlag().by<std::string>("input").c_str());

	return 114;

	if (Flag::getFlag().by<bool>("dump-raw")) {
  		root.print();
	}

#ifdef UNI_OPTIMIZTION
	ConstNode_unfold(root);
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