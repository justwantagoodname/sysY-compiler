%{

#include "sysY.h"
#include "token.h"
#include "sym.h"
#include "ast.h"
#include "action.h"
#include "lib/uthash.h"

int count;

void yyerror(struct ASTNode **cur, const char *s);
%}

%locations

%glr-parser

%expect 2

%start CompUnit

%union {
  int intValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;
  struct ValueSymbol *valueSymbol;
  enum ValueType valueType;
}

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


%token <intValue> IntegerConst

%token <strValue> StringConst Identifier

%type <intValue> Number

%type <astNode> CompUnit GlobalFuncDef MainFuncDef Block BlockItem Stmt LVal 
                Exp UnaryExp PrimaryExp ExpWrapper
                FuncRParams FuncRParamList

%type <strValue> UnaryOp

%type <valueSymbol> VarDecl VarDefList VarDef Decl GlobalDecl

%type <valueType> PrimaryType

%left Or
%left And
%left Equal NotEq
%left Greater GreaterEq Less LessEq
%left Plus Minus
%left Div Mult Mod

%parse-param {struct ASTNode **root}

%%
CompUnit: GlobalDecl GlobalFuncDef MainFuncDef { Scope *global = Scope_create(NULL, "Global"); 
                                                 global->vSymbols = $1;
                                                 *root = $$ = ASTNode_create("CompUnit", global);
                                                 ASTNode_add_child($$, $3);
                                                }
        ;

MainFuncDef: Int Main LeftParent RightParent Block { $$ = ASTNode_create("Main", NULL); 
                                                     ASTNode_add_child($$, $5); }
           ;

GlobalDecl: /* empty */ { $$ = NULL; }
          | GlobalDecl Decl { $$ = appendVSList($1, $2); }
          ;

GlobalFuncDef: /* empty */
             | GlobalFuncDef FuncDef
             ;

Decl: VarDecl { $$ = $1; }
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
                | ConstInitValue
                | ConstInitValList Comma ConstInitValue
                ;

VarDecl: PrimaryType VarDefList SemiCon { modifyVSType($2, $1); $$ = $2; }
       ;

VarDefList: VarDef { $$ = addVSArray(NULL, $1); }
          | VarDefList Comma VarDef { $$ = addVSArray($1, $3);}
          ;

VarDef: Identifier { $$ = ValueSymbol_create($1, ANY, NULL); }
      | Identifier Assign InitValue { $$ = ValueSymbol_create($1, ANY, NULL); }
      | Identifier ArrayDecl { $$ = ValueSymbol_create($1, ANY, NULL); }
      | Identifier ArrayDecl Assign InitValue { $$ = ValueSymbol_create($1, ANY, NULL); }
      ;

ArrayDecl: LeftBrack ConstExp RightBrack 
         | LeftBrack ConstExp RightBrack ArrayDecl
         ;

InitValue: Exp {print_tokens(@$.last_line, @$.last_column); printf("<InitVal>\n");};
         | LeftBrace InitValList RightBrace {print_tokens(@$.last_line, @$.last_column); printf("<InitVal>\n");}
         ;

InitValList: /* empty */
           | InitValue /* {print_tokens(@$.last_line, @$.last_column); printf("<InitValList>\n");} */
           | InitValList Comma InitValue /* {print_tokens(@$.last_line, @$.last_column); printf("<InitValList>\n");} */
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

Block: LeftBrace BlockItem RightBrace { $$ = $2; };

BlockItem:  /* empty */ { $$ = ASTNode_create("Block", NULL); /* TODO: Add Scope */ }
         | BlockItem Decl { /* TODO: collect decls to scope */ }
         | BlockItem Stmt { ASTNode_add_child($1, $2); $$ = $1; }
         ;

PrimaryType: Int { $$ = INT; }
           ;

Stmt: LVal Assign ExpWrapper SemiCon { $$ = ASTNode_create("Assign", NULL); ASTNode* dest = ASTNode_create("Dest", NULL); ASTNode_add_child(dest, $1); ASTNode_add_child($$, dest); ASTNode_add_child($$, $3);}
    | SemiCon { $$ = ASTNode_create("NOP", NULL); }
    | ExpWrapper SemiCon { $$ = $1; }
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

LVal: Identifier { $$ = ASTNode_create("Address", NULL); ASTNode_add_attr_str($$, "base", $1); }
    | Identifier ArrayLocatorList { $$ = ASTNode_create("Fetch", NULL); ASTNode_add_attr_str($$, "base", $1); /* TODO: calc base */ }
    ;

ArrayLocator: LeftBrack Exp RightBrack
            ;

ArrayLocatorList: ArrayLocator
                | ArrayLocator ArrayLocatorList
                ;

ExpWrapper: Exp { $$ = ASTNode_create("Exp", NULL); ASTNode_add_child($$, $1); }
          ;

Exp: Exp Or Exp       { $$ = createOpNode("Or", $1, $3);        }
   | Exp And Exp       { $$ = createOpNode("And", $1, $3);       }
   | Exp Equal Exp     { $$ = createOpNode("Equal", $1, $3);     }
   | Exp NotEq Exp     { $$ = createOpNode("NotEq", $1, $3);     }
   | Exp Less Exp      { $$ = createOpNode("Less", $1, $3);      }
   | Exp Greater Exp   { $$ = createOpNode("Greater", $1, $3);   }
   | Exp LessEq Exp    { $$ = createOpNode("LessEq", $1, $3);    }
   | Exp GreaterEq Exp { $$ = createOpNode("GreaterEq", $1, $3); }
   | Exp Plus Exp      { $$ = createOpNode("Plus", $1, $3);      }
   | Exp Minus Exp     { $$ = createOpNode("Minus", $1, $3);     }
   | Exp Mult Exp      { $$ = createOpNode("Mult", $1, $3);      }
   | Exp Div Exp       { $$ = createOpNode("Div", $1, $3);       }
   | Exp Mod Exp       { $$ = createOpNode("Mod", $1, $3);       }
   | UnaryExp          { $$ = $1; }
   ;

UnaryExp: PrimaryExp { $$ = $1; }
        | Identifier LeftParent FuncRParams RightParent { $$ = ASTNode_create("FuncCall", NULL); ASTNode_add_attr_str($$, "name", $1); $$->children = $3; }
        | UnaryOp UnaryExp { $$ = ASTNode_create($1, NULL); ASTNode_add_child($$, $2); }
        ; 

PrimaryExp: LVal { $$ = ASTNode_create("Fetch", NULL); ASTNode_add_child($$, $1); }
          | Number { $$ = ASTNode_create("Number", NULL); ASTNode_add_attr_int($$, "value", $1);}
          | LeftParent Exp RightParent {print_tokens(@$.last_line, @$.last_column); printf("<PrimaryExp>\n");}
          ;

UnaryOp: Plus {$$ = "UnPlus"; }
       | Minus { $$ = "UnMinus"; }
       | Not { $$ = "Not"; }
       ;

FuncRParams: /* empty */ { $$ = NULL; }
           | FuncRParamList { $$ = $1; }
           ;

FuncRParamList: ExpWrapper { $$ = NULL; ASTNode* param = ASTNode_create("Param", NULL); ASTNode_add_child(param, $1); $$ = addASTList($$, param); }
              | FuncRParamList Comma ExpWrapper { $$ = $1; ASTNode* param = ASTNode_create("Param", NULL); ASTNode_add_child(param, $3); addASTList($$, param); }
              ;

Number: IntegerConst { $$ = $1;};

Cond: Exp {print_tokens(@$.last_line, @$.last_column); printf("<Cond>\n");}
    ;

ConstExp: Exp {print_tokens(@$.last_line, @$.last_column); printf("<ConstExp>\n");}
        ;
%%

void yyerror(struct ASTNode **cur, const char *s) {
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
  struct ASTNode *root = NULL;
  int result = yyparse(&root);
  if (result == 0) {
    printf("====AST Info====\n");
    ASTNode_print(root);
  }
}