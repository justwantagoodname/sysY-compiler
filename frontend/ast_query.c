#include "sysY.h"
#include "ast.h"
#include "query.tab.h"

extern void qqlex_init(yyscan_t *scanner);
extern void qq_scan_string(const char *str, yyscan_t scanner);
extern void qqlex_destroy(yyscan_t scanner);

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

ASTNode *ASTNode_querySelectorOne(ASTNode *node, const char* selector) {
    QueryResult *result = ASTNode_querySelector(node, selector);
    if (result == NULL) return NULL;
    ASTNode *nodeResult = result->node;
    free(result);
    return nodeResult;
}

