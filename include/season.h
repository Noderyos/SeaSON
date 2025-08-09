#ifndef SEASON_H
#define SEASON_H

#include <stddef.h>
#include <stdio.h>

enum season_type {
    SEASON_STRING,
    SEASON_NULL,
    SEASON_NUMBER,
    SEASON_BOOLEAN,
    SEASON_OBJECT,
    SEASON_ARRAY,
};

struct season_string {
    size_t len;
    char *str;
};
struct season_object {
    size_t count;
    size_t capacity;
    struct season_object_el {
        char *key;
        struct season *value;
    } *items;
};
struct season_array {
    size_t count;
    size_t capacity;
    struct season *items;
};

struct season {
    enum season_type type;
    union {
        struct season_string string;
        double number;
        int boolean;
        struct season_object object;
        struct season_array array;
    };
};


struct season *season_object_get(struct season *object, const char *key);
void season_object_add(struct season *object, char *key, struct season *item);

struct season *season_array_get(struct season *array, size_t idx);
void season_array_add(struct season *array, struct season item);

void season_load(struct season *season, char *json_string);
void season_render(struct season *season, FILE *stream);
void season_free(struct season *season);

#endif