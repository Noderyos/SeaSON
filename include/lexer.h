#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#define SEASON_LEX_UNREACH(...) \
        do { \
            printf("%s:%d: UNREACHABLE: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
            exit(1); \
        } while(0)

enum season_token_type {
    SEASON_TOK_END = 0,
    SEASON_TOK_OPEN_CURLY,
    SEASON_TOK_CLOSE_CURLY,
    SEASON_TOK_OPEN_BRACKET,
    SEASON_TOK_CLOSE_BRACKET,
    SEASON_TOK_COMMA,
    SEASON_TOK_COLON,
    SEASON_TOK_STRING,
    SEASON_TOK_NUMBER,
    SEASON_TOK_NULL,
    SEASON_TOK_TRUE,
    SEASON_TOK_FALSE,
    SEASON_TOK_SYMBOL, // should not be used outside of season_lex_next
    SEASON_TOK_INVALID
};

struct season_token {
    enum season_token_type type;
    size_t line;
    size_t column;
    const char *text;
    size_t text_len;
};

struct season_lexer {
    char *content;
    size_t content_len;
    size_t cursor;
    size_t line;
    size_t bol;
};

struct season_lexer season_lex_init(char *content, size_t content_len);
struct season_token season_lex_next(struct season_lexer *l);

#endif