%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;
extern FILE *yyin;
const char *filename = "testfile.txt";
int yylex(void);

void yyerror(const char *s);

%}

%token 
    Main Const 

    Int Void 
    
    While Break 
    
    If Else Continue 
    
    GetInt Printf 
    
    Return
    
    Not And Or 
    
    Plus  Minus Mult  Div  Mod  

    Less  Greater  GreaterEq  LessEq  Equal  NotEq  

    Assign SemiCon Comma 
    
    LeftParent RightParent

    LeftBrack RightBrack

    LeftBrace RightBrace

    StringConst IntegerConst Identifier

%%
/* resolve confilcts wtf? */
CompUnit: GlobalDecl GlobalFuncDef MainFuncDef {printf("<CompUnit>\n");}
        | GlobalDecl MainFuncDef {printf("<CompUnit>\n");}
        ;

MainFuncDef: Int Main LeftParent RightParent Block {printf("<MainFuncDef>\n");};

GlobalFuncDef: /* empty */
             | GlobalFuncDef FuncDef {printf("<GlobalFuncDef>\n");};

GlobalDecl: /* empty */ 
          | GlobalDecl Decl {printf("<GlobalDecl>\n");}
          ;

Decl: VarDecl 
    | ConstDecl 
    ;

ConstDecl: Const PrimaryType ConstDef SemiCon {printf("<ConstDecl>\n");};

ConstDef: /* empty */ 
        | Identifier Assign ConstInitValue {printf("<ConstDef>\n");}
        | Identifier ArrayDecl Assign ConstInitValue {printf("<ConstDef>\n");}
        | ConstDef Comma ConstDef
        ;

ConstInitValue: ConstExp {printf("<ConstInitVal>\n");};
              | LeftBrace ConstInitValList RightBrace {printf("<ConstInitVal>\n");}
              ;

ConstInitValList: /* empty */
                | ConstInitValue {printf("<ConstInitValList>\n");}
                | ConstInitValList Comma ConstInitValue {printf("<ConstInitValList>\n");}
                ;

VarDecl: PrimaryType VarDef SemiCon {printf("<VarDecl>\n");};

VarDef: /* empty */ 
      | Identifier {printf("<VarDef>\n");}
      | Identifier Assign InitValue {printf("<VarDef>\n");}
      | Identifier ArrayDecl {printf("<VarDef>\n");}
      | Identifier ArrayDecl Assign InitValue {printf("<VarDef>\n");}
      | VarDef Comma VarDef
      ;

ArrayDecl: /* empty */ 
         | LeftBrack ConstExp RightBrack ArrayDecl
         ;

InitValue: Exp {printf("<InitVal>\n");};
         | LeftBrace InitValList RightBrace {printf("<InitVal>\n");};

InitValList: /* empty */
           | InitValue {printf("<InitValList>\n");}
           | InitValList Comma InitValue {printf("<InitValList>\n");}
           ;

Exp: IntegerConst;

ConstExp: IntegerConst {printf("<ConstExp>\n");};

FuncType: Void {printf("<FuncType>\n");}
        | Int {printf("<FuncType>\n");}
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block {printf("<FuncDef>\n");};

FuncFParams: /* empty */
           | FuncFParam {printf("<FuncFParams>\n");}
           | FuncFParam Comma FuncFParams {printf("<FuncFParams>\n");}
           ;

FuncFParam: PrimaryType Identifier {printf("<FuncFParam>\n");}
          | PrimaryType Identifier LeftBrack RightBrack {printf("<FuncFParam>\n");}
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl {printf("<FuncFParam>\n");} 
          ;

Block: LeftBrace BlockItem RightBrace {printf("<Block>\n");};

BlockItem:  /* empty */
         | BlockItem Decl {printf("<BlockItem>\n");}
         ;

PrimaryType: Int;
%%

void yyerror(const char *s) {
  fprintf (stderr, "%s\n", s);
}

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