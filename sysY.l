%{

#include "y.tab.h"

extern char *yytext, *filename;
extern int yylval;
void yyerror(const char*);

%}

IntConst [0-9]+
Identifier [a-zA-Z_][a-zA-Z0-9_]*
FormatString \".*\"
COMMENT "//".*\n

%x MULTI_COMMENT

%%
"/\*"           BEGIN(MULTI_COMMENT);
<MULTI_COMMENT>"*/"    BEGIN(INITIAL);
<MULTI_COMMENT>[ \f\n\r\t\v]      {}
<MULTI_COMMENT>.   {}


{COMMENT}       {}
"main"          {printf("MAINTK %s\n", yytext);         }
"const"         {printf("CONSTTK %s\n", yytext);        }
"int"           {printf("INTTK %s\n", yytext);          return Int;}
"break"         {printf("BREAKTK %s\n", yytext);        }
"continue"      {printf("CONTINUETK %s\n", yytext);     }
"if"            {printf("IFTK %s\n", yytext);           }
"else"          {printf("ELSETK %s\n", yytext);         }
"!"             {printf("NOT %s\n", yytext);            }
"&&"            {printf("AND %s\n", yytext);            }
"||"            {printf("OR %s\n", yytext);             }
"while"         {printf("WHILETK %s\n", yytext);        }
"getint"        {printf("GETINTTK %s\n", yytext);       }
"printf"        {printf("PRINTFTK %s\n", yytext);       }
"return"        {printf("RETURNTK %s\n", yytext);       }
"+"             {printf("PLUS %s\n", yytext);           }
"-"             {printf("MINU %s\n", yytext);           }
"void"          {printf("VOIDTK %s\n", yytext);         }
"*"             {printf("MULT %s\n", yytext);           }
"/"             {printf("DIV %s\n", yytext);            }
"%"             {printf("MOD %s\n", yytext);            }
"<"             {printf("LSS %s\n", yytext);            }
"<="            {printf("LEQ %s\n", yytext);            }
">"             {printf("GRE %s\n", yytext);            }
">="            {printf("GEQ %s\n", yytext);            }
"=="            {printf("EQL %s\n", yytext);            }
"!="            {printf("NEQ %s\n", yytext);            }
"="             {printf("ASSIGN %s\n", yytext);         return Eq;}
";"             {printf("SEMICN %s\n", yytext);         }
","             {printf("COMMA %s\n", yytext);          }
"("             {printf("LPARENT %s\n", yytext);        }
")"             {printf("RPARENT %s\n", yytext);        }
"["             {printf("LBRACK %s\n", yytext);         }
"]"             {printf("RBRACK %s\n", yytext);         }
"{"             {printf("LBRACE %s\n", yytext);         }
"}"             {printf("RBRACE %s\n", yytext);         }
{FormatString}  {printf("STRCON %s\n", yytext);         }
{IntConst}      {printf("INTCON %s\n", yytext);         yylval = atoi(yytext); return IntConst;}
{Identifier}    {printf("IDENFR %s\n", yytext);         return Indent;}
[ \f\n\r\t\v]   {}
%%

int yywrap() {
    return 1;
}



#ifdef CC_LAB_LEXICAL
int main(int argc, char *argv[]) {
    if (argc == 2) {
        filename = argv[1];
    }

    yyin = fopen(filename, "r");
    yyout = fopen("output.txt", "w");

#if 0
    char c;
    while ((c = fgetc(yyin)) != EOF) {
        if (c == '\n') continue;
        printf("%c", c);
    }
    printf("\n");
    rewind(yyin);
#endif

    yylex();
    return 0;
}

#endif
