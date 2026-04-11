#include <stdio.h>
#include <stdlib.h>

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


void analyze_text(const char *text, int *declarative_count, int *ellipsis_count) {
    int i = 0;

    *declarative_count = 0;
    *ellipsis_count = 0;

    while (text[i] != '\0') {
        if (text[i] == '.' && text[i + 1] == '.' && text[i + 2] == '.') {
            (*ellipsis_count)++;
            i += 3;
        } else if (text[i] == '.') {
            (*declarative_count)++;
            i++;
        } else {
            i++;
        }
    }
}



int main(void) {
    char *text = NULL;
    char *file_text = NULL;
    int declarative_count = 0;
    int ellipsis_count = 0;

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

    printf("\nText read from file:\n%s\n", file_text);

    analyze_text(file_text, &declarative_count, &ellipsis_count);

    printf("\nSentence detection result:\n");
    printf("Sentences ending with '.': %d\n", declarative_count);
    printf("Sentences ending with '...': %d\n", ellipsis_count);

    free(text);
    free(file_text);

    return 0;
}