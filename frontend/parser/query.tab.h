/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_QQ_FRONTEND_PARSER_QUERY_TAB_H_INCLUDED
# define YY_QQ_FRONTEND_PARSER_QUERY_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef QQDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define QQDEBUG 1
#  else
#   define QQDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define QQDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined QQDEBUG */
#if QQDEBUG
extern int qqdebug;
#endif
/* "%code requires" blocks.  */
#line 15 "frontend/parser/query.y"

  typedef void* yyscan_t;

#line 61 "frontend/parser/query.tab.h"

/* Token kinds.  */
#ifndef QQTOKENTYPE
# define QQTOKENTYPE
  enum qqtokentype
  {
    QQEMPTY = -2,
    QQEOF = 0,                     /* "end of file"  */
    QQerror = 256,                 /* error  */
    QQUNDEF = 257,                 /* "invalid token"  */
    NodeName = 258,                /* NodeName  */
    String = 259,                  /* String  */
    Prefix = 260,                  /* Prefix  */
    Number = 261,                  /* Number  */
    Slash = 262,                   /* Slash  */
    DoubleSlash = 263,             /* DoubleSlash  */
    LeftBracket = 264,             /* LeftBracket  */
    RightBracket = 265,            /* RightBracket  */
    At = 266,                      /* At  */
    Equal = 267,                   /* Equal  */
    Comma = 268,                   /* Comma  */
    Or = 269                       /* Or  */
  };
  typedef enum qqtokentype qqtoken_kind_t;
#endif

/* Value type.  */
#if ! defined QQSTYPE && ! defined QQSTYPE_IS_DECLARED
union QQSTYPE
{
#line 24 "frontend/parser/query.y"

  const char* string;
  int number;
  struct AttrOption* attrOption;
  struct SearchParam searchParam;
  int index;

#line 100 "frontend/parser/query.tab.h"

};
typedef union QQSTYPE QQSTYPE;
# define QQSTYPE_IS_TRIVIAL 1
# define QQSTYPE_IS_DECLARED 1
#endif




int qqparse (yyscan_t scanner, QueryResult **result, QueryResult **last);


#endif /* !YY_QQ_FRONTEND_PARSER_QUERY_TAB_H_INCLUDED  */
