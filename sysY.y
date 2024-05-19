%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define debug printf
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

FuncType: Void {printf("<FuncType>\n");}
        | Int {printf("<FuncType>\n");}
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block {printf("<FuncDef>\n");};

FuncFParams: /* empty */
           | FuncFParam {printf("<FuncFParams>\n");}
           | FuncFParam Comma FuncFParams {printf("<FuncFParams>\n");}
           ;

FuncFParam: PrimaryType Identifier {printf("<FuncFParam>\n");}
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl {printf("<FuncFParam>\n");} 
          ;

Block: LeftBrace BlockItem RightBrace {printf("<Block>\n");};

BlockItem:  /* empty */
         | BlockItem Decl 
         | BlockItem Stmt 
         ;

PrimaryType: Int;

Stmt: LVal Assign Exp SemiCon {printf("<Stmt>\n");}
    | Exp SemiCon {printf("<Stmt>\n");}
    | Block {printf("<Stmt>\n");}
    | If LeftParent Exp RightParent Stmt {printf("<Stmt>\n");}
    | If LeftParent Exp RightParent Stmt Else Stmt {printf("<Stmt>\n");}
    | While LeftParent Exp RightParent Stmt {printf("<Stmt>\n");}
    | Return Exp SemiCon {printf("<Stmt>\n");}
    | Return SemiCon {printf("<Stmt>\n");}
    | PrintfStmt {printf("<Stmt>\n");}
    | LVal Assign GetInt LeftParent RightParent SemiCon {printf("<Stmt>\n");}
    | Break SemiCon {printf("<Stmt>\n");}
    | Continue SemiCon {printf("<Stmt>\n");}
    ;

PrintfStmt: Printf LeftParent StringConst Comma PrintfArgs RightParent SemiCon {printf("<PrintfStmt>\n");}
          ; 

PrintfArgs: /* empty */
          | Exp {printf("<PrintfArgs>\n");}
          | Exp Comma PrintfArgs {printf("<PrintfArgs>\n");}
          ;

LVal: Identifier {printf("<LVal>\n");}
    | Identifier LeftBrack Exp RightBrack {printf("<LVal>\n");}
    ;

Exp: AddExp;

AddExp: MulExp {printf("<AddExp>\n");}
      | AddExp Plus MulExp {printf("<AddExp>\n");}
      | AddExp Minus MulExp {printf("<AddExp>\n");}
      ;

MulExp: UnaryExp {printf("<MulExp>\n");}
      | MulExp Mult UnaryExp {printf("<MulExp>\n");}
      | MulExp Div UnaryExp {printf("<MulExp>\n");}
      | MulExp Mod UnaryExp {printf("<MulExp>\n");}
      ;

UnaryExp: PrimaryExp {printf("<UnaryExp>\n");}
        | Identifier LeftParent FuncRParams RightParent {printf("<UnaryExp>\n");}
        | UnaryOp UnaryExp {printf("<UnaryExp>\n");}
        ; 

UnaryOp: Plus {printf("<UnaryOp>\n");}
       | Minus {printf("<UnaryOp>\n");}
       | Not {printf("<UnaryOp>\n");}
       ;

FuncRParams: /* empty */
           | Exp {printf("<FuncRParams>\n");}
           | Exp Comma FuncRParams {printf("<FuncRParams>\n");}
           ;

PrimaryExp: LVal {printf("<PrimaryExp>\n");}
          | Number {printf("<PrimaryExp>\n");}
          | LeftParent Exp RightParent {printf("<PrimaryExp>\n");}
          ;

Number: IntegerConst {printf("<Number>\n");};

Cond: LOrExp {printf("<Cond>\n");};

LOrExp: LAndExp {printf("<LOrExp>\n");}
      | LOrExp Or LAndExp {printf("<LOrExp>\n");}
      ;

LAndExp: EqExp {printf("<LAndExp>\n");}
       | LAndExp And EqExp {printf("<LAndExp>\n");}
       ;

EqExp: RelExp {printf("<EqExp>\n");}
     | EqExp Equal RelExp {printf("<EqExp>\n");}
     | EqExp NotEq RelExp {printf("<EqExp>\n");}
     ;

RelExp: AddExp {printf("<RelExp>\n");}
      | RelExp Less AddExp {printf("<RelExp>\n");}
      | RelExp Greater AddExp {printf("<RelExp>\n");}
      | RelExp LessEq AddExp {printf("<RelExp>\n");}
      | RelExp GreaterEq AddExp {printf("<RelExp>\n");}
      ;

ConstExp: AddExp {printf("<ConstExp>\n");}
        ;
%%

void yyerror(const char *s) {
  fprintf (stderr, "%s\n", s);
}

int main(int argc, const char** argv) {
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
    yyparse();
}