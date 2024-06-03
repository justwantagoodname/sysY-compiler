%{
#include "sysY.h"
#include "ast.h"
int count = 0;

struct SearchParam {
  const char* id;
};
typedef struct SearchParam SearchParam;

typedef QueryResult* (*SearchFunc)(QueryResult*, const SearchParam*);

#define moveList(src, dest) do { if ((src) != (dest)) freeList(&(dest)); (dest) = (src); (src) = NULL; } while(0);
#define copyList(src, dest) do { \
                                  if ((src) != (dest)) { \
                                    freeList(&(dest));   \
                                    QueryResult *cur = NULL; \
                                    DL_FOREACH((src), (cur)) { QueryResult *record = QueryResult_create(cur->node); DL_APPEND((dest), record); } \
                                  }} while (0);
void freeList(QueryResult **list);
void execSearch(QueryResult **list, QueryResult **result, SearchFunc func, const SearchParam *param);
QueryResult* searchChildName(QueryResult* cur, const SearchParam* param);
QueryResult *searchDescendentName(QueryResult* cur, const SearchParam* param);
%}
%define api.pure full
%define api.prefix {qq}

%param {yyscan_t scanner}

%parse-param {struct QueryResult **result}
%parse-param {struct QueryResult **last}

%code requires {
  typedef void* yyscan_t;
}

%code {
  int qqlex(QQSTYPE* yylval, yyscan_t scanner);
  void qqerror(yyscan_t scanner, QueryResult **result, QueryResult **last, const char* msg);
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

Query: %empty { printf("Start Traveling...\n"); }
     | Query Selector { printf("Finish %d\n", ++count); }

Selector: NodeName AttrSelector { freeList(result);
                                  printf("Search for child %s\n", $1);
                                  SearchParam param; param.id = $1;
                                  execSearch(last, result, searchChildName, &param);
                                  copyList(*result, *last);
                                  }
        | Slash    { copyList(*last, *result);
                     printf("Return current\n"); /* ref current node do nothing just return result. */ }
        | DoubleSlash NodeName AttrSelector { /* find id with 'NodeName' in descendents */
                                              freeList(result);
                                              printf("Search for descendent %s\n", $2);
                                              SearchParam param; param.id = $2;
                                              execSearch(last, result, searchDescendentName, &param);
                                              copyList(*result, *last);
                                              }

AttrSelector: %empty
            | LeftBracket Number RightBracket {  }
            | LeftBracket AttrOptions RightBracket {}

AttrName: NodeName

AttrOptions: At AttrName Equal String
           | AttrOptions Comma At AttrName Equal String

%%

void qqerror(yyscan_t scanner, QueryResult **result, QueryResult **last, const char* msg) {
  fprintf(stderr, "%s\n", msg);
}

void freeList(QueryResult **list) {
  QueryResult *cur = NULL, *tmp = NULL;
  DL_FOREACH_SAFE(*list, cur, tmp) {
    DL_DELETE(*list, cur);
    free(cur);
  }
  *list = NULL;
}

void execSearch(QueryResult **list, QueryResult **result, SearchFunc func, const SearchParam *param) {
  assert(func != NULL);
  assert(*result == NULL);
  
  if (*list == NULL) return;

  QueryResult *cur = NULL;
  DL_FOREACH(*list, cur) {
    QueryResult *curResult = func(cur, param);
    if (curResult) DL_CONCAT(*result, curResult);
  }
}

QueryResult *searchChildName(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *child = NULL;
  bool isAny = strcmp(param->id, "*") == 0;
  DL_FOREACH(cur->node->children, child) {
    if (isAny || ASTNode_id_is(child, param->id)) {
      QueryResult *record = QueryResult_create(child);
      DL_APPEND(ret, record);
    }
  }
  return ret;
}

QueryResult *searchDescendentNameRecursive(ASTNode* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *child = NULL;
  bool isAny = strcmp(param->id, "*") == 0;
  DL_FOREACH(cur->children, child) {
    if (isAny || ASTNode_id_is(child, param->id)) {
      QueryResult *record = QueryResult_create(child);
      DL_APPEND(ret, record);
    }
    QueryResult *sub = searchDescendentNameRecursive(child, param);
    if (sub) DL_CONCAT(ret, sub);
  }
  return ret;
}

QueryResult *searchDescendentName(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  return searchDescendentNameRecursive(cur->node, param);
}