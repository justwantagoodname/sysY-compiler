#include "Element.h"

extern "C" {
  extern int yyparse(struct ASTNode **root);
  extern FILE *yyin;
}

int main(int argc, const char** argv) {
  const char *filename = "testfile.txt";
  const char* output = "output.txt";
  for (int i = 0;i < argc; i++) {
      if (strcmp(argv[i], "-i") == 0) {
          if (i + 1 >= argc) {
            fprintf(stderr, "No input file specified\n");
            return 1;
          }
          filename = argv[i+1];
      }
      if (strcmp(argv[i], "-o") == 0) {
          if (i + 1 >= argc) {
            fprintf(stderr, "No output file specified\n");
            return 1;
          }
          output = argv[i+1];
      }
  }
  if (strcmp(output, "-") != 0) freopen(output, "w", stdout);
  yyin = fopen(filename, "r");

#if 0
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
#if 1
  Element root = Element::CreateByFile(filename);
  Query result = root("//Plus");
  result[0].table("a").print();

  int count = 0;
  for (auto cur : result) {
      printf("=== Result %d ===\n", ++count);
      cur.print();
      printf("=======\n");
  }
#endif
  return 0;
}