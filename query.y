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
}

%token <string> NodeName

%%

Query: NodeName { printf("NodeName: %s\n", $1); }

%%

void qqerror(yyscan_t scanner, const char* msg) {
  fprintf(stderr, "%s\n", msg);
}