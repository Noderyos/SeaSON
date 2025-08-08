#include "season.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

#define SEASON_ASSERT(x, msg) \
        do { \
            if(!(x)) { \
                fprintf(stderr, msg "\n"); \
                exit(1); \
            } \
        }while(0)

void season_array_add(struct season *array, struct season item) {
    SEASON_ASSERT(array->type == SEASON_ARRAY, "Object must be an array");
    if (array->array.count >= array->array.capacity) {
        array->array.capacity = array->array.capacity == 0 ? 8 : array->array.capacity*2;
        array->array.items = realloc(array->array.items, array->array.capacity*sizeof(*array->array.items));
        SEASON_ASSERT(array->array.items != NULL, "Buy more RAM lol");
    }
    array->array.items[array->array.count++] = item;
}

void season_object_add(struct season *object, char *key, struct season *item) {
    SEASON_ASSERT(object->type == SEASON_OBJECT, "Object must be an object");
    if (object->object.count >= object->object.capacity) {
        object->object.capacity = object->object.capacity == 0 ? 8 : object->object.capacity*2;
        object->object.items = realloc(object->object.items, object->object.capacity*sizeof(*object->object.items));
        SEASON_ASSERT(object->object.items != NULL, "Buy more RAM lol");
    }
    object->object.items[object->object.count].key = strdup(key);
    object->object.items[object->object.count].value = malloc(sizeof(*item));
    memcpy(object->object.items[object->object.count].value, item, sizeof(*item));
    object->object.count++;
}

int season_is_int(double x) {
    const double eps = 1e-9;
    long xi = (long)x;
    double diff = x - (double)xi;

    if (diff < 0) diff = -diff;
    return diff < eps;
}

void season_render(struct season *season, FILE *stream) {
    switch (season->type) {
        case SEASON_NULL:
            fprintf(stream, "null");
            break;
        case SEASON_BOOLEAN:
            fprintf(stream, "%s", season->boolean ? "true" : "false");
            break;
        case SEASON_STRING:
            fprintf(stream, "\"%.*s\"", (int)season->string.len, season->string.str);
            break;
        case SEASON_NUMBER:
            if (season_is_int(season->number))
                fprintf(stream, "%ld", (long)season->number);
            else
                fprintf(stream, "%lf", season->number);
            break;
        case SEASON_OBJECT:
            fprintf(stream, "{");
            for (size_t i = 0; i < season->object.count; i++) {
                if (i) fprintf(stream, ", ");
                struct season_object_el object = season->object.items[i];
                fprintf(stream, "\"%s\": ", object.key);
                season_render(object.value, stream);
            }
            fprintf(stream, "}");
            break;
        case SEASON_ARRAY:
            fprintf(stream, "[");
            for (size_t i = 0; i < season->array.count; i++) {
                if (i) fprintf(stream, ", ");
                season_render(&season->array.items[i], stream);
            }
            fprintf(stream, "]");
            break;
    }
}

void season_free(struct season *season) {
    switch (season->type) {
        case SEASON_STRING:
            free(season->string.str);
            season->string.str = NULL;
            season->string.len = 0;
            break;
        case SEASON_OBJECT:
            for (size_t i = 0; i < season->object.count; i++) {
                struct season_object_el object = season->object.items[i];
                free(object.key);
                season_free(object.value);
                free(object.value);
            }
            free(season->object.items);
            season->object.items = NULL;
            season->object.count = 0;
            season->object.capacity = 0;
            break;
        case SEASON_ARRAY:
            for (size_t i = 0; i < season->array.count; i++) {
                season_free(&season->array.items[i]);
            }
            free(season->array.items);
            season->array.items = NULL;
            season->array.count = 0;
            season->array.capacity = 0;
            break;
        case SEASON_NULL:
        case SEASON_BOOLEAN:
        case SEASON_NUMBER:
        default:
            // Nothing to free
            break;
    }
}

struct season season_parse_symbol(struct season_token t) {
    struct season value;
    switch (t.type) {
        case SEASON_TOK_STRING:
            value.type = SEASON_STRING;
            value.string.str = strndup(t.text, t.text_len);
            value.string.len = t.text_len;
            break;
        case SEASON_TOK_NUMBER:
            value.type = SEASON_NUMBER;
            value.number = strtod(t.text, NULL);
            break;
        case SEASON_TOK_NULL:
            value.type = SEASON_NULL;
            break;
        case SEASON_TOK_TRUE:
            value.type = SEASON_BOOLEAN;
            value.boolean = 1;
            break;
        case SEASON_TOK_FALSE:
            value.type = SEASON_BOOLEAN;
            value.boolean = 0;
            break;
        default:
            SEASON_LEX_UNREACH("Not a symbol");
    }
    return value;
}

struct season season_parse_array(struct season_lexer *l);
struct season season_parse_object(struct season_lexer *l) {
    struct season object = {.type = SEASON_OBJECT};
    struct season_token t = season_lex_next(l);
    while (t.type != SEASON_TOK_CLOSE_CURLY) {
        if (t.type != SEASON_TOK_STRING) SEASON_LEX_UNREACH("Expecting key");
        char *key = strndup(t.text, t.text_len);
        if (season_lex_next(l).type != SEASON_TOK_COLON) SEASON_LEX_UNREACH("Expecting colon");
        t = season_lex_next(l);

        struct season value;
        switch (t.type) {
            case SEASON_TOK_STRING:
            case SEASON_TOK_NUMBER:
            case SEASON_TOK_NULL:
            case SEASON_TOK_TRUE:
            case SEASON_TOK_FALSE:
                value = season_parse_symbol(t);
                break;

            case SEASON_TOK_OPEN_CURLY:
                value = season_parse_object(l);
                break;
            case SEASON_TOK_OPEN_BRACKET:
                value = season_parse_array(l);
                break;

            case SEASON_TOK_CLOSE_CURLY:
                SEASON_LEX_UNREACH("Should not fall here");

            default:
                SEASON_LEX_UNREACH("Invalid token");
        }
        season_object_add(&object, key, &value);
        free(key);
        t = season_lex_next(l);
        if (t.type != SEASON_TOK_CLOSE_CURLY && t.type != SEASON_TOK_COMMA) SEASON_LEX_UNREACH("Expecting comma");
        if (t.type == SEASON_TOK_COMMA && (t=season_lex_next(l)).type == SEASON_TOK_CLOSE_CURLY) SEASON_LEX_UNREACH("Expecting value");
    }
    return object;
}

struct season season_parse_array(struct season_lexer *l) {
    struct season array = {.type = SEASON_ARRAY};
    struct season_token t = season_lex_next(l);
    while (t.type != SEASON_TOK_CLOSE_BRACKET) {
        struct season value;
        switch (t.type) {
            case SEASON_TOK_STRING:
            case SEASON_TOK_NUMBER:
            case SEASON_TOK_NULL:
            case SEASON_TOK_TRUE:
            case SEASON_TOK_FALSE:
                value = season_parse_symbol(t);
                break;

            case SEASON_TOK_OPEN_CURLY:
                value = season_parse_object(l);
                break;
            case SEASON_TOK_OPEN_BRACKET:
                value = season_parse_array(l);
                break;

            case SEASON_TOK_CLOSE_BRACKET:
                SEASON_LEX_UNREACH("Should not fall here");

            default:
                SEASON_LEX_UNREACH("Invalid token");
        }
        season_array_add(&array, value);
        t = season_lex_next(l);
        if (t.type != SEASON_TOK_CLOSE_BRACKET && t.type != SEASON_TOK_COMMA) SEASON_LEX_UNREACH("Expecting comma");
        if (t.type == SEASON_TOK_COMMA && (t=season_lex_next(l)).type == SEASON_TOK_CLOSE_BRACKET) SEASON_LEX_UNREACH("Expecting value");
    }
    return array;
}

void season_load(struct season *season, char *json_string) {
    struct season_lexer l = season_lex_init(json_string, strlen(json_string));
    struct season_token t = season_lex_next(&l);
    switch (t.type) {
        case SEASON_TOK_OPEN_CURLY:
            *season = season_parse_object(&l);
            break;
        case SEASON_TOK_OPEN_BRACKET:
            *season = season_parse_array(&l);
            break;
        case SEASON_TOK_STRING:
        case SEASON_TOK_NUMBER:
        case SEASON_TOK_NULL:
        case SEASON_TOK_TRUE:
        case SEASON_TOK_FALSE:
            *season = season_parse_symbol(t);
            break;
        default:
            SEASON_LEX_UNREACH("Invalid token");
    }
}