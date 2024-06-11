%{
#include "sysY.h"
#include "ast.h"
#include "action.h"

%}
%define api.pure full
%define api.prefix {qq}

%param {yyscan_t scanner}

%parse-param {QueryResult **result}
%parse-param {QueryResult **last}

%code requires {
  typedef void* yyscan_t;
}

%code {
  int qqlex(QQSTYPE* yylval, yyscan_t scanner);
  void qqerror(yyscan_t scanner, QueryResult **result, QueryResult **last, const char* msg);
}

%union {
  const char* string;
  int number;
  struct AttrOption* attrOption;
  struct SearchParam searchParam;
  int index;
}

%token <string> NodeName String Prefix
%token <number> Number
%token Slash DoubleSlash LeftBracket RightBracket At Equal Comma Or
%type <string> AttrName
%type <attrOption> AttrOption AttrOptions
%type <searchParam> AttrSelector
%type <string> SelectorPrefix

%left Or Comma
%%

Query: %empty { /* printf("Start Traveling...\n"); */ }
     | Query Selector { /* printf("Finish %d\n", ++count); */ }

SelectorPrefix: %empty { $$ = ""; }
              | Prefix { $$ = $1; /* printf("Selector Prefix: %s\n", $1); */ }

Selector: SelectorPrefix NodeName AttrSelector { freeList(result);
                                  // printf("Search for child %s\n", $2);
                                  SearchParam param; 
                                  param.id = $2; param.options = $3.options; param.index = $3.index;
                                  if (strcmp($1, "parent::") == 0 || strcmp($2, "..") == 0) execSearch(last, result, searchParent, &param);
                                  else if (strcmp($1, "ancestor::") == 0) execSearch(last, result, searchAncestor, &param);
                                  else if (strcmp($2, ".") == 0) execSearch(last, result, searchCurrentName, &param);
                                  else execSearch(last, result, searchChildName, &param);
                                  copyList(*result, *last);
                                  }
        | Slash    { copyList(*last, *result);
                     /* printf("Return current\n"); ref current node do nothing just return result. */ }
        | DoubleSlash NodeName AttrSelector { /* find id with 'NodeName' in descendents */
                                              freeList(result);
                                              // printf("Search for descendent %s\n", $2);
                                              SearchParam param; 
                                              param.id = $2; param.options = $3.options; param.index = $3.index;
                                              // if ($3.index != -1) { qqerror(scanner, result, last, "WARN: Index is not allowed in descendent search."); }
                                              execSearch(last, result, searchDescendentName, &param);
                                              copyList(*result, *last);
                                              }

AttrSelector: %empty { $$.id = NULL; $$.options = NULL; $$.index = -1; }
            | LeftBracket AttrOptions RightBracket LeftBracket Number RightBracket { $$.options = $2; $$.index = $5; }
            | LeftBracket AttrOptions RightBracket { $$.options = $2; $$.index = -1; }
            | LeftBracket Number RightBracket { $$.id = NULL; $$.options = NULL; $$.index = $2; }

AttrName: NodeName

AttrOptions: AttrOption { $$ = AttrOption_push_with_logic(NULL, $1, false); }
           | AttrOptions Comma AttrOption { $$ = AttrOption_push_with_logic($1, $3, true); }
           | AttrOptions Or AttrOption { $$ = AttrOption_push_with_logic($1, $3, false); }

AttrOption: At AttrName Equal String { $$ = AttrOption_create_str($2, $4); }
          | At AttrName { $$ = AttrOption_create_has($2); }
          | At AttrName Equal Number { $$ = AttrOption_create_num($2, $4); }
%%

void qqerror(yyscan_t scanner, QueryResult **result, QueryResult **last, const char* msg) {
  fprintf(stderr, "AQL Error Occurred check your selector syntax: %s\n", msg);
  assert(0); // AQL选择器错误，暂时强制终止程序
}

