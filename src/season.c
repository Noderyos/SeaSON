#include "season.h"

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEASON_ASSERT(x, fmt, ...) \
        do { \
            if(!(x)) { \
                fprintf(stderr, "assert: %s:%d: %s: '%s': " \
                                fmt "\n", __FILE__, __LINE__, __FUNCTION__, \
                                #x, ##__VA_ARGS__); \
                exit(1); \
            } \
        }while(0)

#define SEASON_ERROR(fmt, ...) \
        do { \
                fprintf(stderr, "error: %s:%d: %s: " fmt "\n", \
                        __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
                exit(1); \
        }while(0)

#define SEASON_PARSE_ERROR(fmt, ...) \
        do { \
                fprintf(stderr, "season parse error: line %ld column %ld: " fmt "\n",\
                t.line, t.column, ##__VA_ARGS__); \
                exit(1); \
        }while(0)

char *_season_strdup(const char *s) { // C99 don't have strdup
    size_t size = strlen(s) + 1;
    char *str = malloc(size);
    if (str) memcpy(str, s, size);
    return str;
}

int _season_is_int(double x) {
    const double eps = 1e-9;
    long xi = (long)x;
    double diff = x - (double)xi;

    if (diff < 0) diff = -diff;
    return diff < eps;
}


char *_season_escape(const char *str, size_t len) {
    char *out = malloc(len*2 + 1);
    if (!out) return NULL;

    char *p = out;

    while (len--) {
        char c = *str++;
        switch (c) {
            case '"':  *p++ = '\\'; *p++ = '"';  break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '/':  *p++ = '\\'; *p++ = '/';  break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\v': *p++ = '\\'; *p++ = 'v';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            default:
                *p++ = c;
                break;
        }
    }
    *p = '\0';
    return out;
}

char *_season_unescape(const char *str, size_t len) {
    char *out = malloc(len + 1);
    char *p = out;

    while(len--) {
        if (*str == '\\') {
            str++;
            char next = *str;
            switch(next) {
                case '"':
                case '\\':
                case '/':
                    *p++ = next;
                    break;
                case 'b':*p++ = 8;break;
                case 't':*p++ = 9;break;
                case 'n':*p++ = 10;break;
                case 'v':*p++ = 11;break;
                case 'f':*p++ = 12;break;
                case 'r':*p++ = 13;break;
                case 'u':
                    SEASON_ERROR("Unicode is not yet supported");
                default:
                    SEASON_ERROR("Invalid escape code");
            }
            str++;
        } else {
            *p++ = *str++;
        }
    }
    *p = '\0';
    return out;
}

struct season _season_parse_symbol(struct season_token t) {
    struct season value;
    switch (t.type) {
        case SEASON_TOK_STRING:
            value.type = SEASON_STRING;
            value.string.str = _season_unescape(t.text, t.text_len);
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
            SEASON_ASSERT(0, "Not a symbol (my fault, not yours)");
    }
    return value;
}

struct season _season_parse_array(struct season_lexer *l);
struct season _season_parse_object(struct season_lexer *l) {
    struct season object = {.type = SEASON_OBJECT};
    struct season_token t = season_lex_next(l);
    while (t.type != SEASON_TOK_CLOSE_CURLY) {
        if (t.type != SEASON_TOK_STRING)
            SEASON_PARSE_ERROR("Expecting key");
        char *key = _season_unescape(t.text, t.text_len);
        t = season_lex_next(l);
        if (t.type != SEASON_TOK_COLON)
            SEASON_PARSE_ERROR("Expecting ':'");
        t = season_lex_next(l);

        struct season value;
        switch (t.type) {
            case SEASON_TOK_STRING:
            case SEASON_TOK_NUMBER:
            case SEASON_TOK_NULL:
            case SEASON_TOK_TRUE:
            case SEASON_TOK_FALSE:
                value = _season_parse_symbol(t);
                break;

            case SEASON_TOK_OPEN_CURLY:
                value = _season_parse_object(l);
                break;
            case SEASON_TOK_OPEN_BRACKET:
                value = _season_parse_array(l);
                break;

            default:
                SEASON_PARSE_ERROR("Invalid token");
        }
        season_object_add(&object, key, &value);
        free(key);
        t = season_lex_next(l);
        if (t.type != SEASON_TOK_CLOSE_CURLY && t.type != SEASON_TOK_COMMA)
            SEASON_PARSE_ERROR("Expecting ','");
        if (t.type == SEASON_TOK_COMMA) {
            t = season_lex_next(l);
            if (t.type == SEASON_TOK_CLOSE_CURLY)
                SEASON_PARSE_ERROR("Illegal trailing comma before end of object");
        }
    }
    return object;
}

struct season _season_parse_array(struct season_lexer *l) {
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
                value = _season_parse_symbol(t);
                break;

            case SEASON_TOK_OPEN_CURLY:
                value = _season_parse_object(l);
                break;
            case SEASON_TOK_OPEN_BRACKET:
                value = _season_parse_array(l);
                break;

            default:
                SEASON_PARSE_ERROR("Invalid token");
        }
        season_array_add(&array, value);
        t = season_lex_next(l);
        if (t.type != SEASON_TOK_CLOSE_BRACKET && t.type != SEASON_TOK_COMMA)
            SEASON_PARSE_ERROR("Expecting ','");
        if (t.type == SEASON_TOK_COMMA) {
            t = season_lex_next(l);
            if (t.type == SEASON_TOK_CLOSE_BRACKET)
                SEASON_PARSE_ERROR("Illegal trailing comma before end of array");
        }
    }
    return array;
}

int _season_object_idx(struct season *object, const char *key) {
    SEASON_ASSERT(object != NULL, "object must be non-null");
    SEASON_ASSERT(object->type == SEASON_OBJECT, "object must be an object");

    for (size_t i = 0; i < object->object.count; i++) {
        if (strcmp(object->object.items[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}


struct season *season_object_get(struct season *object, const char *key) {
    int idx = _season_object_idx(object, key);
    if (idx < 0) return NULL;
    return object->object.items[idx].value;
}

void season_object_add(struct season *object, char *key, struct season *item) {
    SEASON_ASSERT(object != NULL, "object must be non-null");
    SEASON_ASSERT(object->type == SEASON_OBJECT, "object must be an object");
    int key_idx = _season_object_idx(object, key);
    if (key_idx < 0) {
        if (object->object.count >= object->object.capacity) {
            object->object.capacity =
                object->object.capacity == 0 ? 8 : object->object.capacity*2;
            object->object.items = realloc(
                object->object.items, object->object.capacity*sizeof(*object->object.items));
            SEASON_ASSERT(object->object.items != NULL, "Buy more RAM lol");
        }
        struct season_object_el *el = &object->object.items[object->object.count];
        el->key = _season_strdup(key);
        el->value = malloc(sizeof(*item));
        memcpy(el->value, item, sizeof(*item));
        object->object.count++;
    } else {
        season_free(object->object.items[key_idx].value);
        memcpy(object->object.items[key_idx].value, item, sizeof(*item));
    }
}

void season_object_remove(struct season *object, char *key) {
    SEASON_ASSERT(object != NULL, "object must be non-null");
    SEASON_ASSERT(object->type == SEASON_OBJECT, "object must be an object");

    int idx = _season_object_idx(object, key);
    if (idx >= 0) {
        free(object->object.items[idx].key);
        season_free(object->object.items[idx].value);
        free(object->object.items[idx].value);
        memcpy(&object->object.items[idx], &object->object.items[idx+1],
            (object->object.count-idx-1)*sizeof(*object->object.items));
        object->object.count--;
    }
}

struct season *season_array_get(struct season *array, size_t idx) {
    SEASON_ASSERT(array != NULL, "array must be non-null");
    SEASON_ASSERT(array->type == SEASON_ARRAY, "array must be an array");

    SEASON_ASSERT(idx < array->object.count, "invalid index");

    return &array->array.items[idx];
}

void season_array_add(struct season *array, struct season item) {
    SEASON_ASSERT(array != NULL, "array must be non-null");
    SEASON_ASSERT(array->type == SEASON_ARRAY, "array must be an array");
    season_array_insert(array, item, array->array.count);
}

void season_array_remove(struct season *array, size_t idx) {
    SEASON_ASSERT(array != NULL, "array must be non-null");
    SEASON_ASSERT(array->type == SEASON_ARRAY, "array must be an array");

    SEASON_ASSERT(idx < array->object.count, "invalid index");

    season_free(&array->array.items[idx]);
    memcpy(&array->array.items[idx], &array->array.items[idx+1],
        (array->array.count-idx-1)*sizeof(*array->array.items));
    array->array.count--;
}

void season_array_insert(struct season *array, struct season item, size_t idx) {
    SEASON_ASSERT(array != NULL, "array must be non-null");
    SEASON_ASSERT(array->type == SEASON_ARRAY, "array must be an array");

    SEASON_ASSERT(idx <= array->object.count, "invalid index");

    if (array->array.count >= array->array.capacity) {
        array->array.capacity =
            array->array.capacity == 0 ? 8 : array->array.capacity*2;
        array->array.items = realloc(
            array->array.items, array->array.capacity*sizeof(*array->array.items));
        SEASON_ASSERT(array->array.items != NULL, "Buy more RAM lol");
    }
    memcpy(&array->array.items[idx+1], &array->array.items[idx],
        (array->array.count-idx)*sizeof(*array->array.items));
    array->array.items[idx] = item;
    array->array.count++;
}

void season_load(struct season *season, char *json_string) {
    struct season_lexer l = season_lex_init(json_string, strlen(json_string));
    struct season_token t = season_lex_next(&l);
    switch (t.type) {
        case SEASON_TOK_OPEN_CURLY:
            *season = _season_parse_object(&l);
            break;
        case SEASON_TOK_OPEN_BRACKET:
            *season = _season_parse_array(&l);
            break;
        case SEASON_TOK_STRING:
        case SEASON_TOK_NUMBER:
        case SEASON_TOK_NULL:
        case SEASON_TOK_TRUE:
        case SEASON_TOK_FALSE:
            *season = _season_parse_symbol(t);
            break;
        default:
            SEASON_PARSE_ERROR("Invalid token");
    }
}

void season_render(struct season *season, FILE *stream) {
    SEASON_ASSERT(season != NULL, "season must be non-null");
    switch (season->type) {
        case SEASON_NULL:
            fprintf(stream, "null");
            break;
        case SEASON_BOOLEAN:
            fprintf(stream, "%s", season->boolean ? "true" : "false");
            break;
        case SEASON_STRING:
            char *escaped = _season_escape(season->string.str, season->string.len);
            fprintf(stream, "\"%s\"", escaped);
            free(escaped);
            break;
        case SEASON_NUMBER:
            if (_season_is_int(season->number))
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
    SEASON_ASSERT(season != NULL, "season must be non-null");
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