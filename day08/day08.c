#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    long x;
    long y;
    long z;
} Vec3;

typedef struct {
    Vec3* boxes;
    size_t n;
    bool parse_successful;
} Data;

bool isDigit(const char c) {
    return '0' <= c && c <= '9';
}

size_t parseInt(const char* line, const size_t start, const size_t end) {
    size_t num = 0;
    for (size_t i = start; i < end; i++) {
        num = num * 10 + (size_t) (line[i] - '0');
    }
    return num;
}

Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    size_t n = 0;
    Vec3* boxes = malloc(sizeof(Vec3) * n);
    if (!boxes) {
        perror("Out of memory.");
    error_1:
        fclose(fp);
        goto error;
    }

    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) > 0) {
        size_t start = 0, end = 1;
        while (isDigit(line[end])) {
            end++;
        }
        const long x = (long) parseInt(line, start, end);

        start = ++end;
        while (isDigit(line[end])) {
            end++;
        }
        const long y = (long) parseInt(line, start, end);

        start = ++end;
        while (isDigit(line[end])) {
            end++;
        }
        const long z = (long) parseInt(line, start, end);

        Vec3* new = realloc(boxes, sizeof(Vec3) * (n + 1));
        if (!new) {
            perror("Out of memory.");
            free(boxes);
            goto error_1;
        }

        boxes = new;
        boxes[n++] = (Vec3) { x, y, z };
    }
    free(line);
    fclose(fp);

    return (Data) { boxes, n, true };
error:
    return (Data) { NULL, 0, false };
}

size_t tile_area(const Vec3* a, const Vec3* b) {
    const long x_a = a->x, y_a = a->y;
    const long x_b = b->x, y_b = b->y;

    const size_t x_distance = (size_t) labs(x_a - x_b);
    const size_t y_distance = (size_t) labs(y_a - y_b);

    return (x_distance + 1) * (y_distance + 1);
}

size_t part1(const Data* data) {
    return 0;
}

size_t part2(const Data* data) {
    return 0;
}

int main(void) {
    const char* path = "inputs/day08.txt";

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.boxes);
}
