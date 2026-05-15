#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define DATA_DIR_NAME "data"
#define INPUT_FILENAME "input.txt"
#define OUTPUT_FILENAME "output.txt"
#define PATH_BUFFER_SIZE 1024

typedef enum {
    ENDING_NONE,
    ENDING_DECLARATIVE,
    ENDING_ELLIPSIS,
    ENDING_OTHER
} SentenceEndingType;

typedef struct {
    const char *sequence;
    SentenceEndingType type;
} SentenceEnding;

static const SentenceEnding SENTENCE_ENDINGS[] = {
    /* Longer alternatives must stay before their shorter prefixes. */
    {"......", ENDING_ELLIPSIS},                                      // six ASCII dots
    {"...", ENDING_ELLIPSIS},                                         // three ASCII dots
    {"\xE2\x80\xA6\xE2\x80\xA6", ENDING_ELLIPSIS},                    // U+2026 U+2026
    {"\xE2\x80\xA6", ENDING_ELLIPSIS},                                // U+2026
    {"\xE2\x8B\xAF", ENDING_ELLIPSIS},                                // U+22EF
    {"\xEF\xBC\x8E\xEF\xBC\x8E\xEF\xBC\x8E", ENDING_ELLIPSIS},        // U+FF0E repeated

    {".", ENDING_DECLARATIVE},
    {"\xE3\x80\x82", ENDING_DECLARATIVE},                             // U+3002
    {"\xEF\xBC\x8E", ENDING_DECLARATIVE},                             // U+FF0E
    {"\xEF\xBD\xA1", ENDING_DECLARATIVE},                             // U+FF61
    {"\xD6\x89", ENDING_DECLARATIVE},                                  // U+0589
    {"\xDB\x94", ENDING_DECLARATIVE},                                  // U+06D4
    {"\xE0\xA5\xA4", ENDING_DECLARATIVE},                             // U+0964
    {"\xE1\x8D\xA2", ENDING_DECLARATIVE},                             // U+1362

    {"!", ENDING_OTHER},
    {"?", ENDING_OTHER},
    {"\xEF\xBC\x81", ENDING_OTHER},                                   // U+FF01
    {"\xEF\xBC\x9F", ENDING_OTHER},                                   // U+FF1F
    {"\xD8\x9F", ENDING_OTHER}                                        // U+061F
};

static int starts_with_sequence(const char *text, const char *sequence, size_t *sequence_length) {
    const char *p = text; // current text byte
    const char *s = sequence; // current sequence byte

    while (*s != '\0') {
        if (*p != *s) {
            return 0; // mismatch
        }

        p++;
        s++;
    }

    *sequence_length = (size_t)(s - sequence);
    return 1; // full sequence matched
}

static SentenceEndingType find_sentence_ending(const char *text, size_t *ending_length) {
    size_t i; // ending table index

    for (i = 0; i < sizeof(SENTENCE_ENDINGS) / sizeof(SENTENCE_ENDINGS[0]); i++) {
        if (starts_with_sequence(text, SENTENCE_ENDINGS[i].sequence, ending_length)) {
            return SENTENCE_ENDINGS[i].type;
        }
    }

    *ending_length = 0;
    return ENDING_NONE;
}

static size_t utf8_char_length(const char *text) {
    unsigned char ch = (unsigned char)*text; // first byte of character
    size_t expected_length; // expected UTF-8 character length
    size_t i; // continuation byte index

    if ((ch & 0x80u) == 0) {
        return 1; // ASCII
    }
    if ((ch & 0xE0u) == 0xC0u) {
        expected_length = 2; // two-byte UTF-8 character
    } else if ((ch & 0xF0u) == 0xE0u) {
        expected_length = 3; // three-byte UTF-8 character
    } else if ((ch & 0xF8u) == 0xF0u) {
        expected_length = 4; // four-byte UTF-8 character
    } else {
        return 1; // invalid or continuation byte
    }

    for (i = 1; i < expected_length; i++) {
        ch = (unsigned char)*(text + i);
        if (ch == '\0' || (ch & 0xC0u) != 0x80u) {
            return 1; // invalid or incomplete UTF-8 character
        }
    }

    return expected_length;
}

static int is_sequence(const char *text, size_t text_length, const char *sequence) {
    size_t sequence_length = 0; // length of matched sequence

    return starts_with_sequence(text, sequence, &sequence_length) && sequence_length == text_length;
}

static int is_utf8_space(const char *text, size_t text_length) {
    if (text_length == 1) {
        return isspace((unsigned char)*text);
    }

    return is_sequence(text, text_length, "\xC2\xA0") || // U+00A0
           is_sequence(text, text_length, "\xE1\x9A\x80") || // U+1680
           is_sequence(text, text_length, "\xE2\x80\x80") || // U+2000
           is_sequence(text, text_length, "\xE2\x80\x81") || // U+2001
           is_sequence(text, text_length, "\xE2\x80\x82") || // U+2002
           is_sequence(text, text_length, "\xE2\x80\x83") || // U+2003
           is_sequence(text, text_length, "\xE2\x80\x84") || // U+2004
           is_sequence(text, text_length, "\xE2\x80\x85") || // U+2005
           is_sequence(text, text_length, "\xE2\x80\x86") || // U+2006
           is_sequence(text, text_length, "\xE2\x80\x87") || // U+2007
           is_sequence(text, text_length, "\xE2\x80\x88") || // U+2008
           is_sequence(text, text_length, "\xE2\x80\x89") || // U+2009
           is_sequence(text, text_length, "\xE2\x80\x8A") || // U+200A
           is_sequence(text, text_length, "\xE2\x80\xA8") || // U+2028
           is_sequence(text, text_length, "\xE2\x80\xA9") || // U+2029
           is_sequence(text, text_length, "\xE2\x80\xAF") || // U+202F
           is_sequence(text, text_length, "\xE2\x81\x9F") || // U+205F
           is_sequence(text, text_length, "\xE3\x80\x80");   // U+3000
}

static void skip_spaces(const char **text) {
    size_t char_length; // length of current UTF-8 character

    while (**text != '\0') {
        char_length = utf8_char_length(*text);
        if (!is_utf8_space(*text, char_length)) {
            return;
        }

        *text += char_length;
    }
}

static int build_data_path(char *path, size_t path_size, const char *filename) {
    const char *source_path = __FILE__; // source file location at compile time
    const char *last_separator = strrchr(source_path, '/'); // Unix path separator
    int written; // snprintf result

#ifdef _WIN32
    const char *last_backslash = strrchr(source_path, '\\'); // Windows path separator

    if (last_backslash != NULL && (last_separator == NULL || last_backslash > last_separator)) {
        last_separator = last_backslash;
    }
#endif

    if (last_separator != NULL) {
        written = snprintf(path, path_size, "%.*s/%s/%s",
                           (int)(last_separator - source_path),
                           source_path,
                           DATA_DIR_NAME,
                           filename);
    } else {
        written = snprintf(path, path_size, "%s/%s", DATA_DIR_NAME, filename);
    }

    return written > 0 && (size_t)written < path_size;
}

static int build_data_directory_path(char *path, size_t path_size) {
    const char *source_path = __FILE__; // source file location at compile time
    const char *last_separator = strrchr(source_path, '/'); // Unix path separator
    int written; // snprintf result

#ifdef _WIN32
    const char *last_backslash = strrchr(source_path, '\\'); // Windows path separator

    if (last_backslash != NULL && (last_separator == NULL || last_backslash > last_separator)) {
        last_separator = last_backslash;
    }
#endif

    if (last_separator != NULL) {
        written = snprintf(path, path_size, "%.*s/%s",
                           (int)(last_separator - source_path),
                           source_path,
                           DATA_DIR_NAME);
    } else {
        written = snprintf(path, path_size, "%s", DATA_DIR_NAME);
    }

    return written > 0 && (size_t)written < path_size;
}

static int ensure_data_directory(void) {
    char data_directory[PATH_BUFFER_SIZE]; // path to the data folder

    if (!build_data_directory_path(data_directory, sizeof(data_directory))) {
        return 0;
    }

#ifdef _WIN32
    if (_mkdir(data_directory) == 0) {
#else
    if (mkdir(data_directory, 0777) == 0) {
#endif
        return 1; // directory created
    }

    return errno == EEXIST; // directory already exists
}

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
    const char *p = start; // current UTF-8 character
    const char *first_text = NULL; // first non-space character
    const char *after_last_text = NULL; // byte after last non-space character
    size_t char_length; // current character length in bytes
    size_t result = 0; // trimmed length in characters

    while (p <= end && *p != '\0') {
        char_length = utf8_char_length(p);
        if (p + char_length - 1 > end) {
            char_length = (size_t)(end - p + 1); // clamp incomplete final character
        }

        if (!is_utf8_space(p, char_length)) {
            if (first_text == NULL) {
                first_text = p;
            }
            after_last_text = p + char_length;
        }

        p += char_length;
    }

    if (first_text == NULL) {
        return 0; // empty text
    }

    p = first_text;
    while (p < after_last_text) {
        char_length = utf8_char_length(p);
        if (p + char_length > after_last_text) {
            char_length = (size_t)(after_last_text - p); // clamp incomplete final character
        }

        result++;
        p += char_length;
    }

    return result; // return trimmed length in UTF-8 characters
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
        size_t ending_length = 0; // length of matched punctuation
        SentenceEndingType ending_type = find_sentence_ending(p, &ending_length);

        if (ending_type == ENDING_ELLIPSIS) { // found an ellipsis variant
            size_t len = trimmed_length(sentence_start, p + ending_length - 1); // sentence length

            if (len > 0) {
                (*ellipsis_count)++; // count ellipsis sentence
                ellipsis_index++; // increment ellipsis index

                if (ellipsis_index == 2) {
                    *second_ellipsis_length = len; // save second ellipsis length
                    *has_second = 1; // second ellipsis exists
                }
            }

            p += ending_length; // skip ellipsis
            skip_spaces(&p); // skip spaces after sentence
            sentence_start = p; // next sentence starts here
        } else if (ending_type == ENDING_DECLARATIVE) { // found a declarative ending
            size_t len = trimmed_length(sentence_start, p + ending_length - 1); // sentence length

            if (len > 0) {
                (*declarative_count)++; // count declarative sentence
            }

            p += ending_length; // move past punctuation
            skip_spaces(&p); // skip spaces
            sentence_start = p; // next sentence starts here
        } else if (ending_type == ENDING_OTHER) { // other sentence endings
            p += ending_length; // move past punctuation
            skip_spaces(&p); // skip spaces
            sentence_start = p; // next sentence starts here
        } else {
            p += utf8_char_length(p); // continue scanning
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
    char input_file[PATH_BUFFER_SIZE]; // path to input.txt
    char output_file[PATH_BUFFER_SIZE]; // path to output.txt
    int declarative_count = 0; // number of declarative sentences
    int ellipsis_count = 0; // number of ellipsis sentences
    int difference = 0; // difference between counts
    int has_second = 0; // flag for second ellipsis sentence
    size_t second_ellipsis_length = 0; // length of second ellipsis sentence

    if (!build_data_path(input_file, sizeof(input_file), INPUT_FILENAME) ||
        !build_data_path(output_file, sizeof(output_file), OUTPUT_FILENAME)) {
        fprintf(stderr, "Data file path is too long\n");
        return 1;
    }

    if (!ensure_data_directory()) {
        fprintf(stderr, "Could not create %s directory\n", DATA_DIR_NAME);
        return 1;
    }

    printf("Enter a text line:\n");
    text = read_line(); // read text from keyboard

    if (text == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1; // stop if input allocation failed
    }

    if (!write_text_file(input_file, text)) {
        fprintf(stderr, "Could not write to %s\n", input_file);
        free(text); // free memory before exit
        return 1;
    }

    file_text = read_text_file(input_file); // read text back from file
    if (file_text == NULL) {
        fprintf(stderr, "Could not read from %s\n", input_file);
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

    if (!save_result(output_file, declarative_count, ellipsis_count,
                     difference, second_ellipsis_length, has_second)) {
        fprintf(stderr, "Could not write to %s\n", output_file);
        free(text);
        free(file_text);
        return 1;
    }

    printf("Results were written to %s\n", output_file); // confirmation message

    free(text); // free input text
    free(file_text); // free file text

    return 0; // successful end
}
