#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "gutenberg.h"
#include "transfer.h"

#define GUTENBERG_RAW_FILE  "/tmp/powertools-gutenberg-raw.txt"
#define GUTENBERG_TEXT_FILE "/tmp/powertools-gutenberg.txt"

static int clean_gutenberg_file(const char *input_filename,
                                const char *output_filename)
{
    FILE *in;
    FILE *out;
    char line[4096];
    int started = 0;

    in = fopen(input_filename, "r");
    if (in == NULL) {
        perror("Could not open Gutenberg download");
        return -1;
    }

    out = fopen(output_filename, "w");
    if (out == NULL) {
        perror("Could not create Gutenberg text file");
        fclose(in);
        return -1;
    }

    while (fgets(line, sizeof(line), in) != NULL) {

        if (!started) {
            if (strstr(line, "*** START OF THE PROJECT GUTENBERG EBOOK") != NULL ||
                strstr(line, "*** START OF THIS PROJECT GUTENBERG EBOOK") != NULL) {
                started = 1;
            }

            continue;
        }

        if (strstr(line, "*** END OF THE PROJECT GUTENBERG EBOOK") != NULL ||
            strstr(line, "*** END OF THIS PROJECT GUTENBERG EBOOK") != NULL) {
            break;
        }

        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    return 0;
}

static int gutenberg_search(const char *query,
                            int book_ids[],
                            int max_results)
{
    char command[1024];
    FILE *input;
    char *buffer;
    size_t buffer_size = 65536;
    struct json_object *json;
    struct json_object *results;
    size_t length;
    int i;

    snprintf(command, sizeof(command),
             "curl -L -s --get "
             "--data-urlencode \"search=%s\" "
             "--data-urlencode \"languages=en\" "
             "https://gutendex.com/books",
             query);

    input = popen(command, "r");

    if (input == NULL) {
        perror("Could not search Project Gutenberg");
        return -1;
    }

buffer = malloc(buffer_size);

if (buffer == NULL) {
    printf("Could not allocate memory for search results.\n");
    pclose(input);
    return -1;
}

length = fread(buffer, 1, buffer_size - 1, input);
buffer[length] = '\0';

pclose(input);

json = json_tokener_parse(buffer);

free(buffer);

    if (json == NULL) {
        printf("Could not parse Gutenberg search results.\n");
        return -1;
    }

    if (!json_object_object_get_ex(json, "results", &results)) {
        printf("No search results found.\n");
        json_object_put(json);
        return 0;
    }

    length = json_object_array_length(results);

    if (length == 0) {
        printf("No books found.\n");
        json_object_put(json);
        return 0;
    }

if (length > (size_t)max_results) {
    length = (size_t)max_results;
}
    printf("\n");
    printf("Gutenberg Search Results\n");
    printf("\n");

    for (i = 0; i < (int)length; i++) {
        struct json_object *book;
        struct json_object *id;
        struct json_object *title;
        struct json_object *authors;
        struct json_object *author;
        struct json_object *name;

        book = json_object_array_get_idx(results, i);

        json_object_object_get_ex(book, "id", &id);
        json_object_object_get_ex(book, "title", &title);
        json_object_object_get_ex(book, "authors", &authors);

        book_ids[i] = json_object_get_int(id);

        printf("%d. %s\n",
               i + 1,
               json_object_get_string(title));

        if (authors != NULL &&
            json_object_array_length(authors) > 0) {

            author = json_object_array_get_idx(authors, 0);

            if (json_object_object_get_ex(author, "name", &name)) {
                printf("   %s\n",
                       json_object_get_string(name));
            }
        }

        printf("   Gutenberg #%d\n\n",
               json_object_get_int(id));
    }

    json_object_put(json);

    return (int)length;
}

static int gutenberg_download(int book_id)
{
    char command[512];
    char prefix[512];
    int max_size;
    int parts;
    int current_part;

    snprintf(command, sizeof(command),
             "curl -L -s "
             "\"https://www.gutenberg.org/cache/epub/%d/pg%d.txt\" "
             "-o \"%s\"",
             book_id,
             book_id,
             GUTENBERG_RAW_FILE);

    printf("\nDownloading Gutenberg ebook %d...\n", book_id);

    if (system(command) != 0) {
        printf("Could not download the ebook.\n");
        return -1;
    }

    if (clean_gutenberg_file(GUTENBERG_RAW_FILE,
                             GUTENBERG_TEXT_FILE) != 0) {
        return -1;
    }

    printf("\nHow much free PowerNote memory is available? ");

    if (scanf("%d", &max_size) != 1 || max_size <= 0) {
        printf("Invalid memory size.\n");
        return -1;
    }

    getchar();

    snprintf(prefix, sizeof(prefix),
             "/tmp/powertools-gutenberg");

    parts = split_file(GUTENBERG_TEXT_FILE,
                       prefix,
                       (size_t)max_size);

    if (parts <= 0) {
        printf("Could not split Gutenberg text.\n");
        return -1;
    }

    printf("\nGutenberg ebook split into %d part%s.\n",
           parts,
           parts == 1 ? "" : "s");

    current_part = 1;

    while (current_part <= parts) {
        char part_filename[512];

        snprintf(part_filename,
                 sizeof(part_filename),
                 "/tmp/powertools-gutenberg_%02d.txt",
                 current_part);

        if (send_transfer_part(part_filename,
                               current_part,
                               parts) != 0) {
            return -1;
        }

        if (current_part == parts) {
            printf("\nAll Gutenberg parts transferred.\n");
            break;
        }

        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|             Gutenberg Transfer              |\n"
            "+---------------------------------------------+\n"
            "|  1. Send next part                          |\n"
            "|  2. Cancel transfer                         |\n"
            "+---------------------------------------------+\n"
            "\n"
            "Enter your choice: "
        );

        {
            int choice;

            if (scanf("%d", &choice) != 1) {
                printf("Invalid input. Transfer cancelled.\n");
                return -1;
            }

            getchar();

            if (choice == 1) {
                current_part++;
            } else {
                printf("Transfer cancelled.\n");
                return -1;
            }
        }
    }

    return 0;
}

void gutenberg_lookup(void)
{
    char query[256];
    int result_count;
    int choice;
    int book_ids[5];
    int selected_id;

    printf("\nProject Gutenberg\n\n");
    printf("Search for a book: ");

    if (fgets(query, sizeof(query), stdin) == NULL) {
        printf("Input error.\n");
        return;
    }

    query[strcspn(query, "\n")] = '\0';

    if (query[0] == '\0') {
        printf("No search entered.\n");
        return;
    }

    result_count = gutenberg_search(query, book_ids, 5);

    if (result_count <= 0) {
        return;
    }

    printf("Select a book (1-%d), or 0 to cancel: ",
           result_count);

    if (scanf("%d", &choice) != 1) {
        printf("Invalid selection.\n");

        while (getchar() != '\n') {
            /* discard invalid input */
        }

        return;
    }

    getchar();

    if (choice < 1 || choice > result_count) {
        printf("Search cancelled.\n");
        return;
    }

   selected_id = book_ids[choice - 1];

printf("\nYou selected Gutenberg #%d.\n", selected_id);

gutenberg_download(selected_id);
}