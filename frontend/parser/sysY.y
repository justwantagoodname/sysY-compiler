%{

#include "sysY.h"
#include "ast.h"
#include "action.h"

void yyerror(struct ASTNode **cur, const char *s);

#define YYINITDEPTH 240000
%}

%code requires {
  extern int yylex();
}

%locations

%glr-parser

%expect 1
%expect-rr 2

%start CompUnit

%union {
  int intValue;
  float floatValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;
}

%token
    Main Const 

    Int Void Float
    
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

%token <floatValue> FloatConst

%token <strValue> StringConst Identifier

%type <astNode> CompUnit GlobalScope 
                Block BlockItem Stmt LVal 
                ConstExp Exp UnaryExp PrimaryExp ExpWrapper 
                InitValue InitValList 
                IfStmt Cond FuncRParams FuncRParamList FuncDef
                FuncFParams FuncFParamList FuncFParam
                VarDecl VarDefList VarDef 
                ConstDecl ConstDefList ConstDef 
                Decl ArrayDecl 
                ArrayLocator ArrayLocatorList 
                Number

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
/* CompUnit: GlobalDecl GlobalFuncDef MainFuncDef { 
                                                 *root = $$ = ASTNode_create("CompUnit");
                                                 ASTNode_add_child($$, scope);
                                                 ASTNode_add_nchild(scope, 3, $1, $2, $3);
                                                }
        ; */
CompUnit: GlobalScope { *root = $$ = ASTNode_create("CompUnit"); ASTNode_add_child($$, $1); }

GlobalScope: %empty { 
                      $$ = ASTNode_create_attr("Scope", 1, "name", "Global");
                      ASTNode *decl = ASTNode_create("Decl");
                      ASTNode *func = ASTNode_create("FunctionDef");
                      ASTNode_add_nchild($$, 2, decl, func);
                    }
           | GlobalScope Decl {
                              $$ = $1;
                              ASTNode* decl = ASTNode_querySelectorOne($$, "/Decl");
                              ASTNode_move_children($2, decl);
                              ASTNode_free($2);
                           }
           | GlobalScope FuncDef {
                                $$ = $1;
                                ASTNode* func = ASTNode_querySelectorOne($$, "/FunctionDef");
                                ASTNode_add_child(func, $2);
                              }

Decl: VarDecl
    | ConstDecl
    ;

ConstDecl: Const PrimaryType ConstDefList SemiCon { $$ = $3; modifyValueType($$, $2); }
         ;

ConstDefList: ConstDef { $$ = ASTNode_create("ConstantTemp"); ASTNode_add_child($$, $1); }
            | ConstDefList Comma ConstDef { $$ = $1; ASTNode_add_child($$, $3); }
            ;

ConstDef: Identifier Assign InitValue {
                                        $$ = ASTNode_create_attr("Const", 1, "name", $1);
                                        ASTNode_add_attr_int($$, "line", @1.first_line);
                                        ASTNode_add_attr_int($$, "column", @1.first_column);
                                        auto value = ASTNode_create("InitValue");
                                        ASTNode_add_child(value, $3);
                                        ASTNode_add_child($$, value);
                                      }
        | Identifier ArrayDecl Assign InitValue { 
                                                  $$ = ASTNode_create_attr("Const", 2, "name", $1, "array", "true"); 
                                                  ASTNode* as = ASTNode_create("ArraySize");
                                                  ASTNode_add_attr_int($$, "line", @1.first_line);
                                                  ASTNode_add_attr_int($$, "column", @1.first_column);
                                                  ASTNode_move_children($2, as);
                                                  ASTNode_add_nchild($$, 2, as, $4);
                                                }
        ;

VarDecl: PrimaryType VarDefList SemiCon { modifyValueType($2, $1); $$ = $2; }
       ;

VarDefList: VarDef { $$ = ASTNode_create("VarTemp"); ASTNode_add_child($$, $1);}
          | VarDefList Comma VarDef { $$ = $1; ASTNode_add_child($$, $3); }
          ;

VarDef: Identifier { $$ = ASTNode_create_attr("Var", 1, "name", $1); ASTNode_add_attr_int($$, "line", @1.first_line); ASTNode_add_attr_int($$, "column", @1.first_column); }
      | Identifier Assign InitValue { $$ = ASTNode_create_attr("Var", 1, "name", $1);                                               
                                      auto value = ASTNode_create("InitValue");
                                      ASTNode_add_attr_int($$, "line", @1.first_line);
                                      ASTNode_add_attr_int($$, "column", @1.first_column);
                                      ASTNode_add_child(value, $3);
                                      ASTNode_add_child($$, value);
                                    }
      | Identifier ArrayDecl { $$ = ASTNode_create_attr("Var", 2, "name", $1, "array", "true"); 
                               ASTNode_add_child($$, $2);
                               ASTNode_add_attr_int($$, "line", @1.first_line);
                               ASTNode_add_attr_int($$, "column", @1.first_column);
                              }
      | Identifier ArrayDecl Assign InitValue { $$ = ASTNode_create_attr("Var", 2, "name", $1, "array", "true"); 
                                                ASTNode_add_child($$, $2);
                                                ASTNode_add_child($$, $4);
                                                ASTNode_add_attr_int($$, "line", @1.first_line);
                                                ASTNode_add_attr_int($$, "column", @1.first_column);
                                              }
      ;

ArrayDecl: LeftBrack ConstExp RightBrack  { 
                                            $$ = ASTNode_create("ArraySize");
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

InitValue: ExpWrapper { $$ = $1; }
         | LeftBrace InitValList RightBrace { $$ = $2; }
         ;

InitValList: %empty { $$ = ASTNode_create("InitValue"); }
           | InitValue { $$ = ASTNode_create("InitValue"); ASTNode_add_child($$, $1);}
           | InitValList Comma InitValue { $$ = $1; ASTNode_add_child($$, $3); }
           ;

FuncType: Void { $$ = "Void"; }
        | Int  { $$ = "Int"; }
        | Float {$$ = "Float";}
        ;

FuncDef: FuncType Identifier LeftParent FuncFParams RightParent Block { 
            $$ = ASTNode_create_attr("Function", 2, "return", $1, "name", $2);
            ASTNode_add_nchild($$, 2, $4, $6);
            ASTNode_set_attr_str($6, "name", $2);
            ASTNode* decl = ASTNode_querySelectorOne($6, "/Decl");
            ASTNode_copy_children($4, decl);
          } 
       ;

FuncFParams: %empty    { $$ = ASTNode_create("Params"); }
           | FuncFParamList { $$ = ASTNode_create("Params"); ASTNode_move_children($1, $$); ASTNode_free($1);}
           ;

FuncFParamList: FuncFParam { $$ = ASTNode_create("ParamList"); ASTNode_add_child($$, $1); }
              | FuncFParamList Comma FuncFParam { $$ = $1; ASTNode_add_child($$, $3); }
              ;

FuncFParam: PrimaryType Identifier  { $$ = ASTNode_create_attr("ParamDecl", 2, "type", $1, "name", $2); 
                                      ASTNode_add_attr_int($$, "line", @1.first_line);
                                      ASTNode_add_attr_int($$, "column", @1.first_column);
                                    }
          | PrimaryType Identifier LeftBrack RightBrack { 
                                                          $$ = ASTNode_create_attr("ParamDecl", 3, "type", $1, "name", $2, "array", "true"); 
                                                          ASTNode* dimension = ASTNode_create_attr("Dimension", 1, "size", "Unknown");
                                                          ASTNode_add_attr_int($$, "line", @1.first_line);
                                                          ASTNode_add_attr_int($$, "column", @1.first_column);
                                                          ASTNode_add_child($$, dimension);
                                                        } 
          | PrimaryType Identifier LeftBrack RightBrack ArrayDecl { 
                                                                    $$ = ASTNode_create_attr("ParamDecl", 3, "type", $1, "name", $2, "array", "true"); 
                                                                    ASTNode* dimension = ASTNode_create_attr("Dimension", 1, "size", "Unknown");
                                                                    ASTNode_add_attr_int($$, "line", @1.first_line);
                                                                    ASTNode_add_attr_int($$, "column", @1.first_column);
                                                                    ASTNode_add_child($$, dimension);
                                                                    ASTNode_move_children($5, $$);
                                                                  } 
          ;

Block: LeftBrace BlockItem RightBrace { $$ = $2; };

BlockItem: %empty { $$ = ASTNode_create("Scope");
                          char* scopeName = getAnonymousName();
                          ASTNode_add_attr_str($$, "name", scopeName);
                          free(scopeName);
                          ASTNode_add_nchild($$, 2, ASTNode_create("Decl"), ASTNode_create("Block")); 

                        }
         | BlockItem Decl { $$ = collectDecl($1, $2); }
         | BlockItem Stmt { $$ = $1;
                            ASTNode* block = ASTNode_querySelectorOne($$, "/Block"); 
                            ASTNode_add_child(block, $2); 
                          }
         ;

PrimaryType: Int { $$ = "Int"; }
           | Float { $$ = "Float"; }
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

LVal: Identifier { $$ = ASTNode_create_attr("Address", 1, "base", $1); ASTNode_add_attr_int($$, "line", @1.first_line); ASTNode_add_attr_int($$, "column", @1.first_column); }
    | Identifier ArrayLocatorList { $$ = ASTNode_create_attr("Address", 1, "base", $1); ASTNode_add_child($$, $2); ASTNode_add_attr_int($$, "line", @1.first_line); ASTNode_add_attr_int($$, "column", @1.first_column);}
    ;

ArrayLocator: LeftBrack Exp RightBrack { $$ = ASTNode_create("Dimension"); ASTNode_add_child($$, $2); }
            ;

ArrayLocatorList: ArrayLocator { $$ = ASTNode_create("Locator"); ASTNode_add_child($$, $1); }
                | ArrayLocatorList ArrayLocator { $$ = $1; ASTNode_add_child($$, $2); }
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
                                                          ASTNode_add_attr_int($$, "line", @1.first_line); // 一些外部函数调用需要使用 __LINE__ 宏，这里模拟一下
                                                          ASTNode_add_attr_str($$, "name", $1); 
                                                          ASTNode_move_children($3, $$);
                                                          ASTNode_free($3); }
        | UnaryOp UnaryExp { $$ = ASTNode_create($1); ASTNode_add_child($$, $2); }
        ; 

PrimaryExp: LVal { $$ = ASTNode_create("Fetch"); ASTNode_add_child($$, $1); }
          | Number { $$ = $1; }
          | LeftParent Exp RightParent { $$ = $2; }
          ;

UnaryOp: Plus   { $$ = "UnPlus";  }
       | Minus  { $$ = "UnMinus"; }
       | Not    { $$ = "Not";     }
       ;

FuncRParams: %empty    { $$ = ASTNode_create("ParamArray"); }
           | FuncRParamList { $$ = $1;  }
           ;

FuncRParamList: Exp { 
                      $$ = ASTNode_create("ParamArray");
                      ASTNode* param = ASTNode_create("Param"); 
                      ASTNode_add_attr_str(param, "type", "Exp"); // 应该从上下文中推断是 float 或者 int
                      ASTNode_add_child($$, param);
                      ASTNode_add_child(param, $1); 
                    }
              | StringConst { 
                              $$ = ASTNode_create("ParamArray");
                              ASTNode* param = ASTNode_create("Param"); 
                              ASTNode_add_child($$, param);
                              char* strContent = trimQuoteStr($1); 
                              ASTNode_add_attr_str(param, "value", strContent);
                              ASTNode_add_attr_str(param, "type", "StringConst");
                              free(strContent);
                            }
              | FuncRParamList Comma Exp { 
                                            $$ = $1; 
                                            ASTNode* param = ASTNode_create("Param"); 
                                            ASTNode_add_attr_str(param, "type", "Exp"); // 应该从上下文中推断是 float 或者 int
                                            ASTNode_add_child(param, $3); 
                                            ASTNode_add_child($$, param); 
                                          }
              ;

Number: IntegerConst  { $$ = ASTNode_create_attr("Number", 1, "type", "Int"); ASTNode_add_attr_int($$, "value", $1);     }
      | FloatConst    { $$ = ASTNode_create_attr("Number", 1, "type", "Float"); ASTNode_add_attr_float($$, "value", $1);   }
      ;

Cond: ExpWrapper
    ;

ConstExp: ExpWrapper 
        ;
%%

void yyerror(struct ASTNode **cur, const char *s) {
  fprintf (stderr, "%s\n", s);
}