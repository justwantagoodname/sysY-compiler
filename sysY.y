%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;
extern FILE *yyin;
const char *filename = "testfile.txt";

%}


%token Main = 0 Const 

    Int Void 
    
    While Break If Else Continue Not And Or 

    GetInt Printf 
    
    Return

    Plus, Minus, Mult, Div, Mod, 

    Less, Greater, GreaterEq, LessEq, Equal, NotEq, 

    Assign SemiCon Comma,
    
    LeftParent RightParent

    LeftBrack RightBrack

    LeftBrace RightBrace

    String Integer Identifier

%%
VariableDef:  Int Indent Eq IntConst {printf("Defined %s as %d", $1, $4);}
%%

void yyerror(const char *s) {
  fprintf (stderr, "%s\n", s);
}

int main() {
    yyin = fopen(filename, "r");
    yyparse();
}