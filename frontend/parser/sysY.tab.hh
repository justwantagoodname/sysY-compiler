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
#line 10 "frontend/parser/sysY.y"

  extern int yylex();

#line 48 "frontend/parser/sysY.tab.hh"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    Main = 258,
    Const = 259,
    Int = 260,
    Void = 261,
    Float = 262,
    While = 263,
    Break = 264,
    If = 265,
    Else = 266,
    Continue = 267,
    Return = 268,
    Not = 269,
    And = 270,
    Or = 271,
    Plus = 272,
    Minus = 273,
    Mult = 274,
    Div = 275,
    Mod = 276,
    Less = 277,
    Greater = 278,
    GreaterEq = 279,
    LessEq = 280,
    Equal = 281,
    NotEq = 282,
    Assign = 283,
    SemiCon = 284,
    Comma = 285,
    LeftParent = 286,
    RightParent = 287,
    LeftBrack = 288,
    RightBrack = 289,
    LeftBrace = 290,
    RightBrace = 291,
    IntegerConst = 292,
    FloatConst = 293,
    StringConst = 294,
    Identifier = 295
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 23 "frontend/parser/sysY.y"

  int intValue;
  float floatValue;
  const char* strValue;
  const char* tokenValue;
  struct ASTNode *astNode;

#line 108 "frontend/parser/sysY.tab.hh"

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
