#include <cstdarg>
#include "sysY.h"
#include "ast.h"
#include "query.tab.hh"

int vasprintf(char ** __restrict__ ret,
                      const char * __restrict__ format,
                      va_list ap) {
    int len;
    /* Get Length */
    len = _vsnprintf(NULL,0,format,ap);
    if (len < 0) return -1;
    /* +1 for \0 terminator. */
    *ret = static_cast<char *>(malloc(len + 1));
    /* Check malloc fail*/
    if (!*ret) return -1;
    /* Write String */
    _vsnprintf(*ret,len+1,format,ap);
    /* Terminate explicitly */
    (*ret)[len] = '\0';
    return len;
}

extern void qqlex_init(yyscan_t *scanner);
extern void qq_scan_string(const char *str, yyscan_t scanner);
extern void qqlex_destroy(yyscan_t scanner);

QueryResult *QueryResult_create(const ASTNode *node) {
    QueryResult *result = (QueryResult*) calloc(1, sizeof(QueryResult));
    result->node = (ASTNode*) node; // 虽然这里是const的，但是但是我们大多期望修改查询到的节点所以专为可修改，由调用者确保不会发生悬空的引用
    return result;
}

QueryResult *ASTNode_querySelector(const ASTNode *node, const char* selector) {
    assert(selector != NULL);
    // printf("Querying selector: %s\n", selector);
    QueryResult* result = NULL, *last = NULL;

    DL_APPEND(last, QueryResult_create(node));

    yyscan_t scanner;
    qqlex_init(&scanner);
    qq_scan_string(selector, scanner);
    int execResult = qqparse(scanner, &result, &last);
    qqlex_destroy(scanner);

    return execResult ? NULL : result;
}

ASTNode *ASTNode_querySelectorOne(const ASTNode *node, const char* selector) {
    QueryResult *result = ASTNode_querySelector(node, selector);
    if (result == NULL) return NULL;
    ASTNode *nodeResult = (ASTNode *) result->node;
    free(result);
    return nodeResult;
}

QueryResult *ASTNode_querySelectorf(const ASTNode *node, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *selector = NULL;
    int ret = vasprintf(&selector, fmt, args);
    va_end(args);
    QueryResult *result = ASTNode_querySelector(node, selector);
    free(selector);
    return result;
}

ASTNode *ASTNode_querySelectorfOne(const ASTNode *node, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *selector = NULL;
    int ret = vasprintf(&selector, fmt, args);
    va_end(args);
    ASTNode *result = ASTNode_querySelectorOne(node, selector);
    free(selector);
    return result;
}
