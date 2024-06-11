%{

#include "sysY.h"
#include "token.h"
#include "ast.h"
#include "action.h"

void yyerror(struct ASTNode **cur, const char *s);
%}

%code requires {
  extern int yylex();
}

%locations

%glr-parser

/* %expect 2 */

%start CompUnit

%union {
  int intValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;
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
                IfStmt Cond FuncRParams FuncRParamList GlobalFuncDef FuncDef
                FuncFParams FuncFParamList FuncFParam
                VarDecl VarDefList VarDef 
                ConstDecl ConstDefList ConstDef 
                Decl GlobalDecl 
                ArrayLocator ArrayLocatorList

%type <strValue> UnaryOp

%type <strValue> PrimaryType FuncType

%left Or
%left And
%left Equal NotEq
%left Greater GreaterEq Less LessEq
%left Plus Minus
%left Div Mult Mod

%parse-param {struct ASTNode **root}

%%
CompUnit: GlobalDecl GlobalFuncDef MainFuncDef { ASTNode* scope = ASTNode_create_attr("Scope", 1, "name", "Global");
                                                 *root = $$ = ASTNode_create("CompUnit");
                                                 ASTNode_add_child($$, scope);
                                                 ASTNode_add_nchild(scope, 3, $1, $2, $3);
                                                }
        ;

MainFuncDef: Int Main LeftParent RightParent Block { $$ = ASTNode_create("Main"); 
                                                     ASTNode_add_child($$, $5); }
           ;

GlobalDecl: /* empty */ { $$ = ASTNode_create("Decl");}
          | GlobalDecl Decl { $$ = $1; ASTNode_move_children($2, $$); ASTNode_free($2); }
          ;

GlobalFuncDef: /* empty */ { $$ = ASTNode_create("FunctionDef"); }
             | GlobalFuncDef FuncDef { $$ = $1; ASTNode_add_child($$, $2);  }
             ;

Decl: VarDecl
    | ConstDecl
    ;

ConstDecl: Const PrimaryType ConstDefList SemiCon { $$ = $3; modifyValueType($$, $2); }
         ;

ConstDefList: ConstDef { $$ = ASTNode_create("ConstantTemp"); ASTNode_add_child($$, $1); }
            | ConstDefList Comma ConstDef { $$ = $1; ASTNode_add_child($$, $3); }
            ;

ConstDef: Identifier Assign ConstInitValue {
                                              $$ = ASTNode_create_attr("Const", 1, "name", $1); 
                                              ASTNode_add_child($$, $3); 
                                           }
        | Identifier ArrayDecl Assign ConstInitValue { 
                                                       $$ = ASTNode_create_attr("Const", 2, "name", $1, "array", "true"); 
                                                       ASTNode* as = ASTNode_create("ArraySize");
                                                       ASTNode_move_children($2, as);
                                                       ASTNode_add_nchild($$, 2, as, $4);
                                                      }
        ;

ConstInitValue: ConstExp { $$ = ASTNode_create("ConstInitValue"); ASTNode_add_child($$, $1); }
              | LeftBrace ConstInitValList RightBrace { $$ = $2; }
              ;

ConstInitValList: /* empty */ { $$ = ASTNode_create("ConstInitValue"); }
                | ConstInitValue { $$ = ASTNode_create("ConstInitValue"); ASTNode_add_child($$, $1);}
                | ConstInitValList Comma ConstInitValue { $$ = $1; ASTNode_add_child($$, $3); }
                ;

VarDecl: PrimaryType VarDefList SemiCon { modifyValueType($2, $1); $$ = $2; }
       ;

VarDefList: VarDef { $$ = ASTNode_create("VarTemp"); ASTNode_add_child($$, $1);}
          | VarDefList Comma VarDef { $$ = $1; ASTNode_add_child($$, $3); }
          ;

VarDef: Identifier { $$ = ASTNode_create_attr("Var", 1, "name", $1); }
      | Identifier Assign InitValue { $$ = ASTNode_create_attr("Var", 1, "name", $1); ASTNode_add_child($$, $3); }
      | Identifier ArrayDecl { $$ = ASTNode_create_attr("Var", 2, "name", $1, "array", "true"); 
                               ASTNode_move_children($2, $$);
                              }
      | Identifier ArrayDecl Assign InitValue { $$ = ASTNode_create_attr("Var", 2, "name", $1, "array", "true"); 
                                                ASTNode_move_children($2, $$);
                                                ASTNode_add_child($$, $4);
                                              }
      ;

ArrayDecl: LeftBrack ConstExp RightBrack  { 
                                            $$ = ASTNode_create("ArrayDeclTemp");
                                            ASTNode* dimension = ASTNode_create("Dimension"); 
                                            ASTNode_add_child(dimension, $2);
                                            ASTNode_add_child($$, dimension); 
                                          } 
         | ArrayDecl LeftBrack ConstExp RightBrack  { 
                                                      $$ = $1; 
                                                      ASTNode* dimension = ASTNode_create("Dimension"); 
                                                      ASTNode_add_child(dimension, $3);
                                                      ASTNode_add_child($$, dimension); 
                                                    }
         ;

InitValue: ExpWrapper { $$ = ASTNode_create("InitValue"); ASTNode_add_child($$, $1); }
         | LeftBrace InitValList RightBrace { $$ = $2; }
         ;

InitValList: /* empty */ { $$ = ASTNode_create("InitValue"); }
           | InitValue { $$ = ASTNode_create("InitValue"); ASTNode_add_child($$, $1);}
           | InitValList Comma InitValue { $$ = $1; ASTNode_add_child($$, $3); }
           ;

FuncType: Void { $$ = "Void"; }
        | Int  { $$ = "Int"; }
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block { 
            $$ = ASTNode_create_attr("Function", 2, "return", $1, "name", $2);
            ASTNode_add_nchild($$, 2, $4, $6);
            ASTNode* decl = ASTNode_querySelectorOne($6, "/Decl");
            ASTNode_copy_children($4, decl);
          } 
       ;

FuncFParams: /* empty */    { $$ = ASTNode_create("Params"); }
           | FuncFParamList { $$ = ASTNode_create("Params"); ASTNode_move_children($1, $$); ASTNode_free($1);}
           ;

FuncFParamList: FuncFParam { $$ = ASTNode_create("ParamList"); ASTNode_add_child($$, $1); }
              | FuncFParamList Comma FuncFParam { $$ = $1; ASTNode_add_child($$, $3); }
              ;

FuncFParam: PrimaryType Identifier  { $$ = ASTNode_create_attr("ParamDecl", 2, "type", $1, "name", $2); }
          | PrimaryType Identifier LeftBrack RightBrack { 
                                                          $$ = ASTNode_create_attr("ParamDecl", 3, "type", $1, "name", $2, "array", "true"); 
                                                          ASTNode* dimension = ASTNode_create_attr("Dimension", 1, "size", "Unknown");
                                                          ASTNode_add_child($$, dimension);
                                                        } 
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl { 
                                                                    $$ = ASTNode_create_attr("ParamDecl", 3, "type", $1, "name", $2, "array", "true"); 
                                                                    ASTNode* dimension = ASTNode_create_attr("Dimension", 1, "size", "Unknown");
                                                                    ASTNode_add_child($$, dimension);
                                                                    ASTNode_move_children($5, $$);
                                                                  } 
          ;

Block: LeftBrace BlockItem RightBrace { $$ = $2; };

BlockItem:  /* empty */ { $$ = ASTNode_create("Scope"); ASTNode_add_nchild($$, 2, ASTNode_create("Decl"), ASTNode_create("Block")); }
         | BlockItem Decl { $$ = collectDecl($1, $2); }
         | BlockItem Stmt { $$ = $1;
                            ASTNode* block = ASTNode_querySelectorOne($$, "/Block"); 
                            ASTNode_add_child(block, $2); 
                          }
         ;

PrimaryType: Int { $$ = "Int"; }
           ;

Stmt: LVal Assign ExpWrapper SemiCon { $$ = ASTNode_create("Assign"); ASTNode* dest = ASTNode_create("Dest"); ASTNode_add_child(dest, $1); ASTNode_add_child($$, dest); ASTNode_add_child($$, $3);}
    | SemiCon { $$ = ASTNode_create("NOP"); }
    | ExpWrapper SemiCon { $$ = $1; }
    | Block { $$ = $1; }
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

LVal: Identifier { $$ = ASTNode_create_attr("Address", 1, "base", $1); /*ASTNode_add_attr_str($$, "base", $1);*/ }
    | Identifier ArrayLocatorList { $$ = ASTNode_create_attr("Address", 1, "base", $1); /*ASTNode_add_attr_str($$, "base", $1); */ASTNode_add_child($$, $2); /* TODO: calc base */ }
    ;

ArrayLocator: LeftBrack Exp RightBrack { $$ = ASTNode_create("Dimension"); ASTNode_add_child($$, $2); }
            ;

ArrayLocatorList: ArrayLocator { $$ = ASTNode_create("Locator"); ASTNode_add_child($$, $1); }
                | ArrayLocatorList ArrayLocator { $$ = $1; ASTNode_add_child($$, $2); }
                ;

ExpWrapper: Exp { $$ = ASTNode_create("Exp"); ASTNode_add_child($$, $1); }
          ;

Exp: Exp Or Exp        { $$ = createOpNode("Or", $1, $3);        } %dprec 7
   | Exp And Exp       { $$ = createOpNode("And", $1, $3);       } %dprec 6
   | Exp Equal Exp     { $$ = createOpNode("Equal", $1, $3);     } %dprec 4
   | Exp NotEq Exp     { $$ = createOpNode("NotEq", $1, $3);     } %dprec 4
   | Exp Less Exp      { $$ = createOpNode("Less", $1, $3);      } %dprec 4
   | Exp Greater Exp   { $$ = createOpNode("Greater", $1, $3);   } %dprec 4
   | Exp LessEq Exp    { $$ = createOpNode("LessEq", $1, $3);    } %dprec 4
   | Exp GreaterEq Exp { $$ = createOpNode("GreaterEq", $1, $3); } %dprec 4
   | Exp Plus Exp      { $$ = createOpNode("Plus", $1, $3);      } %dprec 3
   | Exp Minus Exp     { $$ = createOpNode("Minus", $1, $3);     } %dprec 3
   | Exp Mult Exp      { $$ = createOpNode("Mult", $1, $3);      } %dprec 2
   | Exp Div Exp       { $$ = createOpNode("Div", $1, $3);       } %dprec 2
   | Exp Mod Exp       { $$ = createOpNode("Mod", $1, $3);       } %dprec 2
   | UnaryExp          { $$ = $1; } %dprec 1
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