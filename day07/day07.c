#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    char** lines;
    const size_t start_x;
    const size_t width, depth;
    const bool parse_successful;
} Data;

Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    char** lines = NULL;
    char* line = NULL;
    size_t n = 0, start_x = 0, len;
    while (getline(&line, &len, fp) > 0) {
        len = strlen(line);
        line[--len] = '\0';
        if (!len) {
            break;
        }

        if (!start_x) {
            for (size_t x = 0; x < len; x++) {
                if (line[x] == 'S') {
                    start_x = x;
                    break;
                }
            }
        }

        char** new = realloc(lines, (n + 1) * sizeof(char*));
        if (!new) {
            perror("Out of memory.");
            goto error_1;
        }
        lines = new;

        char* memline = malloc(len);
        if (!memline) {
            perror("Out of memory.");
        error_1:
            for (size_t i = 0; i < n; i++) {
                free(lines[i]);
            }
            free(lines);
            free(line);
            fclose(fp);
            goto error;
        }
        memcpy(memline, line, len);
        lines[n++] = memline;
    }
    free(line);
    fclose(fp);

    return (Data) { .lines = lines, .start_x = start_x, .width = strlen(lines[0]), .depth = n, .parse_successful = true };
error:
    return (Data) { .lines = NULL, .start_x = 0, .width = 0, .depth = 0, .parse_successful = false };
}

size_t countSplits(char** grid, const size_t depth) {
    size_t splits = 0;
    for (size_t curr = 1, prev = 0; curr < depth; curr++, prev++) {
        for (size_t col = 0; col < strlen(grid[curr]); col++) {
            if (grid[curr][col] == '^' && grid[prev][col] == '|') {
                grid[curr][col - 1] = '|';
                grid[curr][col + 1] = '|';
                splits++;
            }
            if (grid[curr][col] == '.' && grid[prev][col] == '|') {
                grid[curr][col] = '|';
            }
        }
    }
    return splits;
}

size_t part1(const Data* data) {
    char** grid = malloc(data->depth * sizeof(char*));
    if (!grid) {
        perror("Out of memory.");
        return 0;
    }
    for (size_t i = 0; i < data->depth; i++) {
        grid[i] = malloc(data->width);
        if (!grid[i]) {
            perror("Out of memory.");
            for (size_t j = 0; j < i; j++) {
                free(grid[j]);
            }
            free(grid);
            return 0;
        }
        strcpy(grid[i], data->lines[i]);
    }

    grid[1][data->start_x] = '|';
    const size_t splits = countSplits(grid, data->depth);

    for (size_t i = 0; i < data->depth; i++) {
        free(grid[i]);
    }
    free(grid);

    return splits;
}

size_t part2(const Data* data) {
    size_t* curr = calloc(data->width, sizeof(size_t));
    if (!curr) {
        perror("Out of memory.");
        return 0;
    }
    size_t* next = malloc(data->width * sizeof(size_t));
    if (!next) {
        perror("Out of memory.");
        free(curr);
        return 0;
    }

    // initial beam just below S
    curr[data->start_x] = 1;

    for (size_t y = 1; y < data->depth; y++) {
        memset(next, 0, data->width * sizeof(size_t));

        for (size_t x = 0; x < data->width; x++) {
            const size_t count = curr[x];
            if (count == 0) continue;

            if (data->lines[y][x] == '^') {
                next[x - 1] += count;
                next[x + 1] += count;
            } else {
                next[x] += count;
            }
        }

        size_t* temp = curr;
        curr = next;
        next = temp;
    }
    free(next);

    size_t total_beams = 0;
    for (size_t x = 0; x < data->width; x++) {
        total_beams += curr[x];
    }
    free(curr);
    return total_beams;
}

int main(void) {
    const char* path = "inputs/day07.txt";

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    for (size_t i = 0; i < data.depth; i++) {
        free(data.lines[i]);
    }
    free(data.lines);
}
