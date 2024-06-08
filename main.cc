#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "Element.h"
#include "flag.h"

extern "C" {
  extern int yyparse(struct ASTNode **root);
  extern FILE *yyin;
}

int main(int argc, const char** argv) {
  /* 解析命令行选项 */
  Flag::getFlag().init(argc, argv);

#ifdef OUTPUT_OJ_INPUT
  char c;
  while ((c = fgetc(yyin)) != EOF) {
      if (c == '\n') continue;
      printf("%c", c);
  }
  printf("\n");
  rewind(yyin);
#endif

  Element root = Element::CreateByFile(Flag::getFlag().getFlagFor("input").c_str());
  ConstNode_unfold(root);
  ArrayDecl_flatten(root);
#if 0
  Query result = (root("//Decl") / "*")["array"];

  int count = 0;
  for (auto cur : result) {
      printf("=== Result %d ===\n", ++count);
      cur.print();
      printf("=======\n");
  }
#else
  root.print();
#endif
  return 0;
}