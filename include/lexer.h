#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>


#define UNUSED(x) (void)(x)
#define UNREACHABLE(...) \
        do { \
            printf("%s:%d: UNREACHABLE: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
            exit(1); \
        } while(0)

typedef enum {
    TOKEN_END = 0,
    TOKEN_OPEN_CURLY,
    TOKEN_CLOSE_CURLY,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_NULL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SYMBOL, // should not be used outside of lexer_next
    TOKEN_INVALID
} Token_Type;

typedef struct {
    Token_Type type;
    const char *text;
    size_t text_len;
} Token;

typedef struct {
    char *content;
    size_t content_len;
    size_t cursor;
    size_t line;
    size_t bol;
} Lexer;

Lexer lexer_init(char *content, size_t content_len);
Token lexer_next(Lexer *l);
char *lexer_pretty(Token t);

#endif