%{
#include "query.tab.h"
#include <string.h>

#define YY_DECL int yylex(YYSTYPE * yylval, yyscan_t scanner)
%}

%option noyywrap reentrant bison-bridge

%%

"//"                    { return DOUBLE_SLASH; }
"/"                     { return SLASH; }
"["                     { return LBRACKET; }
"]"                     { return RBRACKET; }
"="                     { return EQUALS; }
[a-zA-Z_][a-zA-Z0-9_]*  {
                          yylval->str = strdup(yytext);
                          return IDENTIFIER;
                        }

[ \t\n\r]+              /* skip whitespace */
.                       { return yytext[0]; }

%%

int yywrap() {
    return 1;
}