#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define SEASON_IMPLEMENTATION
#include "season.h"

int main(int argc, char *argv[]) {
    (void) argc;
    struct season season;
    struct timeval start, end_parse, end_render, end_free;

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size < 0) {
        fclose(fp);
        return 1;
    }

    FILE *null = fopen("/dev/null", "w");
    if (null == NULL) {
        fprintf(stderr, "Failed to open /dev/null\n");
        fclose(fp);
        return 1;
    }

    char *data = malloc(size + 1);
    int _ = fread(data, 1, size, fp);
    (void)_;



    gettimeofday(&start, NULL);
    season_load(&season, data);
    gettimeofday(&end_parse, NULL);
    season_render(&season, null);
    gettimeofday(&end_render, NULL);
    season_free(&season);
    gettimeofday(&end_free, NULL);

    long load = (end_parse.tv_sec - start.tv_sec)*1000000
                    + end_parse.tv_usec - start.tv_usec;
    long render = (end_render.tv_sec - end_parse.tv_sec)*1000000
                    + end_render.tv_usec - end_parse.tv_usec;
    long free_time = (end_free.tv_sec - end_render.tv_sec)*1000000
                    + end_free.tv_usec - end_render.tv_usec;

    printf("Loading speed:   %10ld bytes/s\n", size*1000000/load);
    printf("Rendering speed: %10ld bytes/s\n", size*1000000/render);
    printf("Freeing speed:   %10ld bytes/s\n", size*1000000/free_time);

    free(data);
    fclose(fp);
    fclose(null);
    exit(EXIT_SUCCESS);
}