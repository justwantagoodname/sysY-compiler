#include "sysY.h"
#include "ast.h"
#include "query.tab.h"

QueryResult *QueryResult_create(ASTNode *node) {
    QueryResult *result = (QueryResult*) calloc(1, sizeof(QueryResult));
    result->node = node;
    return result;
}

QueryResult *ASTNode_querySelector(ASTNode *node, const char* selector) {
    assert(selector != NULL);
    printf("Querying selector: %s\n", selector);
    QueryResult* result = NULL, *last = NULL;

    DL_APPEND(last, QueryResult_create(node));

    yyscan_t scanner;
    qqlex_init(&scanner);
    qq_scan_string(selector, scanner);
    int execResult = qqparse(scanner, &result, &last);
    qqlex_destroy(scanner);

    return execResult ? NULL : result;
}
