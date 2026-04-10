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
    FILE *file = fopen(filename, "r");
    long size;
    char *buffer;
    size_t bytes_read;

    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}


int main(void) {
    char *text = NULL;

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

    printf("\nText was written to input.txt successfully.\n");

    free(text);

    return 0;
}