#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef unsigned char byte;

typedef struct {
    byte* batteries;
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

    byte* data = NULL;
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
        byte* new = realloc(data, sizeof(byte) * (rows + 1) * cols);
        if (!new) {
            perror("Out of memory");
            free(data);
            free(line);
            fclose(fp);
            goto error;
        }
        data = new;

        for (size_t j = 0; j < len; j++) {
            const byte b = (byte) (line[j] - '0');
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

size_t bestTwo(const byte* batteries, const size_t start, const size_t end) {
    size_t max = 0;
    for (size_t i = start; i < end; i++) {
        for (size_t j = i + 1; j < end; j++) {
            const size_t curr = (size_t) (batteries[i] * 10 + batteries[j]);
            if (curr > max) {
                max = curr;
            }
        }
    }
    return max;
}

size_t part1(const Data* data) {
    const byte* batteries = data->batteries;
    const size_t rows = data->rows, cols = data->cols;

    size_t sum = 0;
    for (size_t i = 0; i < rows; i++) {
        sum += bestTwo(batteries, i*cols, (i+1)*cols);
    }
    return sum;
}

size_t best(const byte* batteries, const size_t start, const size_t end, const size_t depth) {
    size_t left = start;
    size_t curr = 0;
    for (size_t d = depth; d > 0; d--) {
        size_t left_most_i = end - d;
        byte left_most = batteries[left_most_i];

        // greedily taking the highest number but only in such a way
        // that at least depth characters are left over
        for (int i = (int) (end - d); i >= (int) left; i--) {
            const byte c = batteries[i];
            if (c >= left_most) {
                left_most_i = (size_t) i;
                left_most = c;
            }
        }

        curr = curr * 10 + (size_t) left_most;
        left = left_most_i + 1;
    }
    return curr;
}

size_t bestTwelve(const byte* batteries, const size_t start, const size_t end) {
    return best(batteries, start, end, 12);
}

size_t part2(const Data* data) {
    const byte* batteries = data->batteries;
    const size_t rows = data->rows, cols = data->cols;

    size_t sum = 0;
    for (size_t i = 0; i < rows; i++) {
        const size_t best = bestTwelve(batteries, i*cols, (i+1)*cols);
        sum += best;
    }
    return sum;
}

int main(void) {
    const char* path = "inputs/day03.txt";
    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.batteries);
}
