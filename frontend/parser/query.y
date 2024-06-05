%{
#include "sysY.h"
#include "ast.h"
int count = 0;

AttrOption* AttrOption_create_str(const char* name, const char* value) {
  assert(name != NULL);

  AttrOption* ret = (AttrOption*)calloc(1, sizeof(AttrOption));
  ret->name = strdup(name);
  ret->value.str = strdup(value);
  return ret;
}

AttrOption* AttrOption_create_num(const char* name, double value) {
  assert(name != NULL);

  AttrOption* ret = (AttrOption*)calloc(1, sizeof(AttrOption));
  ret->name = strdup(name);
  ret->value.num = value;
  return ret;
}

AttrOption* AttrOption_push_with_logic(AttrOption* list, AttrOption* lastOption, bool logicAnd) {
  assert(lastOption != NULL);

  lastOption->logicAnd = logicAnd;
  lastOption->next = NULL;
  lastOption->prev = NULL;
  DL_APPEND(list, lastOption);
  return list;
}

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
QueryResult *searchParent(QueryResult* cur, const SearchParam* param);
QueryResult *searchAncestor(QueryResult* cur, const SearchParam* param);
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
%%

Query: %empty { /* printf("Start Traveling...\n"); */ }
     | Query Selector { /* printf("Finish %d\n", ++count); */ }

SelectorPrefix: %empty { $$ = ""; }
              | Prefix { $$ = $1; /* printf("Selector Prefix: %s\n", $1); */ }

Selector: SelectorPrefix NodeName AttrSelector { freeList(result);
                                  // printf("Search for child %s\n", $2);
                                  SearchParam param; 
                                  param.id = $2; param.options = $3.options; param.index = $3.index;
                                  if (strcmp($1, "parent::") == 0) execSearch(last, result, searchParent, &param);
                                  else if (strcmp($1, "ancestor::") == 0) execSearch(last, result, searchAncestor, &param);
                                  else execSearch(last, result, searchChildName, &param);
                                  copyList(*result, *last);
                                  }
        | Slash    { copyList(*last, *result);
                     /* printf("Return current\n"); ref current node do nothing just return result. */ }
        | DoubleSlash NodeName AttrSelector { /* find id with 'NodeName' in descendents */
                                              freeList(result);
                                              // printf("Search for descendent %s\n", $2);
                                              SearchParam param; 
                                              param.id = $2; param.options = $3.options; param.index = -1;
                                              if ($3.index != -1) { qqerror(scanner, result, last, "WARN: Index is not allowed in descendent search."); }
                                              execSearch(last, result, searchDescendentName, &param);
                                              copyList(*result, *last);
                                              }

AttrSelector: %empty { $$.id = NULL; $$.options = NULL; $$.index = -1; }
            /* | LeftBracket AttrOptions RightBracket LeftBracket Number RightBracket { $$.options = $2; $$.index = $5; } */
            | LeftBracket AttrOptions RightBracket { $$.options = $2; $$.index = -1; }
            | LeftBracket Number RightBracket { $$.id = NULL; $$.options = NULL; $$.index = $2; }

AttrName: NodeName

AttrOptions: AttrOption { $$ = AttrOption_push_with_logic(NULL, $1, false); }
           | AttrOptions Comma AttrOption { $$ = AttrOption_push_with_logic($1, $3, true); }
           | AttrOptions Or AttrOption { $$ = AttrOption_push_with_logic($1, $3, false); }

AttrOption: At AttrName Equal String { $$ = AttrOption_create_str($2, $4); }
          | At AttrName Equal Number { $$ = AttrOption_create_num($2, $4); }
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

bool AttrOption_test(AttrOption* option, ASTNode* node) {
  assert(node != NULL);

  if (option == NULL) return true;

  bool ret = false;
  AttrOption* cur = NULL;
  DL_FOREACH(option, cur) {
    bool result = false;
    if (cur->type == AttrOptionTypeString) {
      result = ASTNode_attr_eq_str(node, cur->name, cur->value.str);
    } else {
      result = ASTNode_attr_eq_int(node, cur->name, cur->value.num)
              || ASTNode_attr_eq_float(node, cur->name, (float) cur->value.num); 
    }
    if (cur->logicAnd) {
      ret = ret && result;
    } else {
      ret = ret || result;
    }
  }
  return ret;
}

QueryResult *searchChildName(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *child = NULL;
  int childIndex = 0;
  bool isAny = strcmp(param->id, "*") == 0;
  DL_FOREACH(cur->node->children, child) {
    if (isAny || ASTNode_id_is(child, param->id)) {
      QueryResult *record = QueryResult_create(child);
      if ((param->index == -1 || param->index == childIndex)
          && AttrOption_test(param->options, child)) { 
        DL_APPEND(ret, record);
      }
    }
    ++childIndex;
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
      if (AttrOption_test(param->options, child)) DL_APPEND(ret, record);
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

QueryResult *searchParent(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *parent = cur->node->parent;
  if (parent && (strcmp(param->id, "*") == 0 || ASTNode_id_is(parent, param->id))) {
    QueryResult *record = QueryResult_create(parent);
    if (AttrOption_test(param->options, parent)) DL_APPEND(ret, record);
  }
  return ret;
}

QueryResult *searchAncestor(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *parent = cur->node->parent;
  while (parent) {
    if (strcmp(param->id, "*") == 0 || ASTNode_id_is(parent, param->id)) {
      QueryResult *record = QueryResult_create(parent);
      if (AttrOption_test(param->options, parent)) DL_APPEND(ret, record);
    }
    parent = parent->parent;
  }
  return ret;
}