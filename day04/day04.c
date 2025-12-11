#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    bool* grid;
    const size_t rows;
    const size_t cols;
    const bool parse_successful;
} Data;

Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    bool* data = NULL;
    char* line = NULL;
    size_t rows = 0, cols = 0, len;
    while (getline(&line, &len, fp) > 0) {
        len = strlen(line);
        if (line[len - 1] == '\n') {
            len--;
            if (!cols) {
                cols = len;
            }
        }
        bool* new = realloc(data, sizeof(bool) * (rows + 1) * cols);
        if (!new) {
            perror("Out of memory");
            free(data);
            free(line);
            fclose(fp);
            goto error;
        }
        data = new;

        for (size_t j = 0; j < len; j++) {
            const bool b = (bool) (line[j] == '@');
            data[rows*cols + j] = b;
        }
        rows++;
    }

    free(line);
    fclose(fp);
    return (Data) { data, rows, cols, true };
error:
    return (Data) { NULL, 0, 0, false };
}

size_t nAdjacent(const bool* grid, const size_t rows, const size_t cols, const size_t i) {
    int left = 0, up_left = 0, up = 0, up_right = 0, right = 0, down_right = 0, down = 0, down_left = 0;

    if (i % cols > 0 && grid[i - 1]) {
        left = 1;
    }
    if (i % cols > 0 && i / cols > 0 && grid[i - cols - 1]) {
        up_left = 1;
    }
    if (i / cols > 0 && grid[i - cols]) {
        up = 1;
    }
    if (i % cols < cols - 1 && i / cols > 0 && grid[i - cols + 1]) {
        up_right = 1;
    }
    if (i % cols < cols - 1 && grid[i + 1]) {
        right = 1;
    }
    if (i % cols < cols - 1 && i / cols < rows - 1 && grid[i + cols + 1]) {
        down_right = 1;
    }
    if (i / cols < rows - 1 && grid[i + cols]) {
        down = 1;
    }
    if (i % cols > 0 && i / cols < rows - 1 && grid[i + cols - 1]) {
        down_left = 1;
    }

    return (size_t) (left + up_left + up + up_right + right + down_right + down + down_left);
}

size_t part1(const Data* data) {
    const bool* grid = data->grid;
    const size_t rows = data->rows, cols = data->cols;

    size_t reachable = 0;
    for (size_t i = 0; i < rows * cols; i++) {
        if (grid[i] && nAdjacent(grid, rows, cols, i) < 4) {
            reachable++;
        }
    }
    return reachable;
}

size_t part2(const Data* data) {
    const bool* grid = data->grid;
    const size_t rows = data->rows, cols = data->cols;

    bool* prev = malloc(sizeof(bool) * rows * cols);
    if (!prev) {
        perror("Out of memory.");
        return 0;
    }
    bool* curr = malloc(sizeof(bool) * rows * cols);
    if (!curr) {
        perror("Out of memory.");
        free(prev);
        return 0;
    }

    memcpy(curr, grid, sizeof(bool) * rows * cols);
    size_t total_changes = 0;
    size_t n_changes;
    do {
        n_changes = 0;
        memcpy(prev, curr, sizeof(bool) * rows * cols);
        for (size_t i = 0; i < rows * cols; i++) {
            if (prev[i] && nAdjacent(prev, rows, cols, i) < 4) {
                curr[i] = false;
                n_changes++;
            }
        }

        total_changes += n_changes;
    } while (n_changes > 0);
    free(prev);
    free(curr);

    return total_changes;
}

int main(void) {
    const char* path = "inputs/day04.txt";
    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.grid);
}
