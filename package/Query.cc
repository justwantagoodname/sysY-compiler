#include "Query.h"
#include "Element.h"

Query::Query(QueryResult* res) : result(res) {}

Query::Query(ASTNode* node) {
    result = QueryResult_create(node);
}

Query::Query(const Element& e)
{
    result = QueryResult_create(e);
}

Query::operator QueryResult* () {
    return result;
}

Query::operator Element()
{
    if (result && result->node)
        return Element(result->node);
    else 
        return Element((ASTNode*)(NULL));
}

Query::operator bool()
{
    return result;
}

Query& Query::operator+= (const Query&& q) {
    DL_CONCAT(result, q.result);
    return *this;
}

Query Query::operator/ (const char* select) const {
    char* sel = (char*)malloc(strlen(select) + 5);
    sel[0] = '/', sel[1] = '\0';
    sel = strcat(sel, select);
    QueryResult* cur = NULL;
    QueryResult* ans = NULL;
    DL_FOREACH(result, cur) {
        QueryResult* res = ASTNode_querySelector(cur->node, sel);
        if (res) {
            DL_CONCAT(ans, res);
        }
    }
    free(sel);
    return ans;
}

Query Query::operator[] (const char* key) const {
    QueryResult* cur = NULL;
    QueryResult* ans = NULL;
    DL_FOREACH(result, cur) {
        ASTAttribute* res = ASTNode_get_attr_or_null(cur->node, key);
        if (res) {
            DL_APPEND(ans, cur);
        }
    }
    return ans;
}

Element Query::operator[](int index)
{
    QueryResult* que = result;
    int i = 0;
    while (i < index) {
        if (que == nullptr)
            return Element();
        que = que->next;
        ++i;
    }
    return Element(que->node);
}

Query Query::operator()(const char* select) const
{
    QueryResult* cur = NULL;
    QueryResult* ans = NULL;
    DL_FOREACH(result, cur) {
        QueryResult* res = ASTNode_querySelector(cur->node, select);
        if (res) {
            DL_CONCAT(ans, res);
        }
    }
    return ans;
}

Query::iter::iter(QueryResult* q) : it(q) {}

Query::iter& Query::iter::operator ++() {
    it = it->next;
    return *this;
}

bool Query::iter::operator!=(iter& other) {
    return other.it != it;
}

Element Query::iter::operator*() {
    return Element(it->node);
}

Query::iter Query::begin() {
    return iter(result);
}

Query::iter Query::end() {
    return nullptr;
}