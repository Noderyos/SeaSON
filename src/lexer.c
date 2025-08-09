#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEASON_UNUSED(x) (void)(x)

static const enum season_token_type SEASON_LITERAL_MAP[256] = {
    ['{'] = SEASON_TOK_OPEN_CURLY,
    ['}'] = SEASON_TOK_CLOSE_CURLY,
    ['['] = SEASON_TOK_OPEN_BRACKET,
    [']'] = SEASON_TOK_CLOSE_BRACKET,
    [','] = SEASON_TOK_COMMA,
    [':'] = SEASON_TOK_COLON
};

struct season_lexer season_lex_init(char *content, size_t content_len){
    struct season_lexer l = {content, content_len, 0, 0, 0};
    return l;
}

#define season_lex_chop_char(l) ((l)->content[(l)->cursor++])
#define season_is_num_start(c) (strchr("0123456789-", c) != NULL)
#define season_is_num(c) (strchr("0123456789-.eE", c) != NULL)

void season_lex_trim_left(struct season_lexer *l){
    while (l->cursor < l->content_len && isspace(l->content[l->cursor])){
        SEASON_UNUSED(season_lex_chop_char(l));
    }
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
        while (l->cursor < l->content_len && l->content[l->cursor++] != '"') {
            token.text_len++;
        }
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
        if (strncmp(&l->content[l->cursor], "null", 4) == 0) {
            token.type = SEASON_TOK_NULL;
            token.text_len = 4;
        } else if (strncmp(&l->content[l->cursor], "true", 4) == 0) {
            token.type = SEASON_TOK_TRUE;
            token.text_len = 4;
        } else if (strncmp(&l->content[l->cursor], "false", 5) == 0) {
            token.type = SEASON_TOK_FALSE;
            token.text_len = 5;
        }
        return token;
    }

    enum season_token_type lit_type = SEASON_LITERAL_MAP[(unsigned char)l->content[l->cursor]];
    if (lit_type) {
        l->cursor++;
        token.type = lit_type;
        token.text_len = 1;
        return token;
    }

    SEASON_UNUSED(season_lex_chop_char(l));
    token.type = SEASON_TOK_INVALID;
    token.text_len = 1;
    return token;
}