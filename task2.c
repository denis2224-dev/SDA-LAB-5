#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    Date contract_date;
    float wholesale_price;
    float unit_price;
    int quantity;
} Info;

typedef struct {
    char product_name[MAX_CHAR];
    int total_quantity;
} ProductTotal;


char *read_line(void) {
    size_t capacity = 16;
    size_t length = 0;
    int ch;
    char *buffer = (char *)malloc(capacity * sizeof(char));
    char *temp;

    if (buffer == NULL) {
        return NULL;
    }

    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            temp = (char *)realloc(buffer, capacity * sizeof(char));
            if (temp == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }

        *(buffer + length) = (char)ch;
        length++;
    }

    *(buffer + length) = '\0';
    return buffer;
}


void clear_input_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}


void input_info(Info *item) {
    char *line;

    printf("Owner name: ");
    line = read_line();
    if (line != NULL) {
        strncpy(item->owner_name, line, MAX_CHAR - 1);
        item->owner_name[MAX_CHAR - 1] = '\0';
        free(line);
    } else {
        item->owner_name[0] = '\0';
    }

    printf("Owner surname: ");
    line = read_line();
    if (line != NULL) {
        strncpy(item->owner_surname, line, MAX_CHAR - 1);
        item->owner_surname[MAX_CHAR - 1] = '\0';
        free(line);
    } else {
        item->owner_surname[0] = '\0';
    }

    printf("Product name: ");
    line = read_line();
    if (line != NULL) {
        strncpy(item->product_name, line, MAX_CHAR - 1);
        item->product_name[MAX_CHAR - 1] = '\0';
        free(line);
    } else {
        item->product_name[0] = '\0';
    }

    printf("Manufacturer: ");
    line = read_line();
    if (line != NULL) {
        strncpy(item->manufacturer, line, MAX_CHAR - 1);
        item->manufacturer[MAX_CHAR - 1] = '\0';
        free(line);
    } else {
        item->manufacturer[0] = '\0';
    }

    printf("Day: ");
    scanf("%d", &item->contract_date.day);
    clear_input_buffer();

    printf("Month: ");
    line = read_line();
    if (line != NULL) {
        strncpy(item->contract_date.month, line, MAX_CHAR - 1);
        item->contract_date.month[MAX_CHAR - 1] = '\0';
        free(line);
    } else {
        item->contract_date.month[0] = '\0';
    }

    printf("Year: ");
    scanf("%d", &item->contract_date.year);
    clear_input_buffer();

    printf("Wholesale price: ");
    scanf("%f", &item->wholesale_price);
    clear_input_buffer();

    printf("Unit price: ");
    scanf("%f", &item->unit_price);
    clear_input_buffer();

    printf("Quantity: ");
    scanf("%d", &item->quantity);
    clear_input_buffer();
}


void print_all_info(const Info *items, int n) {
    const Info *p;
    int i = 1;

    for (p = items; p < items + n; p++) {
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
    FILE *file = fopen(filename, "w");
    const Info *p;

    if (file == NULL) {
        return 0;
    }

    fprintf(file, "%d\n", n);

    for (p = items; p < items + n; p++) {
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

    fclose(file);
    return 1;
}


int load_records_from_experiment(const char *filename, Info **items, int *n) {
    FILE *file = fopen(filename, "r");
    Info *temp;
    Info *p;

    if (file == NULL) {
        return 0;
    }

    if (fscanf(file, "%d", n) != 1 || *n <= 0) {
        fclose(file);
        return 0;
    }

    temp = (Info *)malloc((*n) * sizeof(Info));
    if (temp == NULL) {
        fclose(file);
        return 0;
    }

    for (p = temp; p < temp + *n; p++) {
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
            free(temp);
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    *items = temp;
    return 1;
}


void calculate_totals(const Info *items, int n, ProductTotal **totals, int *total_count) {
    ProductTotal *temp;
    const Info *p;
    ProductTotal *t;
    int count = 0;
    int found;

    temp = (ProductTotal *)malloc(n * sizeof(ProductTotal));
    if (temp == NULL) {
        *totals = NULL;
        *total_count = 0;
        return;
    }

    for (p = items; p < items + n; p++) {
        found = 0;

        for (t = temp; t < temp + count; t++) {
            if (strcmp(t->product_name, p->product_name) == 0) {
                t->total_quantity += p->quantity;
                found = 1;
                break;
            }
        }

        if (!found) {
            strncpy((temp + count)->product_name, p->product_name, MAX_CHAR - 1);
            (temp + count)->product_name[MAX_CHAR - 1] = '\0';
            (temp + count)->total_quantity = p->quantity;
            count++;
        }
    }

    *totals = temp;
    *total_count = count;
}


void sort_totals_desc(ProductTotal *totals, int total_count) {
    int i, j;
    ProductTotal temp;
    ProductTotal *a;
    ProductTotal *b;

    for (i = 0; i < total_count - 1; i++) {
        for (j = 0; j < total_count - i - 1; j++) {
            a = totals + j;
            b = totals + j + 1;

            if (strcmp(a->product_name, b->product_name) < 0) {
                temp = *a;
                *a = *b;
                *b = temp;
            }
        }
    }
}


int save_totals_to_output(const char *filename, const ProductTotal *totals, int total_count) {
    FILE *file = fopen(filename, "w");
    const ProductTotal *p;

    if (file == NULL) {
        return 0;
    }

    for (p = totals; p < totals + total_count; p++) {
        fprintf(file, "%s - %d units\n", p->product_name, p->total_quantity);
    }

    fclose(file);
    return 1;
}


int output_to_experiment(const char *output_filename, const char *experiment_filename) {
    FILE *fout;
    FILE *fexp;
    FILE *ftemp;
    int ch;

    fout = fopen(output_filename, "r");
    fexp = fopen(experiment_filename, "r");
    ftemp = fopen("temp.txt", "w");

    if (fout == NULL || fexp == NULL || ftemp == NULL) {
        if (fout != NULL) fclose(fout);
        if (fexp != NULL) fclose(fexp);
        if (ftemp != NULL) fclose(ftemp);
        return 0;
    }

    while ((ch = fgetc(fout)) != EOF) {
        fputc(ch, ftemp);
    }

    fputc('\n', ftemp);

    while ((ch = fgetc(fexp)) != EOF) {
        fputc(ch, ftemp);
    }

    fclose(fout);
    fclose(fexp);
    fclose(ftemp);

    remove(experiment_filename);
    rename("temp.txt", experiment_filename);

    return 1;
}


void print_text_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    int ch;

    if (file == NULL) {
        return;
    }

    printf("\n%s:\n", filename);
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }
    printf("\n");

    fclose(file);
}


int main(void) {
    int n;
    int loaded_n;
    int total_count;
    Info *items;
    Info *loaded_items;
    ProductTotal *totals;
    Info *p;

    printf("Enter number of records: ");
    scanf("%d", &n);
    clear_input_buffer();

    if (n <= 0) {
        return 1;
    }

    items = (Info *)malloc(n * sizeof(Info));
    if (items == NULL) {
        return 1;
    }

    for (p = items; p < items + n; p++) {
        printf("\nRecord %ld\n", (long)(p - items) + 1);
        input_info(p);
    }

    if (!save_records_to_experiment("experiment.txt", items, n)) {
        free(items);
        return 1;
    }

    loaded_items = NULL;
    if (!load_records_from_experiment("experiment.txt", &loaded_items, &loaded_n)) {
        free(items);
        return 1;
    }

    print_all_info(loaded_items, loaded_n);

    totals = NULL;
    calculate_totals(loaded_items, loaded_n, &totals, &total_count);
    if (totals == NULL) {
        free(items);
        free(loaded_items);
        return 1;
    }

    sort_totals_desc(totals, total_count);

    if (!save_totals_to_output("output.txt", totals, total_count)) {
        free(items);
        free(loaded_items);
        free(totals);
        return 1;
    }

    print_text_file("output.txt");

    if (!output_to_experiment("output.txt", "experiment.txt")) {
        free(items);
        free(loaded_items);
        free(totals);
        return 1;
    }

    print_text_file("experiment.txt");

    free(items);
    free(loaded_items);
    free(totals);

    return 0;
}