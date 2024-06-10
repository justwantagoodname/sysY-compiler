/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_FRONTEND_PARSER_SYSY_TAB_H_INCLUDED
# define YY_YY_FRONTEND_PARSER_SYSY_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "frontend/parser/sysY.y"

  extern int yylex();

#line 48 "frontend/parser/sysY.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    Main = 258,                    /* Main  */
    Const = 259,                   /* Const  */
    Int = 260,                     /* Int  */
    Void = 261,                    /* Void  */
    While = 262,                   /* While  */
    Break = 263,                   /* Break  */
    If = 264,                      /* If  */
    Else = 265,                    /* Else  */
    Continue = 266,                /* Continue  */
    Return = 267,                  /* Return  */
    Not = 268,                     /* Not  */
    And = 269,                     /* And  */
    Or = 270,                      /* Or  */
    Plus = 271,                    /* Plus  */
    Minus = 272,                   /* Minus  */
    Mult = 273,                    /* Mult  */
    Div = 274,                     /* Div  */
    Mod = 275,                     /* Mod  */
    Less = 276,                    /* Less  */
    Greater = 277,                 /* Greater  */
    GreaterEq = 278,               /* GreaterEq  */
    LessEq = 279,                  /* LessEq  */
    Equal = 280,                   /* Equal  */
    NotEq = 281,                   /* NotEq  */
    Assign = 282,                  /* Assign  */
    SemiCon = 283,                 /* SemiCon  */
    Comma = 284,                   /* Comma  */
    LeftParent = 285,              /* LeftParent  */
    RightParent = 286,             /* RightParent  */
    LeftBrack = 287,               /* LeftBrack  */
    RightBrack = 288,              /* RightBrack  */
    LeftBrace = 289,               /* LeftBrace  */
    RightBrace = 290,              /* RightBrace  */
    IntegerConst = 291,            /* IntegerConst  */
    StringConst = 292,             /* StringConst  */
    Identifier = 293               /* Identifier  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 23 "frontend/parser/sysY.y"

  int intValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;

#line 110 "frontend/parser/sysY.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (struct ASTNode **root);

#endif /* !YY_YY_FRONTEND_PARSER_SYSY_TAB_H_INCLUDED  */
