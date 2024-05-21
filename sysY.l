%{

#if !defined(CC_LAB_LEXICAL)
#include "y.tab.h"


extern char *yytext;
extern int yylval;
void yyerror(const char*);

#endif

%}

IntConst [1-9][0-9]+|0
Identifier [a-zA-Z_][a-zA-Z0-9_]*
FormatString \".*\"
Comment "//".*\n
Whitespaces [ \f\n\r\t\v]

%x MULTI_COMMENT

%%
    /* comments */
"/\*"           BEGIN(MULTI_COMMENT);
<MULTI_COMMENT>"*/"    BEGIN(INITIAL);
<MULTI_COMMENT>[ \f\n\r\t\v]      {}
<MULTI_COMMENT>.   {}

{Comment}       {}

    /* keywords */
"main"          {printf("MAINTK %s\n", yytext);         return Main;    }
"const"         {printf("CONSTTK %s\n", yytext);        return Const;   }
"int"           {printf("INTTK %s\n", yytext);          return Int;     }
"break"         {printf("BREAKTK %s\n", yytext);        return Break;   }
"continue"      {printf("CONTINUETK %s\n", yytext);     return Continue;}
"if"            {printf("IFTK %s\n", yytext);           return If;      }
"else"          {printf("ELSETK %s\n", yytext);         return Else;    }
"!"             {printf("NOT %s\n", yytext);            return Not;     }
"&&"            {printf("AND %s\n", yytext);            return And;     }
"||"            {printf("OR %s\n", yytext);             return Or;      }
"while"         {printf("WHILETK %s\n", yytext);        return While;   }
"getint"        {printf("GETINTTK %s\n", yytext);       return GetInt;  }
"printf"        {printf("PRINTFTK %s\n", yytext);       return Printf;  }
"return"        {printf("RETURNTK %s\n", yytext);       return Return;  }
"+"             {printf("PLUS %s\n", yytext);           return Plus;    }
"-"             {printf("MINU %s\n", yytext);           return Minus;   }
"void"          {printf("VOIDTK %s\n", yytext);         return Void;    }
"*"             {printf("MULT %s\n", yytext);           return Mult;    }
"/"             {printf("DIV %s\n", yytext);            return Div;     }
"%"             {printf("MOD %s\n", yytext);            return Mod;     }
"<"             {printf("LSS %s\n", yytext);            return Less;    }
"<="            {printf("LEQ %s\n", yytext);            return LessEq;  }
">"             {printf("GRE %s\n", yytext);            return Greater; }
">="            {printf("GEQ %s\n", yytext);            return GreaterEq;}
"=="            {printf("EQL %s\n", yytext);            return Equal;   }
"!="            {printf("NEQ %s\n", yytext);            return NotEq;   }
"="             {printf("ASSIGN %s\n", yytext);         return Assign;  }
";"             {printf("SEMICN %s\n", yytext);         return SemiCon; }
","             {printf("COMMA %s\n", yytext);          return Comma;   }
"("             {printf("LPARENT %s\n", yytext);        return LeftParent;}
")"             {printf("RPARENT %s\n", yytext);        return RightParent;}
"["             {printf("LBRACK %s\n", yytext);         return LeftBrack;}
"]"             {printf("RBRACK %s\n", yytext);         return RightBrack;}
"{"             {printf("LBRACE %s\n", yytext);         return LeftBrace;}
"}"             {printf("RBRACE %s\n", yytext);         return RightBrace;}

    /* constants and identifiers */
{FormatString}  {printf("STRCON %s\n", yytext);         return StringConst;}
{IntConst}      {printf("INTCON %s\n", yytext);         yylval = atoi(yytext); return IntegerConst;}
{Identifier}    {printf("IDENFR %s\n", yytext);         return Identifier;}

    /* Whitespaces and error */
{Whitespaces}   {}

.               {printf("Unknown character %s\n", yytext); yyerror("Unknown character");}
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
