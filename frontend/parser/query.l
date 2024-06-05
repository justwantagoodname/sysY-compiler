%{
#include "sysY.h"
#include "ast.h"
#include "query.tab.h"

#define YYSTYPE QQSTYPE

char* stringBody(const char* text);
%}

%option prefix="qq"
%option noyywrap
%option reentrant
%option bison-bridge 

NodeName [a-zA-Z_][a-zA-Z0-9_]*|\.|\.\.|\*
String \'[^\']*\'
Prefix "ancestor::"|"parent::"

%%
[[:digit:]]+    { (*yylval).number = atoi(yytext); return Number; }
{NodeName}      { (*yylval).string = strdup(yytext); return NodeName; }
{String}        {(*yylval).string = stringBody(yytext); return String;}
{Prefix}    { (*yylval).string = strdup(yytext); return Prefix; }
"/"     { return Slash; }
"//"    { return DoubleSlash; }
"["     { return LeftBracket; }
"]"     { return RightBracket; }
"@"     { return At; }
","     { return Comma; }
"="     { return Equal; }
"|"     { return Or; }
.       { printf("Unrecognized character: '%s'.Maybe Error in our lexer?\n", yytext); return -1;}
%%

char* stringBody(const char* text) {
    size_t len = strlen(text);
    char* result = (char*)malloc((len - 1) * sizeof(char));
    strncpy(result, text + 1, len - 2);
    result[len - 2] = '\0';
    return result;
}