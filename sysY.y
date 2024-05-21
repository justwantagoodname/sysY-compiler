%{

#include "sysY.h"
#include "token.h"

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
CompUnit: GlobalDecl GlobalFuncDef MainFuncDef {print_tokens(@$.last_line, @$.last_column); printf("<CompUnit>\n");}
        ;

MainFuncDef: Int Main LeftParent RightParent Block {print_tokens(@$.last_line, @$.last_column); printf("<MainFuncDef>\n");}
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

ConstDecl: Const PrimaryType ConstDefList SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<ConstDecl>\n");}
         ;

ConstDefList: ConstDef
            | ConstDefList Comma ConstDef
            ;

ConstDef: Identifier Assign ConstInitValue {print_tokens(@$.last_line, @$.last_column); printf("<ConstDef>\n");}
        | Identifier ArrayDecl Assign ConstInitValue {print_tokens(@$.last_line, @$.last_column); printf("<ConstDef>\n");}
        ;

ConstInitValue: ConstExp {print_tokens(@$.last_line, @$.last_column); printf("<ConstInitVal>\n");}
              | LeftBrace ConstInitValList RightBrace {print_tokens(@$.last_line, @$.last_column); printf("<ConstInitVal>\n");}
              ;

ConstInitValList: /* empty */
                | ConstInitValue /* {printf("<ConstInitValList>\n");} */
                | ConstInitValList Comma ConstInitValue /* {printf("<ConstInitValList>\n");} */
                ;

VarDecl: PrimaryType VarDefList SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<VarDecl>\n");}
       ;

VarDefList: VarDef
          | VarDefList Comma VarDef
          ;

VarDef: Identifier {print_tokens(@$.last_line, @$.last_column); printf("<VarDef>\n");}
      | Identifier Assign InitValue {print_tokens(@$.last_line, @$.last_column); printf("<VarDef>\n");}
      | Identifier ArrayDecl {print_tokens(@$.last_line, @$.last_column); printf("<VarDef>\n");}
      | Identifier ArrayDecl Assign InitValue {print_tokens(@$.last_line, @$.last_column); printf("<VarDef>\n");}
      ;

ArrayDecl: LeftBrack ConstExp RightBrack 
         | LeftBrack ConstExp RightBrack ArrayDecl
         ;

InitValue: Exp {print_tokens(@$.last_line, @$.last_column); printf("<InitVal>\n");};
         | LeftBrace InitValList RightBrace {print_tokens(@$.last_line, @$.last_column); printf("<InitVal>\n");}
         ;

InitValList: /* empty */
           | InitValue {print_tokens(@$.last_line, @$.last_column); printf("<InitValList>\n");}
           | InitValList Comma InitValue {print_tokens(@$.last_line, @$.last_column); printf("<InitValList>\n");}
           ;

FuncType: Void {print_tokens(@$.last_line, @$.last_column); printf("<FuncType>\n");}
        | Int {print_tokens(@$.last_line, @$.last_column); printf("<FuncType>\n");}
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block {print_tokens(@$.last_line, @$.last_column); printf("<FuncDef>\n");} 
       ;

FuncFParams: /* empty */ 
           | FuncFParamList {print_tokens(@$.last_line, @$.last_column); printf("<FuncFParams>\n");}
           ;

FuncFParamList:  FuncFParam 
              | FuncFParam Comma FuncFParamList
              ;

FuncFParam: PrimaryType Identifier {print_tokens(@$.last_line, @$.last_column); printf("<FuncFParam>\n");}
          |  PrimaryType Identifier LeftBrack RightBrack {print_tokens(@$.last_line, @$.last_column); printf("<FuncFParam>\n");} 
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl {print_tokens(@$.last_line, @$.last_column); printf("<FuncFParam>\n");} 
          ;

Block: LeftBrace BlockItem RightBrace {print_tokens(@$.last_line, @$.last_column); printf("<Block>\n");};

BlockItem:  /* empty */
         | BlockItem Decl 
         | BlockItem Stmt 
         ;

PrimaryType: Int;

Stmt: LVal Assign Exp SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Exp SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Block {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | IfStmt {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | While LeftParent Cond RightParent Stmt {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Return Exp SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Return SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | PrintfStmt {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | LVal Assign GetInt LeftParent RightParent SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Break SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
    | Continue SemiCon {print_tokens(@$.last_line, @$.last_column); printf("<Stmt>\n");}
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
             | PrintfVarArg PrintfVarArgs /* {printf("<PrintfVarArgs>\n");} */
             ;

LVal: Identifier {print_tokens(@$.last_line, @$.last_column); printf("<LVal>\n");}
    | Identifier ArrayLocatorList {print_tokens(@$.last_line, @$.last_column); printf("<LVal>\n");}
    ;

ArrayLocator: LeftBrack Exp RightBrack
            ;

ArrayLocatorList: ArrayLocator
                | ArrayLocator ArrayLocatorList
                ;

Exp: AddExp {print_tokens(@$.last_line, @$.last_column); printf("<Exp>\n");}
   ;

AddExp: MulExp {print_tokens(@$.last_line, @$.last_column); printf("<AddExp>\n");}
      | AddExp Plus MulExp {print_tokens(@$.last_line, @$.last_column); printf("<AddExp>\n");}
      | AddExp Minus MulExp {print_tokens(@$.last_line, @$.last_column); printf("<AddExp>\n");}
      ;

MulExp: UnaryExp {print_tokens(@$.last_line, @$.last_column); printf("<MulExp>\n");}
      | MulExp Mult UnaryExp {print_tokens(@$.last_line, @$.last_column); printf("<MulExp>\n");}
      | MulExp Div UnaryExp {print_tokens(@$.last_line, @$.last_column); printf("<MulExp>\n");}
      | MulExp Mod UnaryExp {print_tokens(@$.last_line, @$.last_column); printf("<MulExp>\n");}
      ;

UnaryExp: PrimaryExp {print_tokens(@$.last_line, @$.last_column); printf("<UnaryExp>\n");}
        | Identifier LeftParent FuncRParams RightParent {print_tokens(@$.last_line, @$.last_column); printf("<UnaryExp>\n");}
        | UnaryOp UnaryExp {print_tokens(@$.last_line, @$.last_column); printf("<UnaryExp>\n");}
        ; 

UnaryOp: Plus {print_tokens(@$.last_line, @$.last_column); printf("<UnaryOp>\n");}
       | Minus {print_tokens(@$.last_line, @$.last_column); printf("<UnaryOp>\n");}
       | Not {print_tokens(@$.last_line, @$.last_column); printf("<UnaryOp>\n");}
       ;

FuncRParams: /* empty */ 
           | FuncRParamList {print_tokens(@$.last_line, @$.last_column); printf("<FuncRParams>\n");}
           ;

FuncRParamList: Exp 
              | Exp Comma FuncRParamList 
              ;

PrimaryExp: LVal {print_tokens(@$.last_line, @$.last_column); printf("<PrimaryExp>\n");}
          | Number {print_tokens(@$.last_line, @$.last_column); printf("<PrimaryExp>\n");}
          | LeftParent Exp RightParent {print_tokens(@$.last_line, @$.last_column); printf("<PrimaryExp>\n");}
          ;

Number: IntegerConst {print_tokens(@$.last_line, @$.last_column); printf("<Number>\n");};

Cond: LOrExp {print_tokens(@$.last_line, @$.last_column); printf("<Cond>\n");};

LOrExp: LAndExp {print_tokens(@$.last_line, @$.last_column); printf("<LOrExp>\n");}
      | LOrExp Or LAndExp {print_tokens(@$.last_line, @$.last_column); printf("<LOrExp>\n");}
      ;

LAndExp: EqExp {print_tokens(@$.last_line, @$.last_column); printf("<LAndExp>\n");}
       | LAndExp And EqExp {print_tokens(@$.last_line, @$.last_column); printf("<LAndExp>\n");}
       ;

EqExp: RelExp {print_tokens(@$.last_line, @$.last_column); printf("<EqExp>\n");}
     | EqExp Equal RelExp {print_tokens(@$.last_line, @$.last_column); printf("<EqExp>\n");}
     | EqExp NotEq RelExp {print_tokens(@$.last_line, @$.last_column); printf("<EqExp>\n");}
     ;

RelExp: AddExp {print_tokens(@$.last_line, @$.last_column); printf("<RelExp>\n");}
      | RelExp Less AddExp {print_tokens(@$.last_line, @$.last_column); printf("<RelExp>\n");}
      | RelExp Greater AddExp {print_tokens(@$.last_line, @$.last_column); printf("<RelExp>\n");}
      | RelExp LessEq AddExp {print_tokens(@$.last_line, @$.last_column); printf("<RelExp>\n");}
      | RelExp GreaterEq AddExp {print_tokens(@$.last_line, @$.last_column); printf("<RelExp>\n");}
      ;

ConstExp: AddExp {print_tokens(@$.last_line, @$.last_column); printf("<ConstExp>\n");}
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