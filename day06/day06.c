#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

typedef struct {
    size_t* numbers;
    char* signs;
    size_t rows;
    size_t cols;
    bool parse_successful;
} Data1;

typedef struct {
    char** lines;
    size_t rows;
    char* signs;
    bool parse_successful;
} Data2;

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

size_t nNumbers(const char* line) {
    size_t cols = 0;

    size_t i = 0;
    while (line[i] == ' ') {
        i++;
    }

    while (i < strlen(line) && line[i] != '\n') {
        while (isDigit(line[i])) {
            i++;
        }
        cols++;

        do {
            i++;
        } while (line[i] == ' ');
    }

    return cols;
}

size_t* parseInts(const char* line, const size_t cols) {
    size_t* numbers = malloc(sizeof(size_t) * cols);
    if (!numbers) {
        return NULL;
    }

    size_t i = 0;
    while (line[i] == ' ') {
        i++;
    }
    for (size_t j = 0; j < cols; j++) {
        const size_t start = i;
        while (isDigit(line[i])) {
            i++;
        }
        numbers[j] = parseInt(line, start, i);

        do {
            i++;
        } while (line[i] == ' ');
    }

    return numbers;
}

char* parseSigns(const char* line, const size_t cols) {
    char* signs = malloc(sizeof(char) * cols);
    if (!signs) {
        return NULL;
    }

    size_t i = 0;
    while (line[i] == ' ') {
        i++;
    }
    for (size_t j = 0; j < cols; j++) {
        signs[j] = line[i++];

        do {
            i++;
        } while (line[i] == ' ');
    }

    return signs;
}

Data1 parseFilePart1(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    char* line = NULL;
    size_t len = 0;
    if (getline(&line, &len, fp) < 0) {
    error_1:
        fclose(fp);
        goto error;
    }

    const size_t cols = nNumbers(line);
    size_t* numbers = parseInts(line, cols);
    size_t rows = 1;

    if (!numbers) {
        perror("Out of memory.");
    error_2:
        free(line);
        goto error_1;
    }

    while (getline(&line, &len, fp) > 0 && line[0] != '*' && line[0] != '+') {
        size_t* nextInts = parseInts(line, cols);
        if (!nextInts) {
            perror("Out of memory.");
        error_3:
            free(numbers);
            goto error_2;
        }
        size_t* new = realloc(numbers, sizeof(size_t) * (rows + 1) * cols);
        if (!new) {
            perror("Out of memory.");
            free(nextInts);
            goto error_3;
        }

        numbers = new;
        memcpy(numbers + rows*cols, nextInts, sizeof(size_t) * cols);
        rows++;

        free(nextInts);
    }
    fclose(fp);

    char* signs = parseSigns(line, cols);
    free(line);
    if (!signs) {
        free(numbers);
        goto error;
    }

    return (Data1) { numbers, signs, rows, cols, true };
error:
    return (Data1) { NULL, NULL, 0, 0, false };
}

size_t solveColumn1(const Data1* data, const size_t col) {
    const char sign = data->signs[col];

    size_t acc = data->numbers[col];
    for (size_t i = data->cols + col; i < data->rows*data->cols; i += data->cols) {
        switch (sign) {
            case '+':
                acc += data->numbers[i];
                break;
            case '*':
                acc *= data->numbers[i];
                break;
            default:
                fprintf(stderr, "Invalid sign encountered: %c\n", sign);
                return 0;
        }
    }

    return acc;
}

size_t part1(const char* path) {
    const Data1 data = parseFilePart1(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 0;
    }

    size_t total = 0;
    for (size_t i = 0; i < data.cols; i++) {
        total += solveColumn1(&data, i);
    }

    free(data.numbers);
    free(data.signs);

    return total;
}

bool containsSign(const char* line) {
    for (size_t i = 0; i < strlen(line); i++) {
        if (line[i] == '+' || line[i] == '*') {
            return true;
        }
    }
    return false;
}

size_t countSigns(const char* line) {
    size_t n = 0;
    for (size_t i = 0; i < strlen(line); i++) {
        if (line[i] == '+' || line[i] == '*') {
            n++;
        }
    }
    return n;
}

size_t indexNextSign(const char* line, size_t offset) {
    for (size_t i = offset + 1; i < strlen(line); i++) {
        if (line[i] == '+' || line[i] == '*') {
            return i;
        }
    }
    // last sign and the next would *would* be where '\0' is
    return strlen(line);
}

Data2 parseFilePart2(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        goto error;
    }

    size_t n_lines = 0;
    char** lines = malloc(sizeof(char*) * n_lines);
    if (!lines) {
        perror("Out of memory.");
    error_1:
        fclose(fp);
        goto error;
    }

    char* line = NULL;
    size_t chars_per_line = 0, len = 0;
    while (getline(&line, &len, fp) > 0) {
        if (containsSign(line)) {
            for (size_t curr_sign = 0, next_sign = indexNextSign(line, curr_sign); curr_sign < chars_per_line; curr_sign = next_sign, next_sign = indexNextSign(line, curr_sign)) {
                for (size_t i = curr_sign; i <= next_sign - 2; i++) {
                    for (size_t row = 0; row < n_lines; row++) {
                        if (!isDigit(lines[row][i])) {
                            lines[row][i] = '0';
                        }
                    }
                }
            }
        } else {
            if (!chars_per_line) {
                // lines are equal size anyways
                chars_per_line = strlen(line);
            }

            char** new = realloc(lines, sizeof(char*) * (n_lines + 1));
            if (!new) {
                perror("Out of memory.");
            error_2:
                for (size_t i = 0; i < n_lines; i++) {
                    free(lines[i]);
                }
                free(lines);
                free(line);
                goto error_1;
            }
            lines = new;

            char* curr_line = malloc(sizeof(char) * (chars_per_line + 1));
            if (!curr_line) {
                perror("Out of memory.");
                goto error_2;
            }
            lines[n_lines++] = curr_line;
            memcpy(curr_line, line, chars_per_line + 1);
        }
    }
    fclose(fp);

    return (Data2) { lines, n_lines, line, true };
error:
    return (Data2) { NULL, 0, NULL, false };
}

size_t arithmeticallyNeutral(const char sign) {
    switch (sign) {
        case '+':
            return 0;
        case '*':
            return 1;
        default:
            return 0;
    }
}

size_t part2(const char* path) {
    const Data2 data = parseFilePart2(path);
    if (!data.parse_successful) {
        fprintf(stderr, "Unable to parse '%s'\n", path);
        return 0;
    }

    size_t total = 0;
    for (size_t curr_sign = 0, next_sign = indexNextSign(data.signs, curr_sign); curr_sign < strlen(data.signs); curr_sign = next_sign, next_sign = indexNextSign(data.signs, curr_sign)) {
        const char sign = data.signs[curr_sign];
        size_t subtotal = arithmeticallyNeutral(sign);

        for (size_t i = curr_sign; i <= next_sign - 2; i++) {
            size_t number = 0;
            for (size_t row = 0; row < data.rows; row++) {
                const char digit = data.lines[row][i];
                if (digit != '0') {
                    number = number * 10 + (size_t) (digit - '0');
                }
            }

            switch (sign) {
                case '+':
                    subtotal += number;
                    break;
                case '*':
                    subtotal *= number;
                    break;
                default:
                    fprintf(stderr, "Invalid sign encountered: %c\n", sign);
                    total = 0;
                    goto end;
            }
        }
        total += subtotal;
    }

end:
    for (size_t i = 0; i < data.rows; i++) {
        free(data.lines[i]);
    }
    free(data.lines);
    free(data.signs);

    return total;
}

int main(void) {
    const char* path = "inputs/day06.txt";

    const size_t p1 = part1(path);
    printf("Part 1: %zu\n", p1);

    const size_t p2 = part2(path);
    printf("Part 2: %zu\n", p2);
}
