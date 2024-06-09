#include "action.h"

AttrOption* AttrOption_create_has(const char* name) {
  assert(name != NULL);

  AttrOption* ret = (AttrOption*)calloc(1, sizeof(AttrOption));
  ret->name = strdup(name);
  ret->type = AttrOptionTypeExists;
  return ret;
}

AttrOption* AttrOption_create_str(const char* name, const char* value) {
  assert(name != NULL);

  AttrOption* ret = (AttrOption*)calloc(1, sizeof(AttrOption));
  ret->name = strdup(name);
  ret->value.str = strdup(value);
  ret->type = AttrOptionTypeString;
  return ret;
}

AttrOption* AttrOption_create_num(const char* name, double value) {
  assert(name != NULL);

  AttrOption* ret = (AttrOption*)calloc(1, sizeof(AttrOption));
  ret->name = strdup(name);
  ret->value.num = value;
  ret->type = AttrOptionTypeNumber;
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

bool AttrOption_test(const AttrOption* option, const ASTNode* node) {
  assert(node != NULL);

  if (option == NULL) return true;

  bool ret = false;
  AttrOption* cur = NULL;
  DL_FOREACH((AttrOption *) option, cur) {
    bool result = false;
    if (cur->type == AttrOptionTypeExists) {
      result = ASTNode_has_attr(node, cur->name);
    } else if (cur->type == AttrOptionTypeString) {
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

QueryResult *searchCurrentName(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  if (AttrOption_test(param->options, cur->node)) {
    QueryResult *record = QueryResult_create(cur->node);
    DL_APPEND(ret, record);
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
    if ((isAny || ASTNode_id_is(child, param->id)) && AttrOption_test(param->options, child)) {
      if ((param->index == -1 || param->index == childIndex)) { 
        QueryResult *record = QueryResult_create(child);
        DL_APPEND(ret, record);
        if (param->index == childIndex) break; // never break on -1
      }
      ++childIndex;
    }
  }
  return ret;
}

QueryResult *searchDescendentNameRecursive(const ASTNode* cur, const SearchParam* param, int *queryCount) {
  assert(cur != NULL && param != NULL);

  QueryResult* ret = NULL;
  ASTNode *child = NULL;
  bool isAny = strcmp(param->id, "*") == 0;
  DL_FOREACH(cur->children, child) {
    if ((isAny || ASTNode_id_is(child, param->id)) && AttrOption_test(param->options, child)) {
      (*queryCount)++;
      if (param->index == *queryCount - 1 || param->index == -1) {
        QueryResult *record = QueryResult_create(child);
        DL_APPEND(ret, record);
        if (param->index == *queryCount) break; // never break on -1
      }
    }
    /* if (*queryCount < param->index + 1 || param->index == -1) { */
      QueryResult *sub = searchDescendentNameRecursive(child, param, queryCount);
      if (sub) DL_CONCAT(ret, sub);
    /* } */
  }
  return ret;
}

QueryResult *searchDescendentName(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);
  int queryCount = 0; // Current Nodes Size
  return searchDescendentNameRecursive(cur->node, param, &queryCount);
}

QueryResult *searchParent(QueryResult* cur, const SearchParam* param) {
  assert(cur != NULL && param != NULL);
  
  QueryResult* ret = NULL;
  ASTNode *parent = cur->node->parent;
  if (parent 
      && (strcmp(param->id, "..") == 0 
          || strcmp(param->id, "*") == 0 
          || ASTNode_id_is(parent, param->id))) {
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