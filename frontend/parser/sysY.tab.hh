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

#ifndef YY_YY_FRONTEND_PARSER_SYSY_TAB_HH_INCLUDED
# define YY_YY_FRONTEND_PARSER_SYSY_TAB_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 12 "frontend/parser/sysY.y"

  extern int yylex();

#line 48 "frontend/parser/sysY.tab.hh"

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
    Float = 262,                   /* Float  */
    While = 263,                   /* While  */
    Break = 264,                   /* Break  */
    If = 265,                      /* If  */
    Else = 266,                    /* Else  */
    Continue = 267,                /* Continue  */
    Return = 268,                  /* Return  */
    Not = 269,                     /* Not  */
    And = 270,                     /* And  */
    Or = 271,                      /* Or  */
    Plus = 272,                    /* Plus  */
    Minus = 273,                   /* Minus  */
    Mult = 274,                    /* Mult  */
    Div = 275,                     /* Div  */
    Mod = 276,                     /* Mod  */
    Less = 277,                    /* Less  */
    Greater = 278,                 /* Greater  */
    GreaterEq = 279,               /* GreaterEq  */
    LessEq = 280,                  /* LessEq  */
    Equal = 281,                   /* Equal  */
    NotEq = 282,                   /* NotEq  */
    Assign = 283,                  /* Assign  */
    SemiCon = 284,                 /* SemiCon  */
    Comma = 285,                   /* Comma  */
    LeftParent = 286,              /* LeftParent  */
    RightParent = 287,             /* RightParent  */
    LeftBrack = 288,               /* LeftBrack  */
    RightBrack = 289,              /* RightBrack  */
    LeftBrace = 290,               /* LeftBrace  */
    RightBrace = 291,              /* RightBrace  */
    IntegerConst = 292,            /* IntegerConst  */
    FloatConst = 293,              /* FloatConst  */
    StringConst = 294,             /* StringConst  */
    Identifier = 295               /* Identifier  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 25 "frontend/parser/sysY.y"

  int intValue;
  float floatValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;

#line 113 "frontend/parser/sysY.tab.hh"

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

#endif /* !YY_YY_FRONTEND_PARSER_SYSY_TAB_HH_INCLUDED  */
