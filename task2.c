#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define MAX_CHAR 50 // maximum string length
#define DATA_DIR_NAME "data"
#define EXPERIMENT_FILENAME "experiment.txt"
#define OUTPUT_FILENAME "output.txt"
#define TEMP_FILENAME "temp.txt"
#define PATH_BUFFER_SIZE 1024

typedef struct {
    int day; // contract day
    char month[MAX_CHAR]; // contract month
    int year; // contract year
} Date;

typedef struct {
    char owner_name[MAX_CHAR]; // owner first name
    char owner_surname[MAX_CHAR]; // owner surname
    char product_name[MAX_CHAR]; // product name
    char manufacturer[MAX_CHAR]; // manufacturer name
    Date contract_date; // contract date
    float wholesale_price; // wholesale price
    float unit_price; // unit price
    int quantity; // quantity of product
} Info;

typedef struct {
    char product_name[MAX_CHAR]; // product name
    int total_quantity; // total quantity for this product
} ProductTotal;


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
        if (length + 1 >= capacity) { // check if buffer is full
            capacity *= 2; // double buffer size
            temp = (char *)realloc(buffer, capacity * sizeof(char)); // resize buffer
            if (temp == NULL) {
                free(buffer); // free old memory if realloc fails
                return NULL;
            }
            buffer = temp; // update buffer pointer
        }

        *(buffer + length) = (char)ch; // store character
        length++; // move to next position
    }

    *(buffer + length) = '\0'; // end the string
    return buffer; // return entered text
}


void clear_input_buffer(void) {
    int ch; // current character

    while ((ch = getchar()) != '\n' && ch != EOF) { // remove leftover input
    }
}


void input_info(Info *item) {
    char *line; // temporary dynamic string

    printf("Owner name: ");
    line = read_line(); // read owner name
    if (line != NULL) {
        strncpy(item->owner_name, line, MAX_CHAR - 1); // copy into struct
        item->owner_name[MAX_CHAR - 1] = '\0'; // ensure end of string
        free(line); // free dynamic memory
    } else {
        item->owner_name[0] = '\0'; // empty string on failure
    }

    printf("Owner surname: ");
    line = read_line(); // read owner surname
    if (line != NULL) {
        strncpy(item->owner_surname, line, MAX_CHAR - 1); // copy into struct
        item->owner_surname[MAX_CHAR - 1] = '\0'; // ensure end of string
        free(line); // free dynamic memory
    } else {
        item->owner_surname[0] = '\0'; // empty string on failure
    }

    printf("Product name: ");
    line = read_line(); // read product name
    if (line != NULL) {
        strncpy(item->product_name, line, MAX_CHAR - 1); // copy into struct
        item->product_name[MAX_CHAR - 1] = '\0'; // ensure end of string
        free(line); // free dynamic memory
    } else {
        item->product_name[0] = '\0'; // empty string on failure
    }

    printf("Manufacturer: ");
    line = read_line(); // read manufacturer
    if (line != NULL) {
        strncpy(item->manufacturer, line, MAX_CHAR - 1); // copy into struct
        item->manufacturer[MAX_CHAR - 1] = '\0'; // ensure end of string
        free(line); // free dynamic memory
    } else {
        item->manufacturer[0] = '\0'; // empty string on failure
    }

    printf("Day: ");
    scanf("%d", &item->contract_date.day); // read day
    clear_input_buffer(); // clear newline

    printf("Month: ");
    line = read_line(); // read month
    if (line != NULL) {
        strncpy(item->contract_date.month, line, MAX_CHAR - 1); // copy month
        item->contract_date.month[MAX_CHAR - 1] = '\0'; // ensure end of string
        free(line); // free dynamic memory
    } else {
        item->contract_date.month[0] = '\0'; // empty string on failure
    }

    printf("Year: ");
    scanf("%d", &item->contract_date.year); // read year
    clear_input_buffer(); // clear newline

    printf("Wholesale price: ");
    scanf("%f", &item->wholesale_price); // read wholesale price
    clear_input_buffer(); // clear newline

    printf("Unit price: ");
    scanf("%f", &item->unit_price); // read unit price
    clear_input_buffer(); // clear newline

    printf("Quantity: ");
    scanf("%d", &item->quantity); // read quantity
    clear_input_buffer(); // clear newline
}


void print_all_info(const Info *items, int n) {
    const Info *p; // pointer for iteration
    int i = 1; // record number

    for (p = items; p < items + n; p++) { // go through all records
        printf("\nRecord %d:\n", i++);
        printf("Owner: %s %s\n", p->owner_name, p->owner_surname);
        printf("Product: %s\n", p->product_name);
        printf("Manufacturer: %s\n", p->manufacturer);
        printf("Date: %d %s %d\n",
               p->contract_date.day,
               p->contract_date.month,
               p->contract_date.year);
        printf("Wholesale price: %.2f\n", p->wholesale_price);
        printf("Unit price: %.2f\n", p->unit_price);
        printf("Quantity: %d\n", p->quantity);
    }
}


int save_records_to_experiment(const char *filename, const Info *items, int n) {
    FILE *file = fopen(filename, "w"); // open file for writing
    const Info *p; // pointer for iteration

    if (file == NULL) {
        return 0; // file open failed
    }

    fprintf(file, "%d\n", n); // save number of records

    for (p = items; p < items + n; p++) { // write each record
        fprintf(file, "%s|%s|%s|%s|%d|%s|%d|%.2f|%.2f|%d\n",
                p->owner_name,
                p->owner_surname,
                p->product_name,
                p->manufacturer,
                p->contract_date.day,
                p->contract_date.month,
                p->contract_date.year,
                p->wholesale_price,
                p->unit_price,
                p->quantity);
    }

    fclose(file); // close file
    return 1; // success
}


int load_records_from_experiment(const char *filename, Info **items, int *n) {
    FILE *file = fopen(filename, "r"); // open file for reading
    Info *temp; // temporary array
    Info *p; // pointer for iteration

    if (file == NULL) {
        return 0; // file open failed
    }

    if (fscanf(file, "%d", n) != 1 || *n <= 0) { // read number of records
        fclose(file);
        return 0;
    }

    temp = (Info *)malloc((*n) * sizeof(Info)); // allocate memory for records
    if (temp == NULL) {
        fclose(file);
        return 0; // allocation failed
    }

    for (p = temp; p < temp + *n; p++) { // read each record
        if (fscanf(file,
                   " %49[^|]|%49[^|]|%49[^|]|%49[^|]|%d|%49[^|]|%d|%f|%f|%d",
                   p->owner_name,
                   p->owner_surname,
                   p->product_name,
                   p->manufacturer,
                   &p->contract_date.day,
                   p->contract_date.month,
                   &p->contract_date.year,
                   &p->wholesale_price,
                   &p->unit_price,
                   &p->quantity) != 10) {
            free(temp); // free memory if reading fails
            fclose(file);
            return 0;
        }
    }

    fclose(file); // close file
    *items = temp; // return loaded records
    return 1; // success
}


void calculate_totals(const Info *items, int n, ProductTotal **totals, int *total_count) {
    ProductTotal *temp; // temporary totals array
    const Info *p; // pointer through items
    ProductTotal *t; // pointer through totals
    int count = 0; // number of unique products
    int found; // flag for existing product

    temp = (ProductTotal *)malloc(n * sizeof(ProductTotal)); // worst case: all unique
    if (temp == NULL) {
        *totals = NULL; // allocation failed
        *total_count = 0;
        return;
    }

    for (p = items; p < items + n; p++) { // process all records
        found = 0; // assume product not found

        for (t = temp; t < temp + count; t++) { // search existing products
            if (strcmp(t->product_name, p->product_name) == 0) {
                t->total_quantity += p->quantity; // add quantity
                found = 1; // mark as found
                break;
            }
        }

        if (!found) { // new product
            strncpy((temp + count)->product_name, p->product_name, MAX_CHAR - 1);
            (temp + count)->product_name[MAX_CHAR - 1] = '\0';
            (temp + count)->total_quantity = p->quantity;
            count++; // increase number of unique products
        }
    }

    *totals = temp; // return totals array
    *total_count = count; // return count
}


// Insertion sort
void sort_totals_desc(ProductTotal *totals, int total_count) {
    int i, j;
    ProductTotal key;

    for (i = 1; i < total_count; i++) {
        key = *(totals + i);
        j = i - 1;

        // For ascending order replace with: while (j >= 0 && strcmp((totals + j)->product_name, key.product_name) > 0) {
        while (j >= 0 && strcmp((totals + j)->product_name, key.product_name) < 0) {
            *(totals + j + 1) = *(totals + j);
            j--;
        }

        *(totals + j + 1) = key;
    }
}


int save_totals_to_output(const char *filename, const ProductTotal *totals, int total_count) {
    FILE *file = fopen(filename, "w"); // open file for writing
    const ProductTotal *p; // pointer for iteration

    if (file == NULL) {
        return 0; // file open failed
    }

    for (p = totals; p < totals + total_count; p++) { // write each total
        fprintf(file, "%s - %d units\n", p->product_name, p->total_quantity);
    }

    fclose(file); // close file
    return 1; // success
}


int output_to_experiment(const char *output_filename, const char *experiment_filename,
                         const char *temp_filename) {
    FILE *fout; // output.txt file
    FILE *fexp; // experiment.txt file
    FILE *ftemp; // temporary file
    int ch; // current character

    fout = fopen(output_filename, "r"); // open output file
    fexp = fopen(experiment_filename, "r"); // open experiment file
    ftemp = fopen(temp_filename, "w"); // open temp file

    if (fout == NULL || fexp == NULL || ftemp == NULL) {
        if (fout != NULL) fclose(fout); // close opened files
        if (fexp != NULL) fclose(fexp);
        if (ftemp != NULL) fclose(ftemp);
        return 0; // opening failed
    }

    while ((ch = fgetc(fout)) != EOF) { // copy output.txt to temp
        fputc(ch, ftemp);
    }

    fputc('\n', ftemp); // add empty line between files

    while ((ch = fgetc(fexp)) != EOF) { // copy experiment.txt after output text
        fputc(ch, ftemp);
    }

    fclose(fout); // close files
    fclose(fexp);
    fclose(ftemp);

    remove(experiment_filename); // delete old experiment.txt
    rename(temp_filename, experiment_filename); // rename temp file

    return 1; // success
}


void print_text_file(const char *filename) {
    FILE *file = fopen(filename, "r"); // open file for reading
    int ch; // current character

    if (file == NULL) {
        return; // file open failed
    }

    printf("\n%s:\n", filename); // print file name
    while ((ch = fgetc(file)) != EOF) { // print whole file
        putchar(ch);
    }
    printf("\n");

    fclose(file); // close file
}


int main(void) {
    int n; // number of records
    int loaded_n; // number of loaded records
    int total_count; // number of product totals
    Info *items; // entered records
    Info *loaded_items; // records loaded from file
    ProductTotal *totals; // totals array
    Info *p; // pointer for iteration
    char experiment_file[PATH_BUFFER_SIZE]; // path to experiment.txt
    char output_file[PATH_BUFFER_SIZE]; // path to output.txt
    char temp_file[PATH_BUFFER_SIZE]; // path to temp.txt

    if (!build_data_path(experiment_file, sizeof(experiment_file), EXPERIMENT_FILENAME) ||
        !build_data_path(output_file, sizeof(output_file), OUTPUT_FILENAME) ||
        !build_data_path(temp_file, sizeof(temp_file), TEMP_FILENAME)) {
        fprintf(stderr, "Data file path is too long\n");
        return 1;
    }

    if (!ensure_data_directory()) {
        fprintf(stderr, "Could not create %s directory\n", DATA_DIR_NAME);
        return 1;
    }

    printf("Enter number of records: ");
    scanf("%d", &n); // read number of records
    clear_input_buffer(); // clear newline

    if (n <= 0) {
        return 1; // invalid count
    }

    items = (Info *)malloc(n * sizeof(Info)); // allocate records array
    if (items == NULL) {
        return 1; // allocation failed
    }

    for (p = items; p < items + n; p++) { // read all records
        printf("\nRecord %ld\n", (long)(p - items) + 1);
        input_info(p);
    }

    if (!save_records_to_experiment(experiment_file, items, n)) {
        free(items); // free memory on failure
        return 1;
    }

    loaded_items = NULL; // initialize pointer
    if (!load_records_from_experiment(experiment_file, &loaded_items, &loaded_n)) {
        free(items); // free memory on failure
        return 1;
    }

    print_all_info(loaded_items, loaded_n); // display read data

    totals = NULL; // initialize pointer
    calculate_totals(loaded_items, loaded_n, &totals, &total_count); // compute totals
    if (totals == NULL) {
        free(items);
        free(loaded_items);
        return 1;
    }

    sort_totals_desc(totals, total_count); // sort totals

    if (!save_totals_to_output(output_file, totals, total_count)) {
        free(items);
        free(loaded_items);
        free(totals);
        return 1;
    }

    print_text_file(output_file); // display output.txt

    if (!output_to_experiment(output_file, experiment_file, temp_file)) {
        free(items);
        free(loaded_items);
        free(totals);
        return 1;
    }

    print_text_file(experiment_file); // display modified experiment.txt

    free(items); // free entered records
    free(loaded_items); // free loaded records
    free(totals); // free totals

    return 0; // successful finish
}
