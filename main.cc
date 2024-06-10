#include "Element.h"
#include "Triples.h"
#include "sysY.h"
#include "ast.h"
#include "pass.h"
#include "flag.h"

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
#if 0
  struct ASTNode *root = NULL;
  int result = yyparse(&root);
  if (result == 0) {
    printf("====AST Info====\n");
    // ASTNode_print(root);
    QueryResult* result = ASTNode_querySelector(root, "/"), *cur = NULL;

    int count = 0;
    DL_FOREACH(result, cur) {
      printf("=== Result %d ===\n", ++count);
      ASTNode_print(cur->node);
      printf("=======\n");
    }
  }
#endif
#if 0
  Element root = Element::CreateByFile(filename);
  Query result = root("/");
  //result[0].table("a").print();

  printf("========\n");
  for (auto i : root("//Plus")[0]) {
      printf("<%s%s>\n", i.id(), i.flag ? "/" : "");
  }

  int count = 0;
  for (auto cur : result) {
      printf("=== Result %d ===\n", ++count);
      cur.print();
      printf("=======\n");
  }
#endif
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
  //ConstNode_unfold(root);
  //ArrayDecl_flatten(root);

  root.print();

  // Triples triples(root);
  //triples.pretreat();
  //root.print();

  // triples.make();
  root.print();

  // triples.print();

  return 0;
}