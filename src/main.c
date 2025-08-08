#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "season.h"
#include "lexer.h"

#define ASSERT(x, msg) \
        do { \
            if(!(x)) { \
                fprintf(stderr, msg "\n"); \
                exit(1); \
            } \
       }while(0)

#define da_append(da, item) \
        do { \
            if (da.count >= da.capacity) {\
                da.capacity = da.capacity == 0 ? 8 : da.capacity*2; \
                da.items = realloc(da.items, da.capacity*sizeof(*da.items)); \
                ASSERT(da.items != NULL, "Buy more RAM lol"); \
            } \
            da.items[da.count++] = (item); \
        } while (0)

#define da_remove(da, idx) \
        do { \
            ASSERT(da.count != 0, "Array is empty lol"); \
            ASSERT((idx) < da.count, "IndexError: list index out of range :)"); \
            da.count--; \
            memmove(&da.items[idx], &da.items[(idx)+1], (da.count-(idx))*sizeof(*da.items)); \
        } while (0)

int main() {
    char *json = "[\"Hello, World!\", 3.141592, {\"name\": \"Noderyos\", \"age\": 18.500000}, null, true]";
    Lexer l = lexer_init(json, strlen(json));

    Token t = lexer_next(&l);
    while (t.type != TOKEN_END) {
        switch (t.type) {
            case TOKEN_END: printf("end\n");break;
            case TOKEN_OPEN_CURLY: printf("{\n");break;
            case TOKEN_CLOSE_CURLY: printf("}\n");break;
            case TOKEN_INVALID: printf("invalid\n");break;
            case TOKEN_OPEN_BRACKET: printf("[\n");break;
            case TOKEN_CLOSE_BRACKET: printf("]\n");break;
            case TOKEN_COMMA: printf(",\n");break;
            case TOKEN_COLON: printf(":\n");break;
            case TOKEN_STRING: printf("'%.*s'\n", (int)t.text_len, t.text);break;
            case TOKEN_NUMBER: printf("%lf\n", strtod(t.text, NULL));break;
            case TOKEN_NULL: printf("null\n");break;
            case TOKEN_TRUE: printf("true\n");break;
            case TOKEN_FALSE: printf("false\n");break;
            default: UNREACHABLE("token_type_name");
        }
        t = lexer_next(&l);
    }

    struct season array = {.type = SEASON_ARRAY};

    struct season item = {SEASON_STRING, .string.str = strdup("Hello, World!"), .string.len = strlen("Hello, World!")};
    season_array_add(&array, item);

    item = (struct season){SEASON_NUMBER, .number = 3.141592};
    season_array_add(&array, item);

    struct season object = (struct season){.type = SEASON_OBJECT};

    item = (struct season){SEASON_STRING, .string.str = strdup("Noderyos"), .string.len = strlen("Noderyos")};
    season_object_add(&object, "name", &item);

    item = (struct season){SEASON_NUMBER, .number = 18.5};
    season_object_add(&object, "age", &item);
    season_array_add(&array, object);

    item = (struct season){.type = SEASON_NULL};
    season_array_add(&array, item);

    item = (struct season){SEASON_BOOLEAN, .boolean = 1};
    season_array_add(&array, item);

    season_render(&array, stdout);
    printf("\n");

    season_free(&array);

    return 0;
}