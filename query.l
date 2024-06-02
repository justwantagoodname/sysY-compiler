%{
#include "sysY.h"
#include "query.tab.h"

#define YYSTYPE QQSTYPE

char* stringBody(const char* text);
%}

%option prefix="qq"
%option noyywrap
%option reentrant
%option bison-bridge 
%option outfile="query.yy.c"

NodeName [a-zA-Z_][a-zA-Z0-9_]*
String \".*\"

%%
[[:digit:]]+ { (*yylval).number = atoi(yytext); return Number; }
{NodeName}|"*" { (*yylval).string = strdup(yytext); return NodeName; }
{String}    {(*yylval).string = stringBody(yytext); return String;}
"/"     { return Slash; }
"//"    { return DoubleSlash; }
"["     { return LeftBracket; }
"]"     { return RightBracket; }
"@"     { return At; }
","     { return Comma; }
"="     { return Equal; }
%%

char* stringBody(const char* text) {
    size_t len = strlen(text);
    // 分配新的字符串，长度为原字符串长度减去2（去掉双引号），并添加终止符
    char* result = (char*)malloc((len - 1) * sizeof(char));
    strncpy(result, text + 1, len - 2);
    result[len - 2] = '\0';
    return result;
}