%{
#include "sysY.h"
%}
%define api.pure full
%define api.prefix {qq}

%param {yyscan_t scanner}

%code requires {
  typedef void* yyscan_t;
}

%code {
  int qqlex(QQSTYPE* yylval, yyscan_t scanner);
  void qqerror(yyscan_t scanner, const char* msg);
}

%union {
  char* string;
  int number;
}

%token <string> NodeName String
%token <number> Number
%token Slash DoubleSlash LeftBracket RightBracket At Equal Comma
%type <string> AttrName
%%

Query: %empty
     | Query Selector

Selector: NodeName { /* find child with id 'NodeName' or '*' for all */ }
        | Slash    { /* ref current Node do nothing. */ }
        | DoubleSlash NodeName { /* find id with 'NodeName' in descendents */ }

AttrSelector: %empty
            | LeftBracket Number RightBracket {  }
            | LeftBracket AttrOptions RightBracket {}

AttrName: NodeName

AttrOptions: At AttrName Equal String
           | AttrOptions Comma At AttrName Equal String

%%

void qqerror(yyscan_t scanner, const char* msg) {
  fprintf(stderr, "%s\n", msg);
}