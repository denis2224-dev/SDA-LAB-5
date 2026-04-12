#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char *read_line(void) {
    size_t capacity = 16; // initial buffer size
    size_t length = 0; // current text length
    int ch; // current character
    char *buffer = (char *)malloc(capacity * sizeof(char)); // dynamic buffer
    char *temp; // temporary pointer for realloc

    if (buffer == NULL) {
        return NULL; // allocation failed
    }

    while ((ch = getchar()) != '\n' && ch != EOF) { // read until Enter or EOF
        if (length + 1 >= capacity) { // need more space
            capacity *= 2; // double buffer size
            temp = (char *)realloc(buffer, capacity * sizeof(char));
            if (temp == NULL) {
                free(buffer); // free old memory on failure
                return NULL;
            }
            buffer = temp; // update buffer pointer
        }

        *(buffer + length) = (char)ch; // save character
        length++; // move to next position
    }

    *(buffer + length) = '\0'; // end string
    return buffer; // return dynamic string
}


int write_text_file(const char *filename, const char *text) {
    FILE *file = fopen(filename, "w"); // open file for writing

    if (file == NULL) {
        return 0; // file open failed
    }

    fputs(text, file); // write text into file
    fclose(file); // close file
    return 1; // success
}


char *read_text_file(const char *filename) {
    FILE *file;
    long size; // file size
    char *buffer; // buffer for file content
    size_t bytes_read; // number of bytes read

    file = fopen(filename, "r"); // open file for reading
    if (file == NULL) {
        return NULL; // file open failed
    }

    if (fseek(file, 0, SEEK_END) != 0) { // move to end of file
        fclose(file);
        return NULL;
    }

    size = ftell(file); // get file size
    if (size < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file); // go back to beginning

    buffer = malloc((size_t)size + 1); // allocate exact memory
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)size, file); // read whole file
    buffer[bytes_read] = '\0'; // make it a string

    fclose(file); // close file
    return buffer; // return file content
}


size_t trimmed_length(const char *start, const char *end) {
    while (start <= end && isspace((unsigned char)*start)) { // skip leading spaces
        start++;
    }

    while (end >= start && isspace((unsigned char)*end)) { // skip trailing spaces
        end--;
    }

    if (start > end) {
        return 0; // empty text
    }

    return (size_t)(end - start + 1); // return trimmed length
}


void analyze_text(const char *text, int *declarative_count, int *ellipsis_count,
                  int *difference, size_t *second_ellipsis_length, int *has_second) {
    const char *p = text; // current position
    const char *sentence_start = text; // start of current sentence
    int ellipsis_index = 0; // counts ellipsis sentences

    *declarative_count = 0; // reset declarative count
    *ellipsis_count = 0; // reset ellipsis count
    *difference = 0; // reset difference
    *second_ellipsis_length = 0; // reset second ellipsis length
    *has_second = 0; // assume second ellipsis sentence does not exist

    while (*p != '\0') {
        if (p[0] == '.' && p[1] == '.' && p[2] == '.') { // found "..."
            size_t len = trimmed_length(sentence_start, p + 2); // sentence length

            if (len > 0) {
                (*ellipsis_count)++; // count ellipsis sentence
                ellipsis_index++; // increment ellipsis index

                if (ellipsis_index == 2) {
                    *second_ellipsis_length = len; // save second ellipsis length
                    *has_second = 1; // second ellipsis exists
                }
            }

            p += 3; // skip "..."
            while (*p != '\0' && isspace((unsigned char)*p)) { // skip spaces after sentence
                p++;
            }
            sentence_start = p; // next sentence starts here
        } else if (*p == '.') { // found normal period
            size_t len = trimmed_length(sentence_start, p); // sentence length

            if (len > 0) {
                (*declarative_count)++; // count declarative sentence
            }

            p++; // move past period
            while (*p != '\0' && isspace((unsigned char)*p)) { // skip spaces
                p++;
            }
            sentence_start = p; // next sentence starts here
        } else if (*p == '!' || *p == '?') { // other sentence endings
            p++; // move past punctuation
            while (*p != '\0' && isspace((unsigned char)*p)) { // skip spaces
                p++;
            }
            sentence_start = p; // next sentence starts here
        } else {
            p++; // continue scanning
        }
    }

    *difference = *declarative_count - *ellipsis_count; // compute difference
}


int save_result(const char *filename, int declarative_count, int ellipsis_count,
                int difference, size_t second_ellipsis_length, int has_second) {
    FILE *file = fopen(filename, "w"); // open output file

    if (file == NULL) {
        return 0; // file open failed
    }

    fprintf(file, "Analysis result:\n"); // title
    fprintf(file, "Declarative sentences: %d\n", declarative_count); // save declarative count
    fprintf(file, "Sentences ending with '...': %d\n", ellipsis_count); // save ellipsis count
    fprintf(file, "Difference: %d\n", difference); // save difference

    if (has_second) {
        fprintf(file, "Length of the 2nd sentence ending with '...': %zu\n",
                second_ellipsis_length); // save second ellipsis length
    } else {
        fprintf(file, "There is no second sentence ending with '...'\n"); // no second ellipsis
    }

    fclose(file); // close file
    return 1; // success
}


int main(void) {
    char *text = NULL; // input text
    char *file_text = NULL; // text read from file
    int declarative_count = 0; // number of declarative sentences
    int ellipsis_count = 0; // number of ellipsis sentences
    int difference = 0; // difference between counts
    int has_second = 0; // flag for second ellipsis sentence
    size_t second_ellipsis_length = 0; // length of second ellipsis sentence

    printf("Enter a text line:\n");
    text = read_line(); // read text from keyboard

    if (text == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1; // stop if input allocation failed
    }

    if (!write_text_file("input.txt", text)) {
        fprintf(stderr, "Could not write to input.txt\n");
        free(text); // free memory before exit
        return 1;
    }

    file_text = read_text_file("input.txt"); // read text back from file
    if (file_text == NULL) {
        fprintf(stderr, "Could not read from input.txt\n");
        free(text); // free input text
        return 1;
    }

    analyze_text(file_text, &declarative_count, &ellipsis_count,
                 &difference, &second_ellipsis_length, &has_second); // analyze text

    printf("\nAnalysis result:\n");
    printf("Declarative sentences: %d\n", declarative_count); // show declarative count
    printf("Sentences ending with '...': %d\n", ellipsis_count); // show ellipsis count
    printf("Difference: %d\n", difference); // show difference

    if (has_second) {
        printf("Length of the 2nd sentence ending with '...': %zu\n",
               second_ellipsis_length); // show second ellipsis length
    } else {
        printf("There is no second sentence ending with '...'\n"); // no second ellipsis
    }

    if (!save_result("output.txt", declarative_count, ellipsis_count,
                     difference, second_ellipsis_length, has_second)) {
        fprintf(stderr, "Could not write to output.txt\n");
        free(text);
        free(file_text);
        return 1;
    }

    printf("Results were written to output.txt\n"); // confirmation message

    free(text); // free input text
    free(file_text); // free file text

    return 0; // successful end
}