#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "query.tab.h"

QueryResult *ASTNode_querySelector(ASTNode *node, const char* selector) {
    assert(selector != NULL);
    printf("Querying selector: %s\n", selector);
    yyscan_t scanner;
    qqlex_init(&scanner);
    qq_scan_string(selector, scanner);
    int token;
    qqparse(scanner);
    // do {
    //    token = qqlex(scanner);
    //    printf("Token: %d\n", token);
    //} while (token != 0);
    qqlex_destroy(scanner);

    return NULL;
}
