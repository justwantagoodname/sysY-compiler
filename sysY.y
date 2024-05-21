%{

#include "sysY.h"

%}

%locations

%glr-parser

%expect 2

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
        ;

MainFuncDef: Int Main LeftParent RightParent Block {printf("<MainFuncDef>\n");}
           ;

GlobalDecl: /* empty */ 
          | GlobalDecl Decl
          ;

GlobalFuncDef: /* empty */
             | GlobalFuncDef FuncDef
             ;

Decl: VarDecl 
    | ConstDecl 
    ;

ConstDecl: Const PrimaryType ConstDefList SemiCon {printf("<ConstDecl>\n");}
         ;

ConstDefList: ConstDef
            | ConstDefList Comma ConstDef
            ;

ConstDef: Identifier Assign ConstInitValue {printf("<ConstDef>\n");}
        | Identifier ArrayDecl Assign ConstInitValue {printf("<ConstDef>\n");}
        ;

ConstInitValue: ConstExp {printf("<ConstInitVal>\n");}
              | LeftBrace ConstInitValList RightBrace {printf("<ConstInitVal>\n");}
              ;

ConstInitValList: /* empty */
                | ConstInitValue /* {printf("<ConstInitValList>\n");} */
                | ConstInitValList Comma ConstInitValue /* {printf("<ConstInitValList>\n");} */
                ;

VarDecl: PrimaryType VarDefList SemiCon {printf("<VarDecl>\n");}
       ;

VarDefList: VarDef
          | VarDefList Comma VarDef
          ;

VarDef: Identifier {printf("<VarDef>\n");}
      | Identifier Assign InitValue {printf("<VarDef>\n");}
      | Identifier ArrayDecl {printf("<VarDef>\n");}
      | Identifier ArrayDecl Assign InitValue {printf("<VarDef>\n");}
      ;

ArrayDecl: LeftBrack ConstExp RightBrack 
         | LeftBrack ConstExp RightBrack ArrayDecl
         ;

InitValue: Exp {printf("<InitVal>\n");};
         | LeftBrace InitValList RightBrace {printf("<InitVal>\n");}
         ;

InitValList: /* empty */
           | InitValue {printf("<InitValList>\n");}
           | InitValList Comma InitValue {printf("<InitValList>\n");}
           ;

FuncType: Void {printf("<FuncType>\n");}
        | Int {printf("<FuncType>\n");}
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block {printf("<FuncDef>\n");} 
       ;

FuncFParams: /* empty */ 
           | FuncFParamList {printf("<FuncFParams>\n");}
           ;

FuncFParamList:  FuncFParam 
              | FuncFParam Comma FuncFParamList
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
    | SemiCon {printf("<Stmt>\n");}
    | Exp SemiCon {printf("<Stmt>\n");}
    | Block {printf("<Stmt>\n");}
    | IfStmt {printf("<Stmt>\n");}
    | While LeftParent Cond RightParent Stmt {printf("<Stmt>\n");}
    | Return Exp SemiCon {printf("<Stmt>\n");}
    | Return SemiCon {printf("<Stmt>\n");}
    | PrintfStmt {printf("<Stmt>\n");}
    | LVal Assign GetInt LeftParent RightParent SemiCon {printf("<Stmt>\n");}
    | Break SemiCon {printf("<Stmt>\n");}
    | Continue SemiCon {printf("<Stmt>\n");}
    ;

/* attach else to cloest if stmt */
/* see https://www.gnu.org/software/bison/manual/html_node/Merging-GLR-Parses.html */
IfStmt: If LeftParent Cond RightParent Stmt /* {printf("<IFStmt>\n");} */ %dprec 2
      | If LeftParent Cond RightParent Stmt Else Stmt /* {printf("<IFStmtElse>\n");} */ %dprec 1
      ;

PrintfStmt: Printf LeftParent PrintfArgs RightParent SemiCon /* {printf("<PrintfStmt>\n");} */
          ; 

PrintfArgs: StringConst
          | StringConst PrintfVarArgs
          ;

PrintfVarArg: Comma Exp
            ;

PrintfVarArgs: PrintfVarArg /* {printf("<PrintfVarArgs>\n");} */
             | PrintfVarArg Comma PrintfVarArgs /* {printf("<PrintfVarArgs>\n");} */
             ;

LVal: Identifier {printf("<LVal>\n");}
    | Identifier ArrayLocatorList {printf("<LVal>\n");}
    ;

ArrayLocator: LeftBrack Exp RightBrack
            ;

ArrayLocatorList: ArrayLocator
                | ArrayLocator ArrayLocatorList
                ;

Exp: AddExp {printf("<Exp>\n");}
   ;

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
           | FuncRParamList {printf("<FuncRParams>\n");}
           ;

FuncRParamList: Exp 
              | Exp Comma FuncRParamList 
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
  yyparse();
}