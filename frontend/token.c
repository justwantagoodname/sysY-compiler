/* Deprecated */
#include "token.h"

void push_token(const char *name, const char* literal) {
#if 0
    struct TokenInfo *token = (struct TokenInfo *)malloc(sizeof(struct TokenInfo));
    token->name = name;
    token->literal = strdup(literal);
    token->line = yylloc.last_line;
    token->col = yylloc.last_column;
    DL_APPEND(head, token);
#endif
}

void print_tokens(int line, int col) {
#if 0
    struct TokenInfo *cur, *tmp;
    // printf("===Output===\n");
    // printf("%d %d ", line, col);
    // return;
    DL_FOREACH_SAFE(head, cur, tmp) {
        
        if (cur->line > line || (cur->line == line && cur->col > col)) break;
        
        // printf("%d %d %d %d\n", line, col, cur->line, cur->col);
        // printf("%s %s\tline: %d col: %d\n", cur->name, cur->literal, cur->line, cur->col);

        printf("%s %s\n", cur->name, cur->literal);

        DL_DELETE(head, cur);
        free(cur->literal);
        free(cur);
    }
    // printf("%d %d ", line, col);
    // printf("===END===\n");
#endif
}