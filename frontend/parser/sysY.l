%{

#include "sysY.h"
#include "sysY.tab.hh"

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

DecimalFloatConst [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
HexFloatConst [-+]?(0x|0X)[0-9a-zA-Z]*\.?[0-9a-zA-Z]+([pP][-+]?[0-9]+)

DecConst [1-9][0-9]*
OctConst 0[0-7]*
HexConst (0x|0X)[0-9a-fA-F]+

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
"main"          { return Main;    }
"const"         { return Const;   }
"int"           { return Int;     }
"float"         { return Float;   }
"break"         { return Break;       }
"continue"      { return Continue;    }
"if"            { return If;          }
"else"          { return Else;        }
"!"             { return Not;         }
"&&"            { return And;         }
"||"            { return Or;          }
"while"         { return While;       }
    /* "getint"        {push_token("GETINTTK", yytext);       return GetInt;  } */
    /* "printf"        {push_token("PRINTFTK", yytext);       return Printf;  } */
"return"        { return Return;  }
"+"             { return Plus;    }
"-"             { return Minus;   }
"void"          { return Void;    }
"*"             { return Mult;    }
"/"             { return Div;     }
"%"             { return Mod;     }
"<"             { return Less;    }
"<="            { return LessEq;  }
">"             { return Greater; }
">="            { return GreaterEq;}
"=="            { return Equal;   }
"!="            { return NotEq;   }
"="             { return Assign;  }
";"             { return SemiCon; }
","             { return Comma;   }
"("             { return LeftParent;}
")"             { return RightParent;}
"["             { return LeftBrack;}
"]"             { return RightBrack;}
"{"             { return LeftBrace;}
"}"             { return RightBrace;}

    /* constants and identifiers */
{FormatString}  { yylval.strValue = strdup(yytext);    return StringConst;  }


    /* 解析十进制常量 */
{DecConst}      { yylval.intValue = std::stoi(yytext, 0, 10);      return IntegerConst; }

    /* 为了尊重原 sysy 文档实际上 单个零会变成八进制常量，不过这无所谓因为 0 表示是一样的 */
{OctConst}          { yylval.intValue = std::stoi(yytext, 0, 8);       return IntegerConst; }

{HexConst}          { yylval.intValue = std::stoi(yytext, 0, 16);      return IntegerConst; }

    /* 这个从神必网址抄过来的 https://www.regular-expressions.info/floatingpoint.html 😇 */
{DecimalFloatConst} { char* end; yylval.floatValue = strtof(yytext, &end); return FloatConst; }

    /* 这是我自己写的不知道对不对，参考 https://en.cppreference.com/w/c/string/byte/strtof 🤓 */
{HexFloatConst} { char* end; yylval.floatValue = strtof(yytext, &end); return FloatConst; }

{Identifier}    { yylval.tokenValue = strdup(yytext);  return Identifier;}

    /* Whitespaces and error */
{Whitespaces}   {}

.               { yyerror(NULL, "Unknown character");}
%%

int yywrap() {
    return 1;
}
