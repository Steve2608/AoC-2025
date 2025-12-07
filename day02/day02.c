#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/errno.h>

struct Range {
    size_t start;
    size_t end;
};

struct Data {
    struct Range* ranges;
    size_t n;
    bool parse_successful;
};

size_t parseInt(const char* line, const size_t start, const size_t end) {
    size_t res = 0;
    for (size_t i = start; i < end; i++) {
        res = res * 10 + line[i] - '0';
    }
    return res;
}

struct Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return (struct Data) { NULL, 0, false };
    }

    char* line = NULL;
    size_t len = 0;
    if (getline(&line, &len, fp) < 0) {
        fclose(fp);
        return (struct Data) { NULL, 0, false };
    }
    fclose(fp);

    size_t n = 0;
    struct Range* ranges = malloc(sizeof(struct Range) * n);
    if (!ranges) {
        perror("Out of memory");
        fclose(fp);
        return (struct Data) { NULL, 0, false };
    }
    
    for (size_t i = 0; i < strlen(line); ) {
        size_t start = i;
        while ('0' <= line[i] && line[i] <= '9') {
            i++;
        }

        const size_t first = parseInt(line, start, i);

        start = ++i;
        while ('0' <= line[i] && line[i] <= '9') {
            i++;
        }
        const size_t second = parseInt(line, start, i);

        struct Range* new = realloc(ranges, sizeof(struct Range) * (n + 1));
        if (!new) {
            free(line);
            free(ranges);
            return (struct Data) { NULL, 0, false };
        }

        ranges = new;
        ranges[n++] = (struct Range) { first, second };
        
        // skip ","
        i++;
    }

    free(line);
    return (struct Data) { ranges, n, true };
}

bool isSelfRepeatingOnce(const size_t val) {
    char num_str[32];
    sprintf(num_str, "%zu", val);
    
    const size_t len = strlen(num_str);
    if (len % 2 != 0) {
        return false;
    }
    
    size_t half = len / 2;
    return memcmp(num_str, num_str + half, half) == 0; 
}

size_t part1(const struct Data* data) {
    const struct Range* ranges = data->ranges;
    const size_t N = data->n;

    size_t sum = 0;
    for (size_t i = 0; i < N; i++) {
        const struct Range r = ranges[i];
        for (size_t j = r.start; j <= r.end; j++) {
            if (isSelfRepeatingOnce(j)) {
                sum += j;
            }
        }
    }
    return sum;
}

bool isSelfRepeating(const size_t val) {
    char num_str[32];
    sprintf(num_str, "%zu", val);
    
    const size_t len = strlen(num_str);
    for (size_t segment_len = 1; segment_len <= len / 2; segment_len++) {
        if (len % segment_len != 0) {
            continue;
        }
    
        for (size_t j = 0; j < len / segment_len - 1; j++) {
            if (memcmp(num_str + j * segment_len, num_str + (j + 1) * segment_len, segment_len) != 0) {
                goto fail;
            }
        }
        return true;

    fail:
        continue;
    }

    return false;
}

size_t part2(const struct Data* data) {
    const struct Range* ranges = data->ranges;
    const size_t N = data->n;

    size_t sum = 0;
    for (size_t i = 0; i < N; i++) {
        const struct Range r = ranges[i];
        for (size_t j = r.start; j <= r.end; j++) {
            if (isSelfRepeating(j)) {
                sum += j;
            }
        }
    }
    return sum;
}

int main(void) {
    const char* path = "inputs/day02_sample.txt";
    const struct Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    if (p1 < 0) {
        goto end;
    }
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    if (p2 < 0) {
        goto end;
    }
    printf("Part 2: %zu\n", p2);

end:
    free(data.ranges);
}
