#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

struct {
    char c;
    Token_Type type;
} literals[] = {
        {'{', TOKEN_OPEN_CURLY},
        {'}', TOKEN_CLOSE_CURLY},
        {'[', TOKEN_OPEN_BRACKET},
        {']', TOKEN_CLOSE_BRACKET},
        {',', TOKEN_COMMA},
        {':', TOKEN_COLON}
};

#define literals_count (sizeof(literals)/sizeof(literals[0]))

const char *keywords[] = {
        "case", "run", "trace"
};

size_t keywords_count = 2;


Lexer lexer_init(char *content, size_t content_len){
    Lexer l = {0};
    l.content = content;
    l.content_len = content_len;
    return l;
}

char lexer_chop_char(Lexer *l){
    assert(l->cursor < l->content_len);

    char x = l->content[l->cursor];
    l->cursor++;
    if(x == '\n'){
        l->line++;
        l->bol = l->cursor;
    }
    return x;
}

void lexer_trim_left(Lexer *l){
    while (l->cursor < l->content_len && isspace(l->content[l->cursor])){
        UNUSED(lexer_chop_char(l));
    }
}

int is_number_start(char c){
    return strchr("0123456789-", c) != NULL;
}

int is_number(char c){
    return strchr("0123456789-.eE", c) != NULL;
}

Token lexer_next(Lexer *l){
    lexer_trim_left(l);
    Token token = {
            .text = &l->content[l->cursor]
    };

    if (l->cursor >= l->content_len) return token;

    if (l->content[l->cursor] == '"') {
        token.type = TOKEN_STRING;
        l->cursor++;
        token.text++;
        while (l->cursor < l->content_len && l->content[l->cursor] != '"') {
            token.text_len++;
            l->cursor++;
        }
        l->cursor++;
        return token;
    }

    if (is_number_start(l->content[l->cursor])) {
        token.type = TOKEN_NUMBER;
        while (l->cursor < l->content_len && is_number(l->content[l->cursor])) {
            token.text_len++;
            l->cursor++;
        }
        return token;
    }

    if (islower(l->content[l->cursor])) {
        token.type = TOKEN_INVALID;
        while (l->cursor < l->content_len && islower(l->content[l->cursor])) {
            token.text_len++;
            l->cursor++;
        }
        if (token.text_len == 4 && strncmp(token.text, "null", 4) == 0)
            token.type = TOKEN_NULL;
        else if (token.text_len == 4 && strncmp(token.text, "true", 4) == 0)
            token.type = TOKEN_TRUE;
        else if (token.text_len == 5 && strncmp(token.text, "false", 5) == 0)
            token.type = TOKEN_FALSE;

        return token;
    }

    for (size_t i = 0; i < literals_count; ++i) {
        if(l->content[l->cursor] == literals[i].c){
            lexer_chop_char(l);
            token.type = literals[i].type;
            token.text_len = 1;
            return token;
        }
    }

    UNUSED(lexer_chop_char(l));
    token.type = TOKEN_INVALID;
    token.text_len = 1;
    return token;
}

char *lexer_pretty(Token t){
    switch (t.type) {
        case TOKEN_END: return "end";
        case TOKEN_OPEN_CURLY: return "open curly";
        case TOKEN_CLOSE_CURLY: return "close curly";
        case TOKEN_INVALID: return "invalid";
        case TOKEN_OPEN_BRACKET: return "open bracket";
        case TOKEN_CLOSE_BRACKET: return "close bracket";
        case TOKEN_COMMA: return "comma";
        case TOKEN_COLON: return "colon";
        case TOKEN_STRING: return "string";
        case TOKEN_NUMBER: return "number";
        default: UNREACHABLE("token_type_name");
    }
}