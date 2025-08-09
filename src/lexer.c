#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define _season_lex_chop_char(l) ((l)->content[(l)->cursor++])
#define _season_is_num_start(c) (strchr("0123456789-", c) != NULL)
#define _season_is_num(c) (strchr("0123456789-.eE", c) != NULL)


struct season_token season_lex_next(struct season_lexer *l){
    while (l->cursor < l->content_len && isspace(l->content[l->cursor])){
        (void)_season_lex_chop_char(l);
    }

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

    if (_season_is_num_start(l->content[l->cursor])) {
        token.type = SEASON_TOK_NUMBER;
        while (l->cursor < l->content_len && _season_is_num(l->content[l->cursor])) {
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
            l->cursor += 4;
        } else if (strncmp(&l->content[l->cursor], "true", 4) == 0) {
            token.type = SEASON_TOK_TRUE;
            token.text_len = 4;
            l->cursor += 4;
        } else if (strncmp(&l->content[l->cursor], "false", 5) == 0) {
            token.type = SEASON_TOK_FALSE;
            token.text_len = 5;
            l->cursor += 5;
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

    (void)_season_lex_chop_char(l);
    token.type = SEASON_TOK_INVALID;
    token.text_len = 1;
    return token;
}