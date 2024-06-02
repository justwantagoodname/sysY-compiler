%{
#include <stdio.h>
#include "query.tab.h"

#define YYSTYPE QQSTYPE
%}

%option prefix="qq"
%option noyywrap
%option reentrant
%option bison-bridge 
%option outfile="query.yy.c"

NodeName [a-zA-Z_][a-zA-Z0-9_]*

%%
{NodeName} { (*yylval).string = strdup(yytext); printf("Lex: %s\n", yytext); return NodeName; }
"/" { printf("%s", yytext); return 2; }
"//" { printf("%s", yytext); return 3; }
%%