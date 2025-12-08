#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

struct Rotation {
    char direction;
    size_t amount;
};

struct Data {
    struct Rotation* rotations;
    size_t n;
    bool parse_successful;
};

size_t parseInt(const char* line, const size_t pos) {
    size_t res = 0;
    for (size_t i = pos; i < strlen(line) - 1; i++) {
        res = res * 10 + (size_t) (line[i] - '0');
    }
    return res;
}

struct Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return (struct Data) { NULL, 0, false };
    }

    size_t n = 0;
    struct Rotation* rotations = malloc(sizeof(struct Rotation) * n);
    if (!rotations) {
        perror("Out of memory");
        fclose(fp);
        return (struct Data) { NULL, 0, false };
    }

    char* line = NULL;
    size_t len = 0;
    for (size_t i = 0; getline(&line, &len, fp) != -1; i++) {
        struct Rotation* new = realloc(rotations, sizeof(struct Rotation) * (n + 1));
        if (!new) {
            perror("Out of memory");
            free(line);
            fclose(fp);
            free(rotations);
            return (struct Data) { NULL, 0, false };
        }

        rotations = new;
        rotations[n++] = (struct Rotation) { line[0], parseInt(line, 1) };
    }

    free(line);
    fclose(fp);
    return (struct Data) { rotations, n, true };
}

size_t part1(const struct Data* data) {
    const struct Rotation* rotations = data->rotations;
    const size_t N = data->n;

    int curr = 50;
    size_t times_zero = 0;
    for (size_t i = 0; i < N; i++) {
        switch (rotations[i].direction) {
        case 'L':
            curr -= (int) rotations[i].amount;
            break;
        case 'R':
            curr += (int) rotations[i].amount;
            break;
        default:
            fprintf(stderr, "Unknown direction %d\n", rotations[i].direction);
            return 0;
        }

        if (curr % 100 == 0) {
            times_zero++;
        }
    }
    return times_zero;
}

size_t part2(const struct Data* data) {
    const struct Rotation* rotations = data->rotations;
    const size_t N = data->n;
    
    int curr = 50;
    size_t times_zero = 0;
    for (size_t i = 0; i < N; i++) {
        const int prev = curr;
        switch (rotations[i].direction) {
        case 'L':
            curr -= (int) rotations[i].amount;
            times_zero += (size_t) (prev - curr + 100) / 100;
            if (curr < 0) {
                curr = 100 + curr % 100;
            }
            break;
        case 'R':
            curr += (int) rotations[i].amount;
            times_zero += (size_t) (curr - prev) / 100;
            if (curr >= 100) {
                curr %= 100;
            }
            break;
        default:
            fprintf(stderr, "Unknown direction %d\n", rotations[i].direction);
            return 0;
        }
    }
    return times_zero;
}

int main(void) {
    const char* path = "inputs/day01.txt";
    const struct Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.rotations);
}
