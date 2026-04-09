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

int main(void) {
    char *text = NULL;

    printf("Enter a text line:\n");
    text = read_line();

    if (text == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    printf("\nYou entered:\n%s\n", text);

    free(text);

    return 0;
}