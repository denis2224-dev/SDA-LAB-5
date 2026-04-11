#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char *read_line(void) {
    size_t capacity = 16;
    size_t length = 0;
    int ch;
    char *buffer = malloc(capacity * sizeof(char));
    char *temp;

    if (buffer == NULL) {
        return NULL;
    }

    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            temp = realloc(buffer, capacity * sizeof(char));
            if (temp == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }

        buffer[length] = (char)ch;
        length++;
    }

    buffer[length] = '\0';
    return buffer;
}


int write_text_file(const char *filename, const char *text) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        return 0;
    }

    fputs(text, file);
    fclose(file);
    return 1;
}


char *read_text_file(const char *filename) {
    FILE *file;
    char *buffer;

    file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    buffer = malloc(1000 * sizeof(char));
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    if (fgets(buffer, 1000, file) == NULL) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}


size_t trimmed_length(const char *start, const char *end) {
    while (start <= end && isspace((unsigned char)*start)) {
        start++;
    }

    while (end >= start && isspace((unsigned char)*end)) {
        end--;
    }

    if (start > end) {
        return 0;
    }

    return (size_t)(end - start + 1);
}


void analyze_text(const char *text, int *declarative_count, int *ellipsis_count,
                  int *difference, size_t *second_ellipsis_length, int *has_second) {
    const char *p = text;
    const char *sentence_start = text;
    int ellipsis_index = 0;

    *declarative_count = 0;
    *ellipsis_count = 0;
    *difference = 0;
    *second_ellipsis_length = 0;
    *has_second = 0;

    while (*p != '\0') {
        if (p[0] == '.' && p[1] == '.' && p[2] == '.') {
            size_t len = trimmed_length(sentence_start, p + 2);

            if (len > 0) {
                (*ellipsis_count)++;
                ellipsis_index++;

                if (ellipsis_index == 2) {
                    *second_ellipsis_length = len;
                    *has_second = 1;
                }
            }

            p += 3;
            while (*p != '\0' && isspace((unsigned char)*p)) {
                p++;
            }
            sentence_start = p;
        } else if (*p == '.') {
            size_t len = trimmed_length(sentence_start, p);

            if (len > 0) {
                (*declarative_count)++;
            }

            p++;
            while (*p != '\0' && isspace((unsigned char)*p)) {
                p++;
            }
            sentence_start = p;
        } else if (*p == '!' || *p == '?') {
            p++;
            while (*p != '\0' && isspace((unsigned char)*p)) {
                p++;
            }
            sentence_start = p;
        } else {
            p++;
        }
    }

    *difference = *declarative_count - *ellipsis_count;
}


int save_result(const char *filename, int declarative_count, int ellipsis_count,
                int difference, size_t second_ellipsis_length, int has_second) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        return 0;
    }

    fprintf(file, "Analysis result:\n");
    fprintf(file, "Declarative sentences: %d\n", declarative_count);
    fprintf(file, "Sentences ending with '...': %d\n", ellipsis_count);
    fprintf(file, "Difference: %d\n", difference);

    if (has_second) {
        fprintf(file, "Length of the 2nd sentence ending with '...': %zu\n",
                second_ellipsis_length);
    } else {
        fprintf(file, "There is no second sentence ending with '...'\n");
    }

    fclose(file);
    return 1;
}


int main(void) {
    char *text = NULL;
    char *file_text = NULL;
    int declarative_count = 0;
    int ellipsis_count = 0;
    int difference = 0;
    int has_second = 0;
    size_t second_ellipsis_length = 0;

    printf("Enter a text line:\n");
    text = read_line();

    if (text == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    if (!write_text_file("input.txt", text)) {
        fprintf(stderr, "Could not write to input.txt\n");
        free(text);
        return 1;
    }

    file_text = read_text_file("input.txt");
    if (file_text == NULL) {
        fprintf(stderr, "Could not read from input.txt\n");
        free(text);
        return 1;
    }

    analyze_text(file_text, &declarative_count, &ellipsis_count,
                 &difference, &second_ellipsis_length, &has_second);

    printf("\nAnalysis result:\n");
    printf("Declarative sentences: %d\n", declarative_count);
    printf("Sentences ending with '...': %d\n", ellipsis_count);
    printf("Difference: %d\n", difference);

    if (has_second) {
        printf("Length of the 2nd sentence ending with '...': %zu\n",
               second_ellipsis_length);
    } else {
        printf("There is no second sentence ending with '...'\n");
    }

    if (!save_result("output.txt", declarative_count, ellipsis_count,
                     difference, second_ellipsis_length, has_second)) {
        fprintf(stderr, "Could not write to output.txt\n");
        free(text);
        free(file_text);
        return 1;
    }

    printf("Results were written to output.txt\n");

    free(text);
    free(file_text);

    return 0;
}