#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "season.h"

int main(int argc, char *argv[]) {
    (void) argc;
    struct season season;
    struct timeval a, b, c, d;

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



    gettimeofday(&a, NULL);
    season_load(&season, data);
    gettimeofday(&b, NULL);
    season_render(&season, null);
    gettimeofday(&c, NULL);
    season_free(&season);
    gettimeofday(&d, NULL);

    long load = (b.tv_sec - a.tv_sec)*1000000 + b.tv_usec - a.tv_usec;
    long render = (c.tv_sec - b.tv_sec)*1000000 + c.tv_usec - b.tv_usec;
    long free_time = (d.tv_sec - c.tv_sec)*1000000 + d.tv_usec - c.tv_usec;
    printf("Loading speed: %lfo/s\n", size/(float)load*1000000);
    printf("Rendering speed: %lfo/s\n", size/(float)render*1000000);
    printf("Freeing speed: %lfo/s\n", size/(float)free_time*1000000);

    free(data);
    fclose(fp);
    fclose(null);
    exit(EXIT_SUCCESS);
}