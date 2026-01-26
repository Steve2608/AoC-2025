#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t bitmask_t;

typedef struct {
    bitmask_t target;
    bitmask_t* masks;
    size_t n_buttons;
    size_t n_masks;
    size_t* joltages;
} Machine;

typedef struct {
    Machine* machines;
    const size_t n;
    const bool parse_successful;
} Data;

void free_machine(Machine* m) {
    free(m->masks);
    free(m->joltages);
}

size_t count_char(const char *s, const char c) {
    size_t count = 0;
    for (size_t i = 0; s[i]; i++) {
        if (s[i] == c) {
            count++;
        }
    }
    return count;
}

bool is_digit(const char c) {
    return '0' <= c && c <= '9';
}

Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    Machine* machines = NULL;
    char* line = NULL;
    size_t n_machines = 0, len;
    while (getline(&line, &len, fp) > 0) {
        const size_t start = 1;
        size_t i = start;
        bitmask_t target = 0;
        while (line[i] != ']') {
            target <<= 1;
            if (line[i] == '#') {
                target |= 1;
            }
            i++;
        }
        const size_t n_buttons = i - start;

        // skip ']' and ' ' and '('
        i += 3;

        const size_t n_masks = count_char(line, '(');
        bitmask_t* masks = malloc(n_masks * sizeof(bitmask_t));
        if (!masks) {
            perror("Out of memory.");
            goto error;
        }
        for (size_t j = 0; j < n_masks; j++) {
            bitmask_t mask = 0;
            while (line[i] != ')') {
                if (line[i] == ',') {
                    i++;
                }

                // 0 means the first light
                mask |= (bitmask_t) (1 << (n_buttons - 1 - (size_t) (line[i] - '0')));
                i++;
            }
            masks[j] = mask;

            // skip ')' and ' ' and '('
            i += 3;
        }
        // skip '{'
        i++;

        size_t* joltages = malloc(n_buttons * sizeof(size_t));
        if (!joltages) {
            perror("Out of memory.");
        error_1:
            free(masks);
            goto error;
        }
        for (size_t j = 0; j < n_buttons; j++) {
            joltages[j] = 0;
            while (is_digit(line[i])) {
                joltages[j] *= 10;
                joltages[j] += (size_t) (line[i] - '0');
                i++;
            }
            // skip ','
            i++;
        }

        Machine* new = realloc(machines, (n_machines + 1) * sizeof(Machine));
        if (!new) {
            perror("Out of memory.");
            free(joltages);
            goto error_1;
        }
        machines = new;

        machines[n_machines++] = (Machine) {
            .target = target,
            .n_buttons = n_buttons,
            .masks = masks,
            .n_masks = n_masks,
            .joltages = joltages,
        };
    }
    free(line);
    fclose(fp);

    return (Data) {
        .machines = machines,
        .n = n_machines,
        .parse_successful = true,
    };

error:
    free(line);
    fclose(fp);

    for (size_t i = 0; i < n_machines; i++) {
        free_machine(&machines[i]);
    }
    free(machines);

    return (Data) {
        .machines = NULL,
        .n = 0,
        .parse_successful = false,
    };
}

size_t popcount(const size_t state, const size_t n_buttons) {
    size_t count = 0;
    for (size_t i = 0; i < n_buttons; i++) {
        if ((state >> i) & 1) {
            count++;
        }
    }
    return count;
}

size_t solve_machine_bruteforce(const Machine *m) {
    const size_t M = m->n_masks;
    const bitmask_t target = m->target;

    size_t best = (size_t) -1;
    for (size_t subset = 0; subset < (size_t) (1 << M); subset++) {
        bitmask_t state = 0;

        for (size_t i = 0; i < M; i++) {
            if (subset & (1 << i)) {
                state ^= m->masks[i];
            }
        }

        if (state == target) {
            size_t presses = popcount(subset, m->n_masks);
            if (presses < best) {
                best = presses;
            }
        }
    }

    return best;
}

size_t part1(const Data* data) {
    size_t total = 0;
    for (size_t i = 0; i < data->n; i++) {
        total += solve_machine_bruteforce(&data->machines[i]);
    }

    return total;
}

size_t part2(const Data* data) {
    return data->n ^ data->n;
}

int main(void) {
    const char* path = "inputs/day10.txt";

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    for (size_t i = 0; i < data.n; i++) {
        free_machine(&data.machines[i]);
    }
    free(data.machines);
}
