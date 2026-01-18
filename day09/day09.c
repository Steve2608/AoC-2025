#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    long x;
    long y;
} Vec2;

typedef struct {
    Vec2* tiles;
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

    Vec2* tiles = NULL;
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

        Vec2* new = realloc(tiles, sizeof(Vec2) * (n + 1));
        if (!new) {
            perror("Out of memory.");
            free(tiles);
            free(line);
            fclose(fp);
            goto error;
        }

        tiles = new;
        tiles[n++] = (Vec2) { x, y };
    }
    free(line);
    fclose(fp);

    return (Data) { tiles, n, true };
error:
    return (Data) { NULL, 0, false };
}

size_t tile_area(const Vec2* a, const Vec2* b) {
    const long x_a = a->x, y_a = a->y;
    const long x_b = b->x, y_b = b->y;

    const size_t x_distance = (size_t) labs(x_a - x_b);
    const size_t y_distance = (size_t) labs(y_a - y_b);

    return (x_distance + 1) * (y_distance + 1);
}

size_t part1(const Data* data) {
    size_t max_area = 0;

    for (size_t i = 0; i < data->n; i++) {
        const Vec2 a = data->tiles[i];
        for (size_t j = i + 1; j < data->n; j++) {
            const Vec2 b = data->tiles[j];

            const size_t area = tile_area(&a, &b);
            if (area > max_area) {
                max_area = area;
            }
        }
    }
    return max_area;
}

typedef struct {
    long x1;
    long y1;
    long x2;
    long y2;
} Edge;

Edge* build_edges(const Data* data) {
    Edge* edges = malloc(sizeof(Edge) * data->n);
    if (!edges) {
        return NULL;
    }

    for (size_t i = 0; i < data->n; i++) {
        const Vec2 a = data->tiles[i];
        const Vec2 b = data->tiles[(i + 1) % data->n];
        edges[i] = (Edge) { a.x, a.y, b.x, b.y };
    }

    return edges;
}

bool point_inside(const Edge* edges, const size_t n, const long x, const long y) {
    int crossings = 0;

    for (size_t i = 0; i < n; i++) {
        const Edge e = edges[i];

        if (e.x1 == e.x2) {
            const long ex = e.x1;
            const long y1 = e.y1 < e.y2 ? e.y1 : e.y2;
            const long y2 = e.y1 < e.y2 ? e.y2 : e.y1;

            if (x == ex && y >= y1 && y <= y2) {
                return true;
            }

            if (y >= y1 && y < y2 && ex > x) {
                crossings++;
            }
        } else {
            const long ey = e.y1;
            const long x1 = e.x1 < e.x2 ? e.x1 : e.x2;
            const long x2 = e.x1 < e.x2 ? e.x2 : e.x1;

            if (y == ey && x >= x1 && x <= x2) {
                return true;
            }
        }
    }

    return crossings % 2 == 1;
}

bool edge_intersects_rect(const Edge* e, const long minx, const long maxx, const long miny, const long maxy) {
    if (e->x1 == e->x2) {
        const long x = e->x1;
        if (x <= minx || x >= maxx) {
            return false;
        }

        const long y1 = e->y1 < e->y2 ? e->y1 : e->y2;
        const long y2 = e->y1 < e->y2 ? e->y2 : e->y1;

        return !(y2 <= miny || y1 >= maxy);
    } else {
        const long y = e->y1;
        if (y <= miny || y >= maxy) {
            return false;
        }

        const long x1 = e->x1 < e->x2 ? e->x1 : e->x2;
        const long x2 = e->x1 < e->x2 ? e->x2 : e->x1;

        return !(x2 <= minx || x1 >= maxx);
    }
}

size_t part2(const Data* data) {
    Edge* edges = build_edges(data);
    if (!edges) {
        return 0;
    }

    size_t best = 0;
    for (size_t i = 0; i < data->n; i++) {
        const Vec2 a = data->tiles[i];

        inner: for (size_t j = i + 1; j < data->n; j++) {
            const Vec2 b = data->tiles[j];

            const long minx = a.x < b.x ? a.x : b.x;
            const long maxx = a.x > b.x ? a.x : b.x;
            const long miny = a.y < b.y ? a.y : b.y;
            const long maxy = a.y > b.y ? a.y : b.y;

            if (!point_inside(edges, data->n, minx, miny)) {
                continue;
            }
            if (!point_inside(edges, data->n, minx, maxy)) {
                continue;
            }
            if (!point_inside(edges, data->n, maxx, miny)) {
                continue;
            }
            if (!point_inside(edges, data->n, maxx, maxy)) {
                continue;
            }

            for (size_t k = 0; k < data->n; k++) {
                if (edge_intersects_rect(&edges[k], minx, maxx, miny, maxy)) {
                    continue inner;
                }
            }

            const size_t area = (size_t) (maxx - minx + 1) * (size_t) (maxy - miny + 1);
            if (area > best) {
                best = area;
            }
        }
    }

    free(edges);
    return best;
}

int main(void) {
    const char* path = "inputs/day09.txt";

    const Data data = parseFile(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 1;
    }

    const size_t p1 = part1(&data);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(&data);
    printf("Part 2: %zu\n", p2);

    free(data.tiles);
}
