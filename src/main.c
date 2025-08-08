#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "season.h"
#include "lexer.h"

int main(void) {
    struct season season;

    char *line;
    size_t len = 0;

    FILE *fp = fopen("test.json", "r");

    while (getline(&line, &len, fp) != -1) {
        line[strcspn(line, "\n")] = 0;
        season_load(&season, line);
        printf("%s => ", line);
        season_render(&season, stdout);
        printf("\n");
        season_free(&season);
    }

    fclose(fp);
    exit(EXIT_SUCCESS);
}