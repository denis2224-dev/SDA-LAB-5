#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 50


typedef struct {
    int day;
    char month[MAX_CHAR];
    int year;
} Date;

typedef struct {
    char owner_name[MAX_CHAR];
    char owner_surname[MAX_CHAR];
    char product_name[MAX_CHAR];
    char manufacturer[MAX_CHAR];
    Date d;
    float wholesale_price;
    float unit_price;
    int quantity;
} Info;


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






    return 0;
}