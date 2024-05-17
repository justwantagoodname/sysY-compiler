%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;
extern FILE *yyin;
const char *filename = "testfile.txt";
int yylex(void);

void yyerror(const char *s) {
  fprintf (stderr, "%s\n", s);
}

%}

%token 
    Main Const 

    Int Void 
    
    While Break If Else Continue Not And Or 

    GetInt Printf 
    
    Return

    Plus  Minus Mult  Div  Mod  

    Less  Greater  GreaterEq  LessEq  Equal  NotEq  

    Assign SemiCon Comma 
    
    LeftParent RightParent

    LeftBrack RightBrack

    LeftBrace RightBrace

    StringConst IntegerConst Identifier

%%
VariableDef:  Int Identifier Equal IntegerConst {printf("Defined %d", $4);}
%%

int main(int argc, const char** argv) {
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
            freopen(argv[i+1], "w", stdout);
        }
    }

    yyin = fopen(filename, "r");
    yyparse();
}