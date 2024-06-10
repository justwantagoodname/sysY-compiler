/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    Main = 258,
    Const = 259,
    Int = 260,
    Void = 261,
    While = 262,
    Break = 263,
    If = 264,
    Else = 265,
    Continue = 266,
    Return = 267,
    Not = 268,
    And = 269,
    Or = 270,
    Plus = 271,
    Minus = 272,
    Mult = 273,
    Div = 274,
    Mod = 275,
    Less = 276,
    Greater = 277,
    GreaterEq = 278,
    LessEq = 279,
    Equal = 280,
    NotEq = 281,
    Assign = 282,
    SemiCon = 283,
    Comma = 284,
    LeftParent = 285,
    RightParent = 286,
    LeftBrack = 287,
    RightBrack = 288,
    LeftBrace = 289,
    RightBrace = 290,
    IntegerConst = 291,
    StringConst = 292,
    Identifier = 293
  };
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

#line 105 "frontend/parser/sysY.tab.h"

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
