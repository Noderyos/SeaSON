#include "season.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ASSERT(x, msg) \
        do { \
            if(!(x)) { \
                fprintf(stderr, msg "\n"); \
                exit(1); \
            } \
        }while(0)

void season_array_add(struct season *array, struct season item) {
    ASSERT(array->type == SEASON_ARRAY, "Object must be an array");
    if (array->array.count >= array->array.capacity) {
        array->array.capacity = array->array.capacity == 0 ? 8 : array->array.capacity*2;
        array->array.items = realloc(array->array.items, array->array.capacity*sizeof(*array->array.items));
        ASSERT(array->array.items != NULL, "Buy more RAM lol");
    }
    array->array.items[array->array.count++] = item;
}

void season_object_add(struct season *object, char *key, struct season *item) {
    ASSERT(object->type == SEASON_OBJECT, "Object must be an object");
    if (object->object.count >= object->object.capacity) {
        object->object.capacity = object->object.capacity == 0 ? 8 : object->object.capacity*2;
        object->object.items = realloc(object->object.items, object->object.capacity*sizeof(*object->object.items));
        ASSERT(object->object.items != NULL, "Buy more RAM lol");
    }
    object->object.items[object->object.count].key = strdup(key);
    object->object.items[object->object.count].value = malloc(sizeof(*item));
    memcpy(object->object.items[object->object.count].value, item, sizeof(*item));
    object->object.count++;
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