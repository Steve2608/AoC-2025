#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    char* name;
    char** outs;
} Device;

typedef struct {
    Device* devices;
    const size_t count;
} DeviceMap;

typedef struct {
    DeviceMap device_map;
    const size_t n_devices;
    const bool parse_successful;
} Data;

void free_devices(Device* devices, const size_t count) {
    if (!devices) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        if (devices[i].name) {
            free(devices[i].name);
        }

        if (devices[i].outs) {
            for (size_t j = 0; devices[i].outs[j]; j++) {
                free(devices[i].outs[j]);
            }
            free(devices[i].outs);
        }
    }

    free(devices);
}

Data parseFile(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    Device* devices = NULL;
    size_t count = 0;

    char* line = NULL;
    size_t cap = 0;
    while (getline(&line, &cap, fp) > 0) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        char* colon = strchr(line, ':');
        if (!colon) {
            goto error;
        }

        *colon = '\0';

        char* name = strdup(line);
        if (!name) {
            goto error;
        }

        char** outs = NULL;
        size_t n_outs = 0;

        char* tok = strtok(colon + 2, " ");
        while (tok) {
            while (*tok == ' ') {
                tok++;
            }

            if (*tok) {
                char** tmp = realloc(outs, (n_outs + 2) * sizeof(*outs));
                if (!tmp) {
                    goto error;
                }
                outs = tmp;

                outs[n_outs] = strdup(tok);
                if (!outs[n_outs]) {
                    goto error;
                }

                n_outs++;
                outs[n_outs] = NULL;
            }

            tok = strtok(NULL, " ");
        }

        Device* tmp = realloc(devices, (count + 1) * sizeof(*devices));
        if (!tmp) {
            goto error;
        }
        devices = tmp;

        devices[count].name = name;
        devices[count].outs = outs;
        count++;
    }
    free(line);
    fclose(fp);

    return (Data) {
        .device_map = {
            .devices = devices,
            .count = count,
        },
        .n_devices = count,
        .parse_successful = true,
    };

error:
    free(line);
    fclose(fp);
    free_devices(devices, count);

    return (Data) {
        .device_map = {
            .devices = NULL,
            .count = 0,
        },
        .n_devices = 0,
        .parse_successful = false,
    };
}

Device* findDevice(const DeviceMap map, const char* name) {
    for (size_t i = 0; i < map.count; i++) {
        if (strcmp(map.devices[i].name, name) == 0) {
            return &map.devices[i];
        }
    }
    return NULL;
}

size_t countPaths(const DeviceMap map, const char* dev) {
    if (strcmp(dev, "out") == 0) {
        return 1;
    }

    const Device* device = findDevice(map, dev);

    size_t total = 0;
    for (size_t i = 0; device->outs[i]; i++) {
        total += countPaths(map, device->outs[i]);
    }

    return total;
}

size_t part1(const Data* data) {
    return countPaths(data->device_map, "you");
}

typedef struct {
    size_t*** table;
    const size_t n_devices;
} Memo;

Memo memo_create(const size_t n_devices) {
    size_t*** table = malloc(n_devices * sizeof(*table));
    if (!table) {
        return (Memo) { .table = NULL, .n_devices = 0 };
    }

    for (size_t i = 0; i < n_devices; i++) {
        table[i] = NULL;
    }

    for (size_t i = 0; i < n_devices; i++) {
        table[i] = malloc(2 * sizeof(**table));
        if (!table[i]) {
            goto error;
        }

        for (size_t d = 0; d < 2; d++) {
            table[i][d] = NULL;
        }

        for (size_t d = 0; d < 2; d++) {
            table[i][d] = malloc(2 * sizeof(***table));
            if (!table[i][d]) {
                goto error;
            }

            for (size_t f = 0; f < 2; f++) {
                table[i][d][f] = SIZE_MAX;
            }
        }
    }

    return (Memo) { .table = table, .n_devices = n_devices };

error:
    for (size_t i = 0; i < n_devices; i++) {
        if (table[i]) {
            for (size_t d = 0; d < 2; d++) {
                if (table[i][d]) {
                    free(table[i][d]);
                }
            }
            free(table[i]);
        }
    }

    free(table);
    return (Memo) { .table = NULL, .n_devices = 0 };
}

void free_memo(Memo* memo) {
    for (size_t i = 0; i < memo->n_devices; i++) {
        for (size_t d = 0; d < 2; d++) {
            free(memo->table[i][d]);
        }
        free(memo->table[i]);
    }

    free(memo->table);
}

size_t countPaths_memo(
    const DeviceMap map,
    const char* dev,
    bool seen_dac,
    bool seen_fft,
    Memo* memo
) {
    if (strcmp(dev, "dac") == 0) {
        seen_dac = true;
    }

    if (strcmp(dev, "fft") == 0) {
        seen_fft = true;
    }

    if (strcmp(dev, "out") == 0) {
        if (seen_dac && seen_fft) {
            return 1;
        }
        return 0;
    }

    const Device* device = findDevice(map, dev);

    size_t* cached = &memo->table[(size_t) (device - map.devices)][seen_dac][seen_fft];
    if (*cached != SIZE_MAX) {
        return *cached;
    }

    size_t total = 0;
    for (size_t i = 0; device->outs[i]; i++) {
        total += countPaths_memo(map, device->outs[i], seen_dac, seen_fft, memo);
    }

    *cached = total;
    return total;
}

size_t part2(const Data* data) {
    Memo memo = memo_create(data->n_devices);

    const size_t result = countPaths_memo(data->device_map, "svr", false, false, &memo);

    free_memo(&memo);
    return result;
}

int main(void) {
    const char* path = "inputs/day11.txt";

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free_devices(data.device_map.devices, data.n_devices);
}
