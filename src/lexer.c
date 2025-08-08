#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define SEASON_UNUSED(x) (void)(x)

const struct {
    char c;
    enum season_token_type type;
} season_literals[] = {
        {'{', SEASON_TOK_OPEN_CURLY},
        {'}', SEASON_TOK_CLOSE_CURLY},
        {'[', SEASON_TOK_OPEN_BRACKET},
        {']', SEASON_TOK_CLOSE_BRACKET},
        {',', SEASON_TOK_COMMA},
        {':', SEASON_TOK_COLON}
};

#define season_literals_count (sizeof(season_literals)/sizeof(*season_literals))

struct season_lexer season_lex_init(char *content, size_t content_len){
    struct season_lexer l = {content, content_len, 0, 0, 0};
    return l;
}

char season_lex_chop_char(struct season_lexer *l){
    assert(l->cursor < l->content_len);

    char x = l->content[l->cursor];
    l->cursor++;
    if(x == '\n'){
        l->line++;
        l->bol = l->cursor;
    }
    return x;
}

void season_lex_trim_left(struct season_lexer *l){
    while (l->cursor < l->content_len && isspace(l->content[l->cursor])){
        SEASON_UNUSED(season_lex_chop_char(l));
    }
}

int season_is_num_start(char c){
    return strchr("0123456789-", c) != NULL;
}

int season_is_num(char c){
    return strchr("0123456789-.eE", c) != NULL;
}

struct season_token season_lex_next(struct season_lexer *l){
    season_lex_trim_left(l);
    struct season_token token = {
            .text = &l->content[l->cursor]
    };

    if (l->cursor >= l->content_len) return token;

    if (l->content[l->cursor] == '"') {
        token.type = SEASON_TOK_STRING;
        l->cursor++;
        token.text++;
        while (l->cursor < l->content_len && l->content[l->cursor] != '"') {
            token.text_len++;
            l->cursor++;
        }
        l->cursor++;
        return token;
    }

    if (season_is_num_start(l->content[l->cursor])) {
        token.type = SEASON_TOK_NUMBER;
        while (l->cursor < l->content_len && season_is_num(l->content[l->cursor])) {
            token.text_len++;
            l->cursor++;
        }
        return token;
    }

    if (islower(l->content[l->cursor])) {
        token.type = SEASON_TOK_INVALID;
        while (l->cursor < l->content_len && islower(l->content[l->cursor])) {
            token.text_len++;
            l->cursor++;
        }
        if (token.text_len == 4 && strncmp(token.text, "null", 4) == 0)
            token.type = SEASON_TOK_NULL;
        else if (token.text_len == 4 && strncmp(token.text, "true", 4) == 0)
            token.type = SEASON_TOK_TRUE;
        else if (token.text_len == 5 && strncmp(token.text, "false", 5) == 0)
            token.type = SEASON_TOK_FALSE;

        return token;
    }

    for (size_t i = 0; i < season_literals_count; ++i) {
        if(l->content[l->cursor] == season_literals[i].c){
            SEASON_UNUSED(season_lex_chop_char(l));
            token.type = season_literals[i].type;
            token.text_len = 1;
            return token;
        }
    }

    SEASON_UNUSED(season_lex_chop_char(l));
    token.type = SEASON_TOK_INVALID;
    token.text_len = 1;
    return token;
}