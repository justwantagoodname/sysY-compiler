%{

#include "sysY.h"
#include "token.h"
#include "sym.h"
#include "sysY.tab.h"

#define YY_USER_ACTION \
    do { \
        yylloc.first_line = yylloc.last_line; \
        yylloc.first_column = yylloc.last_column; \
        for(int i = 0; yytext[i] != '\0'; i++) { \
            if(yytext[i] == '\n') { \
                yylloc.last_line++; \
                yylloc.last_column = 0; \
            } \
            else { \
                yylloc.last_column++; \
            } \
        }\
    } while (0);

extern void yyerror(struct ASTNode **cur, const char *s);
%}

IntConst [1-9][0-9]*|0
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
"main"          {push_token("MAINTK", yytext);         return Main;    }
"const"         {push_token("CONSTTK", yytext);        return Const;   }
"int"           {push_token("INTTK", yytext);          return Int;     }
"break"         {push_token("BREAKTK", yytext);        return Break;   }
"continue"      {push_token("CONTINUETK", yytext);     return Continue;}
"if"            {push_token("IFTK", yytext);           return If;      }
"else"          {push_token("ELSETK", yytext);         return Else;    }
"!"             {push_token("NOT", yytext);            return Not;     }
"&&"            {push_token("AND", yytext);            return And;     }
"||"            {push_token("OR", yytext);             return Or;      }
"while"         {push_token("WHILETK", yytext);        return While;   }
    /* "getint"        {push_token("GETINTTK", yytext);       return GetInt;  } */
    /* "printf"        {push_token("PRINTFTK", yytext);       return Printf;  } */
"return"        {push_token("RETURNTK", yytext);       return Return;  }
"+"             {push_token("PLUS", yytext);           return Plus;    }
"-"             {push_token("MINU", yytext);           return Minus;   }
"void"          {push_token("VOIDTK", yytext);         return Void;    }
"*"             {push_token("MULT", yytext);           return Mult;    }
"/"             {push_token("DIV", yytext);            return Div;     }
"%"             {push_token("MOD", yytext);            return Mod;     }
"<"             {push_token("LSS", yytext);            return Less;    }
"<="            {push_token("LEQ", yytext);            return LessEq;  }
">"             {push_token("GRE", yytext);            return Greater; }
">="            {push_token("GEQ", yytext);            return GreaterEq;}
"=="            {push_token("EQL", yytext);            return Equal;   }
"!="            {push_token("NEQ", yytext);            return NotEq;   }
"="             {push_token("ASSIGN", yytext);         return Assign;  }
";"             {push_token("SEMICN", yytext);         return SemiCon; }
","             {push_token("COMMA", yytext);          return Comma;   }
"("             {push_token("LPARENT", yytext);        return LeftParent;}
")"             {push_token("RPARENT", yytext);        return RightParent;}
"["             {push_token("LBRACK", yytext);         return LeftBrack;}
"]"             {push_token("RBRACK", yytext);         return RightBrack;}
"{"             {push_token("LBRACE", yytext);         return LeftBrace;}
"}"             {push_token("RBRACE", yytext);         return RightBrace;}

    /* constants and identifiers */
{FormatString}  {push_token("STRCON", yytext);         yylval.strValue = strdup(yytext);    return StringConst;}
{IntConst}      {push_token("INTCON", yytext);         yylval.intValue = atoi(yytext);      return IntegerConst;}
{Identifier}    {push_token("IDENFR", yytext);         yylval.tokenValue = strdup(yytext);  return Identifier;}

    /* Whitespaces and error */
{Whitespaces}   {}

.               {printf("Unknown character %s\n", yytext); yyerror(NULL, "Unknown character");}
%%

int yywrap() {
    return 1;
}
