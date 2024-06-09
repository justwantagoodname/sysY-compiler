#include "sysY.h"
#include "ast.h"
#ifndef QUERY_H
#define QUERY_H

class Element;

class Query {
private:
    QueryResult* result;

public:
    Query(QueryResult* res);
    Query(ASTNode* node);
    Query(const Element& e);
    operator QueryResult* ();
    operator Element();
    operator bool();
    Query& operator+= (const Query&& q);
    Query operator/ (const char* select) const;
    Query operator[] (const char* key) const;
    Element operator[] (int);
    Query operator() (const char* select) const;

    struct iter {
        QueryResult* it;
        iter(QueryResult* q);
        iter& operator ++();
        bool operator!=(iter& other);
        Element operator*();
    };

    iter begin();
    iter end();
};

#endif // QUERY_H
