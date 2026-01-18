#include <math.h>
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
    const size_t n;
    const bool parse_successful;
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

    Vec3* boxes = NULL;
    char* line = NULL;
    size_t n = 0, len;
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
            free(line);
            fclose(fp);
            goto error;
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

typedef struct {
    const Vec3* a;
    const Vec3* b;
} Connection;

typedef struct {
    Connection* connections;
    const size_t n;
} Circuit;

typedef struct {
    const Vec3* a;
    const Vec3* b;
    double distance;
} Distance;

typedef struct {
    Distance* distances;
    size_t n;
} Distances;

typedef struct {
    const Vec3** nodes;
    size_t n;
} Vec3Array;

double straight_line_distance(const Vec3* a, const Vec3* b) {
    const double x_a = (double) a->x, x_b = (double) b->x;
    const double y_a = (double) a->y, y_b = (double) b->y;
    const double z_a = (double) a->z, z_b = (double) b->z;

    const double d_x = x_a - x_b;
    const double d_y = y_a - y_b;
    const double d_z = z_a - z_b;

    return sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

int compare(const void* a, const void* b) {
    const double a_distance = ((const Distance*) a)->distance;
    const double b_distance = ((const Distance*) b)->distance;

    if (a_distance < b_distance) return -1;
    if (a_distance > b_distance) return 1;
    return 0;
}

Distances distances(const Vec3* boxes, const size_t n) {
    const size_t n_pairwise = n * (n - 1) / 2;
    Distance* dists = malloc(sizeof(Distance) * n_pairwise);
    if (!dists) {
        perror("Out of memory.");
        return (Distances) { NULL, 0 };
    }

    size_t d = 0;
    for (size_t i = 0; i < n; i++) {
        const Vec3* box_i = &boxes[i];
        for (size_t j = i + 1; j < n; j++) {
            const Vec3* box_j = &boxes[j];
            dists[d++] = (Distance) { box_i, box_j, straight_line_distance(box_i, box_j) };
        }
    }

    qsort(dists, n_pairwise, sizeof(Distance), compare);
    return (Distances) { dists, n_pairwise };
}

Vec3Array neighbors(const Connection* edges, const size_t n_edges, const Vec3* source) {
    size_t n_neighbors = 0;
    for (size_t i = 0; i < n_edges; i++) {
        const Vec3* a = edges[i].a;
        const Vec3* b = edges[i].b;

        if (a == source || b == source) {
            n_neighbors++;
        }
    }

    if (n_neighbors == 0) {
        goto error;
    }

    const Vec3** neighbors = malloc(sizeof(const Vec3*) * n_neighbors);
    if (!neighbors) {
        perror("Out of memory.");
        goto error;
    }

    for (size_t i = 0, j = 0; i < n_edges; i++) {
        if (edges[i].a == source) {
            neighbors[j++] = edges[i].b;
        } else if (edges[i].b == source) {
            neighbors[j++] = edges[i].a;
        }
    }

    return (Vec3Array) { neighbors, n_neighbors };
error:
    return (Vec3Array) { NULL, 0 };
}

void add_to_component(Vec3Array* component, const Vec3* node) {
    const Vec3** new = realloc(component->nodes, sizeof(const Vec3*) * (component->n + 1));
    if (!new) {
        exit(1);
    }

    component->nodes = new;
    component->nodes[component->n++] = node;
}

bool contains(const Vec3Array* array, const Vec3* node) {
    for (size_t i = 0; i < array->n; i++) {
        if (array->nodes[i] == node) {
            return true;
        }
    }
    return false;
}

void findComponent(const Connection* edges, const size_t n_edges, const Vec3* source, Vec3Array* component) {
    add_to_component(component, source);

    const Vec3Array nghbrs = neighbors(edges, n_edges, source);
    if (nghbrs.n <= 0) {
        return;
    }

    for (size_t i = 0; i < nghbrs.n; i++) {
        const Vec3* candidate = nghbrs.nodes[i];
        if (!contains(component, candidate)) {
            findComponent(edges, n_edges, candidate, component);
        }
    }
    free(nghbrs.nodes);
}

bool isEqual(const Vec3Array* a, const Vec3Array* b) {
    if (a->n != b->n) {
        return false;
    }

    for (size_t i = 0; i < a->n; i++) {
        if (!contains(b, a->nodes[i])) {
            return false;
        }
    }
    return true;
}

int compareVec3ArraysAsc(const void* a, const void* b) {
    const size_t a_n = ((const Vec3Array*) a)->n;
    const size_t b_n = ((const Vec3Array*) b)->n;

    if (a_n < b_n) return 1;
    if (a_n > b_n) return -1;
    return 0;
}

void freeVec3Arrays(Vec3Array* arrays, const size_t n) {
    for (size_t i = 0; i < n; i++) {
        free(arrays[i].nodes);
    }
}

size_t part1(const Data* data, const size_t n_junctions, const size_t top_k) {
    const Distances dists = distances(data->boxes, data->n);
    if (!dists.n) {
        goto error;
    }
    Distance* d = dists.distances;

    Connection* connections = malloc(sizeof(Connection) * n_junctions);
    if (!connections) {
        perror("Out of memory.");
    error_1:
        free(d);
        goto error;
    }

    for (size_t i = 0; i < n_junctions; i++) {
        connections[i] = (Connection) { d[i].a, d[i].b };
    }
    free(d);

    Vec3Array* components = malloc(sizeof(Vec3Array) * (top_k + 1));
    if (!components) {
        perror("Out of memory.");
        free(connections);
        goto error_1;
    }

    for (size_t i = 0; i < top_k + 1; i++) {
        components[i] = (Vec3Array) { NULL, 0 };
    }

    outer: for (size_t i = 0; i < n_junctions; i++) {
        free(components[top_k].nodes);
        components[top_k] = (Vec3Array) { NULL, 0 };
        findComponent(connections, n_junctions, connections[i].a, &components[top_k]);

        for (size_t j = 0; j < top_k; j++) {
            if (isEqual(&components[top_k], &components[j])) {
                continue outer;
            }
        }
        qsort(components, top_k+1, sizeof(Vec3Array), compareVec3ArraysAsc);
    }

    size_t product = 1;
    for (size_t i = 0; i < top_k; i++) {
        product *= components[i].n;
    }

    freeVec3Arrays(components, top_k + 1);
    free(components);
    free(connections);
    return product;
error:
    return 0;
}

typedef struct {
    size_t* id;
    size_t n;
    size_t count;
} Components;

Components initComponents(const Data* data) {
    Components c = (Components) { NULL, data->n, data->n };
    c.id = malloc(sizeof(size_t) * c.n);
    if (!c.id) {
        perror("Out of memory.");
        return c;
    }

    for (size_t i = 0; i < c.n; i++) {
        c.id[i] = i;
    }
    return c;
}

bool addConnectionStep(Components* comps, const Vec3* a, const Vec3* b, const Vec3* boxes) {
    size_t ca = comps->id[(size_t) (a - boxes)];
    size_t cb = comps->id[(size_t) (b - boxes)];

    if (ca == cb) {
        return false;
    }

    for (size_t i = 0; i < comps->n; i++) {
        if (comps->id[i] == cb) {
            comps->id[i] = ca;
        }
    }

    comps->count--;
    return true;
}

size_t part2(const Data* data) {
    const Distances dists = distances(data->boxes, data->n);
    if (!dists.n) {
        return 0;
    }

    Components comps = initComponents(data);
    if (!comps.id) {
        return 0;
    }

    for (size_t i = 0; i < dists.n; i++) {
        const Distance* d = &dists.distances[i];

        if (addConnectionStep(&comps, d->a, d->b, data->boxes)) {
            if (comps.count == 1) {
                size_t result = (size_t)d->a->x * (size_t)d->b->x;
                free(comps.id);
                free(dists.distances);
                return result;
            }
        }
    }

    free(comps.id);
    free(dists.distances);
    return 0;
}

int main(void) {
    const char* path = "inputs/day08.txt";
    const size_t n_junctions = 1000;
    const size_t top_k = 3;

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data, n_junctions, top_k);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.boxes);
}
