#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    size_t start;
    size_t end_inclusive;
} Range;

typedef struct {
    Range* fresh;
    size_t n_fresh;
    size_t* ingredients;
    size_t n_ingredients;
    bool parse_successful;
} Data;

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
        return (Data) { NULL, 0, NULL, 0, false };
    }

    size_t n_ranges = 0;
    Range* ranges = malloc(sizeof(Range) * n_ranges);
    if (!ranges) {
        perror("Out of memory");
        fclose(fp);
        return (Data) { NULL, 0, NULL, 0, false };
    }
    
    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) > 0 && line[0] != '\n') {
        size_t sep_i = 0;
        while ('0' <= line[sep_i] && line[sep_i] <= '9') {
            sep_i++;
        }
        const size_t first = parseInt(line, 0, sep_i);

        const size_t start_second = ++sep_i;
        while ('0' <= line[sep_i] && line[sep_i] <= '9') {
            sep_i++;
        }
        const size_t second = parseInt(line, start_second, sep_i);
        
        Range* new = realloc(ranges, sizeof(Range) * (n_ranges + 1));
        if (!new) {
            perror("Out of memory");
            free(line);
            free(ranges);
            fclose(fp);
            return (Data) { NULL, 0, NULL, 0, false };
        }
        ranges = new;
        ranges[n_ranges++] = (Range) { first, second };
    }

    size_t n_ingredients = 0;
    size_t* ingredients = malloc(sizeof(size_t) * n_ingredients);
    if (!ingredients) {
        free(line);
        fclose(fp);
        free(ranges);
    }

    while (getline(&line, &len, fp) > 0) {
        size_t i = 0;
        while ('0' <= line[i] && line[i] <= '9') {
            i++;
        }
        const size_t ing = parseInt(line, 0, i);

        size_t* new = realloc(ingredients, sizeof(size_t) * (n_ingredients + 1));
        if (!new) {
            free(line);
            fclose(fp);
            free(ranges);
            return (Data) { NULL, 0, NULL, 0, false };
        }
        ingredients = new;
        ingredients[n_ingredients++] = ing;
    }

    free(line);
    fclose(fp);
    return (Data) { ranges, n_ranges, ingredients, n_ingredients, true };
}

bool isFresh(const Range* fresh, const size_t n, const size_t ingredient) {
    for (size_t i = 0; i < n; i++) {
        if (fresh[i].start <= ingredient && ingredient <= fresh[i].end_inclusive) {
            return true;
        }
    }
    return false;
}

size_t part1(const Data* data) {
    const Range* fresh = data->fresh;
    const size_t* ingredients = data->ingredients;
    const size_t n_fresh = data->n_fresh, n_ingredients = data->n_ingredients;

    size_t n_valid = 0;
    for (size_t i = 0; i < n_ingredients; i++) {
        if (isFresh(fresh, n_fresh, ingredients[i])) {
            n_valid++;
        }
    }

    return n_valid;
}

int compare(const void* a, const void* b) {
    const size_t a_start = ((const Range*) a)->start;
    const size_t b_start = ((const Range*) b)->start;
    
    if (a_start < b_start) return -1;
    if (a_start > b_start) return 1;
    return 0;
}

size_t part2(const Data* data) {
    const Range* fresh = data->fresh;
    const size_t n_fresh = data->n_fresh;

    Range* sorted = malloc(sizeof(Range) * n_fresh);
    if (!sorted) {
        perror("Out of memory.");
        return 0;
    }
    Range* canonical = malloc(sizeof(Range) * n_fresh);
    if (!canonical) {
        perror("Out of memory.");
        free(sorted);
        return 0;
    }
    memcpy(sorted, fresh, sizeof(Range) * n_fresh);

    qsort(sorted, n_fresh, sizeof(Range), compare);

    size_t c = 0;
    canonical[c] = sorted[c];
    for (size_t i = 1; i < n_fresh; i++) {
        const Range* curr = &sorted[i];
        Range* can = &canonical[c];

        if (can->start <= curr->start && curr->start <= can->end_inclusive) {
            // current range is fully contained already
            if (curr->end_inclusive <= can->end_inclusive) {
                continue;
            } 
            // expand current range to the right
            else {
                can->end_inclusive = curr->end_inclusive;
            }
        } else if (curr->start == can->end_inclusive + 1) {
            can->end_inclusive = curr->end_inclusive;
        } else {
            canonical[++c] = *curr;
        }
    }
    free(sorted);

    size_t total_valid = 0;
    for (size_t i = 0; i <= c; i++) {
        const Range can = canonical[i];
        total_valid += can.end_inclusive - can.start + 1;
    }
    free(canonical);

    return total_valid;
}

int main(void) {
    const char* path = "inputs/day05.txt";
    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.fresh);
    free(data.ingredients);
}
