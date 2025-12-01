#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/errno.h>

#define ROT_LEN 4239
// #define ROT_LEN 10

struct Rotation {
    char direction;
    int amount;
};

int parseInt(const char* line, const int pos) {
    int res = 0;
    for (int i = pos; i < strlen(line) - 1; i++) {
        res = res * 10 + line[i] - '0';
    }
    return res;
}

struct Rotation* parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return NULL;
    }

    struct Rotation* rotations = malloc(sizeof(struct Rotation) * ROT_LEN);
    if (!rotations) {
        perror("Out of memory");
        free(fp);
        return NULL;
    }

    char* line = NULL;
    size_t len = 0;
    for (int i = 0; i < ROT_LEN && getline(&line, &len, fp) != -1; i++) {
        if (strlen(line) == 0) {
            continue;
        }

        rotations[i].direction = line[0];
        rotations[i].amount = parseInt(line, 1);
    }

    free(line);
    fclose(fp);
    return rotations;
}

int part1(const struct Rotation* rotations) {
    // dial starts pointing at 50 because reasons
    int curr = 50, times_zero = 0;
    for (int i = 0; i < ROT_LEN; i++) {
        switch (rotations[i].direction) {
        case 'L':
            curr -= rotations[i].amount;
            break;
        case 'R':
            curr += rotations[i].amount;
            break;
        default:
            fprintf(stderr, "Unknown direction %d\n", rotations[i].direction);
            return -1;
        }

        if (curr % 100 == 0) {
            times_zero++;
        }
    }
    return times_zero;
}

int part2(const struct Rotation* rotations) {
    int curr = 50, times_zero = 0;
    for (int i = 0; i < ROT_LEN; i++) {
        const int prev = curr;
        switch (rotations[i].direction) {
        case 'L':
            for (int j = 0; j < rotations[i].amount; j++) {
                curr--;
                if (curr % 100 == 0) {
                    times_zero++;
                }
            }
            break;
        case 'R':
            for (int j = 0; j < rotations[i].amount; j++) {
                curr++;
                if (curr % 100 == 0) {
                    times_zero++;
                }
            }
            break;
        default:
            fprintf(stderr, "Unknown direction %d\n", rotations[i].direction);
            return -1;
        }
    }
    return times_zero;
}

int main(void) {
    const char* path = "inputs/day01.txt";
    struct Rotation* rotations = parseFile(path);
    if (rotations == NULL) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const int p1 = part1(rotations);
    if (p1 < 0) {
        goto end;
    }
    printf("Part 1: %d\n", p1);

    const int p2 = part2(rotations);
    if (p2 < 0) {
        goto end;
    }
    printf("Part 2: %d\n", p2);

end:
    free(rotations);
}
