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
  struct FunctionSymbol *funcSymbol;
  ValueType valueType;
  FuncType funcType;
}

%token
    Main Const 

    Int Void 
    
    While Break 
    
    If Else Continue 
    
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

%type <astNode> CompUnit MainFuncDef Block BlockItem Stmt LVal 
                ConstExp Exp UnaryExp PrimaryExp ExpWrapper ArrayDecl InitValue InitValList ConstInitValue ConstInitValList
                IfStmt Cond FuncRParams FuncRParamList

%type <strValue> UnaryOp

%type <valueSymbol> VarDecl VarDefList VarDef 
                    ConstDecl ConstDefList ConstDef 
                    Decl GlobalDecl 
                    FuncFParams FuncFParamList FuncFParam

%type <funcSymbol> GlobalFuncDef FuncDef

%type <valueType> PrimaryType 

%type <funcType> FuncType

%left Or
%left And
%left Equal NotEq
%left Greater GreaterEq Less LessEq
%left Plus Minus
%left Div Mult Mod

%parse-param {struct ASTNode **root}

%%
CompUnit: GlobalDecl GlobalFuncDef MainFuncDef { ASTNode* scope = ASTNode_create("Scope");
                                                 ASTNode_add_attr_str(scope, "name", "Global");
                                                 *root = $$ = ASTNode_create("CompUnit");
                                                 ASTNode_add_child($$, $3);
                                                }
        ;

MainFuncDef: Int Main LeftParent RightParent Block { $$ = ASTNode_create("Main"); 
                                                     ASTNode_add_child($$, $5); }
           ;

GlobalDecl: /* empty */ { $$ = NULL; }
          | GlobalDecl Decl { $$ = appendVSList($1, $2); }
          ;

GlobalFuncDef: /* empty */ { $$ = NULL; }
             | GlobalFuncDef FuncDef { $$ = addFSArray($1, $2);  }
             ;

Decl: VarDecl
    | ConstDecl
    ;

ConstDecl: Const PrimaryType ConstDefList SemiCon { modifyVSType($3, $2, true); $$ = $3; }
         ;

ConstDefList: ConstDef { $$ = addVSArray(NULL, $1); }
            | ConstDefList Comma ConstDef { $$ = addVSArray($1, $3);}
            ;

ConstDef: Identifier Assign ConstInitValue { $$ = ValueSymbol_create($1, ANY, $3); }
        | Identifier ArrayDecl Assign ConstInitValue { $$ = ValueSymbol_create_array($1, ANY_ARRAY, $2, $4); }
        ;

ConstInitValue: ConstExp { $$ = ASTNode_create("ConstInitValue"); ASTNode_add_child($$, $1); }
              | LeftBrace ConstInitValList RightBrace { $$ = $2; }
              ;

ConstInitValList: /* empty */ { $$ = ASTNode_create("ConstInitValue"); }
                | ConstInitValue { $$ = ASTNode_create("ConstInitValue"); ASTNode_add_child($$, $1);}
                | ConstInitValList Comma ConstInitValue { $$ = $1; ASTNode_add_child($$, $3); }
                ;

VarDecl: PrimaryType VarDefList SemiCon { modifyVSType($2, $1, false); $$ = $2; }
       ;

VarDefList: VarDef { $$ = addVSArray(NULL, $1); }
          | VarDefList Comma VarDef { $$ = addVSArray($1, $3);}
          ;

VarDef: Identifier { $$ = ValueSymbol_create($1, ANY, NULL); }
      | Identifier Assign InitValue { $$ = ValueSymbol_create($1, ANY, $3); }
      | Identifier ArrayDecl { $$ = ValueSymbol_create_array($1, ANY_ARRAY, $2, NULL); }
      | Identifier ArrayDecl Assign InitValue { $$ = ValueSymbol_create_array($1, ANY_ARRAY, $2, $4); }
      ;

ArrayDecl: LeftBrack ConstExp RightBrack { $$ = ASTNode_create("ArrayDecl"); ASTNode_add_child($$, $2); } 
         | ArrayDecl LeftBrack ConstExp RightBrack  { $$ = $1; ASTNode_add_child($$, $3); }
         ;

InitValue: ExpWrapper { $$ = ASTNode_create("InitValue"); ASTNode_add_child($$, $1); }
         | LeftBrace InitValList RightBrace { $$ = $2; }
         ;

InitValList: /* empty */ { $$ = ASTNode_create("InitValue"); }
           | InitValue { $$ = ASTNode_create("InitValue"); ASTNode_add_child($$, $1);}
           | InitValList Comma InitValue { $$ = $1; ASTNode_add_child($$, $3); }
           ;

FuncType: Void { $$ = VOID_FUNC; }
        | Int  { $$ = INT_FUNC; }
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block { $$ = FunctionSymbol_create($2, $1, $4, $6); } 
       ;

FuncFParams: /* empty */    { $$ = NULL; }
           | FuncFParamList { $$ = $1; }
           ;

FuncFParamList: FuncFParam { $$ = addVSArray(NULL, $1); }
              | FuncFParamList Comma FuncFParam { $$ = addVSArray($1, $3);}
              ;

FuncFParam: PrimaryType Identifier { $$ = ValueSymbol_create($2, $1, NULL); }
          | PrimaryType Identifier LeftBrack RightBrack { ASTNode* len = ASTNode_create("Unknown"); 
                                                          ASTNode* decl = ASTNode_create("ArrayDecl");
                                                          ASTNode_add_child(decl, len);
                                                          $$ = ValueSymbol_create_array($2, $1 + 1, decl, NULL);
                                                        } 
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl { ASTNode* len = ASTNode_create("Unknown"); 
                                                                    ASTNode_lpush_child($5, len);
                                                                    $$ = ValueSymbol_create_array($2, $1 + 1, $5, NULL);
                                                                  } 
          ;

Block: LeftBrace BlockItem RightBrace { $$ = $2; };

BlockItem:  /* empty */ { $$ = ASTNode_create("Block"); /* TODO: Add Scope */ }
         | BlockItem Decl { /* TODO: collect decls to scope */ }
         | BlockItem Stmt { ASTNode_add_child($1, $2); $$ = $1; }
         ;

PrimaryType: Int { $$ = INT; }
           ;

Stmt: LVal Assign ExpWrapper SemiCon { $$ = ASTNode_create("Assign"); ASTNode* dest = ASTNode_create("Dest"); ASTNode_add_child(dest, $1); ASTNode_add_child($$, dest); ASTNode_add_child($$, $3);}
    | SemiCon { $$ = ASTNode_create("NOP"); }
    | ExpWrapper SemiCon { $$ = $1; }
    | Block { $$ = $1; /* TODO: Add new scope */ }
    | IfStmt { $$ = $1; }
    | While LeftParent Cond RightParent Stmt { $$ = createWhileNode($3, $5);}
    | Return ExpWrapper SemiCon { $$ = ASTNode_create("Return"); ASTNode_add_child($$, $2);}
    | Return SemiCon {$$ = ASTNode_create("Return");}
    | Break SemiCon { $$ = ASTNode_create("Break"); }
    | Continue SemiCon { $$ = ASTNode_create("Continue"); }
    ;

/* attach else to cloest if stmt */
/* see https://www.gnu.org/software/bison/manual/html_node/Merging-GLR-Parses.html */
IfStmt: If LeftParent Cond RightParent Stmt { $$ = createIfNode($3, $5, NULL); } %dprec 2
      | If LeftParent Cond RightParent Stmt Else Stmt { $$ = createIfNode($3, $5, $7);} %dprec 1
      ;

LVal: Identifier { $$ = ASTNode_create("Address"); ASTNode_add_attr_str($$, "base", $1); }
    | Identifier ArrayLocatorList { $$ = ASTNode_create("Fetch"); ASTNode_add_attr_str($$, "base", $1); /* TODO: calc base */ }
    ;

ArrayLocator: LeftBrack Exp RightBrack
            ;

ArrayLocatorList: ArrayLocator
                | ArrayLocator ArrayLocatorList
                ;

ExpWrapper: Exp { $$ = ASTNode_create("Exp"); ASTNode_add_child($$, $1); }
          ;

Exp: Exp Or Exp        { $$ = createOpNode("Or", $1, $3);        }
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
        | Identifier LeftParent FuncRParams RightParent { $$ = ASTNode_create("Call"); 
                                                          ASTNode_add_attr_str($$, "name", $1); 
                                                          ASTNode_move_children($3, $$);
                                                          ASTNode_free($3); }
        | UnaryOp UnaryExp { $$ = ASTNode_create($1); ASTNode_add_child($$, $2); }
        ; 

PrimaryExp: LVal { $$ = ASTNode_create("Fetch"); ASTNode_add_child($$, $1); }
          | Number { $$ = ASTNode_create("Number"); ASTNode_add_attr_int($$, "value", $1);}
          | LeftParent Exp RightParent { $$ = $2; }
          ;

UnaryOp: Plus   { $$ = "UnPlus";  }
       | Minus  { $$ = "UnMinus"; }
       | Not    { $$ = "Not";     }
       ;

FuncRParams: /* empty */    { $$ = NULL;}
           | FuncRParamList { $$ = $1;  }
           ;

FuncRParamList: ExpWrapper  { 
                              $$ = ASTNode_create("ParamArray");
                              ASTNode* param = ASTNode_create("Param"); 
                              ASTNode_add_child($$, param);
                              ASTNode_add_child(param, $1); 
                            }
              | StringConst { 
                              $$ = ASTNode_create("ParamArray");
                              ASTNode* param = ASTNode_create("Param"); 
                              ASTNode_add_child($$, param);
                              char* strContent = trimQuoteStr($1); 
                              ASTNode_add_attr_str(param, "value", strContent);
                              free(strContent);
                            }
              | FuncRParamList Comma ExpWrapper { $$ = $1; ASTNode* param = ASTNode_create("Param"); ASTNode_add_child(param, $3); ASTNode_add_child($$, param); }
              ;

Number: IntegerConst
      ;

Cond: ExpWrapper
    ;

ConstExp: ExpWrapper 
        ;
%%

void yyerror(struct ASTNode **cur, const char *s) {
  fprintf (stderr, "%s\n", s);
}