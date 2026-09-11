#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sourcebooks.h"
#include "transfer.h"

#define SOURCEBOOK_RAW_FILE  "/tmp/powertools-sourcebook-raw.html"
#define SOURCEBOOK_TEXT_FILE "/tmp/powertools-sourcebook.txt"

static void write_text(FILE *out, const char *text)
{
    while (*text != '\0') {

        if (strncmp(text, "&nbsp;", 6) == 0) {
            fputc(' ', out);
            text += 6;
            continue;
        }

        if (strncmp(text, "&amp;", 5) == 0) {
            fputc('&', out);
            text += 5;
            continue;
        }

        if (strncmp(text, "&quot;", 6) == 0) {
            fputc('"', out);
            text += 6;
            continue;
        }

        fputc(*text, out);
        text++;
    }
}

static int clean_sourcebook_file(const char *input_filename,
                                 const char *output_filename,
                                 const char *title,
                                 const char *url)
{

    FILE *in;
    FILE *out;
    char line[8192];
    int in_tag = 0;
    int in_document = 0;

    in = fopen(input_filename, "r");

    if (in == NULL) {
        perror("Could not open sourcebook download");
        return -1;
    }

    out = fopen(output_filename, "w");

    if (out == NULL) {
        perror("Could not create sourcebook text file");
        fclose(in);
        return -1;
    }

fprintf(out,
        "Internet Modern History Sourcebook\n\n"
        "%s\n\n",
        title);

    while (fgets(line, sizeof(line), in) != NULL) {
    char text[8192];
    size_t i;
    size_t j = 0;

    if (!in_document) {
        if (strstr(line, "<blockquote>") != NULL) {
            in_document = 1;
        } else {
            continue;
        }
    }

    if (in_document && strstr(line, "<hr") != NULL) {
        break;
    }


    
    for (i = 0; line[i] != '\0' && j < sizeof(text) - 1; i++) {

        if (line[i] == '<') {
            in_tag = 1;
            continue;
        }

        if (line[i] == '>') {
            in_tag = 0;
            continue;
        }

        if (!in_tag) {
            text[j++] = line[i];
        }
    }

    text[j] = '\0';

    write_text(out, text);
}

fprintf(out,
        "\n"
        "----------------------------------------\n\n"
        "Source:\n"
        "Internet History Sourcebooks Project\n"
        "%s\n\n"
        "%s\n",
        title,
        url);

    fclose(in);
    fclose(out);

    return 0;
}

#define SOURCEBOOK_SEARCH_FILE "/tmp/powertools-sourcebook-search.html"

static int sourcebook_search(const char *query,
                             char urls[][512],
                             int max_results)
{
    char command[1024];
    FILE *file;
    char line[8192];
    int count = 0;

    snprintf(command, sizeof(command),
             "curl -L -s --get "
             "--data-urlencode \"query=%s\" "
             "--data-urlencode \"collection=fordham~sp-search\" "
             "--data-urlencode \"clive=fordham~ds-sourcebooks\" "
             "--data-urlencode \"num_ranks=%d\" "
             "https://search.fordham.edu/s/search.html "
             "-o \"%s\"",
             query,
             max_results,
             SOURCEBOOK_SEARCH_FILE);

    if (system(command) != 0) {
        printf("Could not perform sourcebook search.\n");
        return -1;
    }

    file = fopen(SOURCEBOOK_SEARCH_FILE, "r");

    if (file == NULL) {
        perror("Could not open sourcebook search results");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *start;
        char *end;
        size_t length;

        start = strstr(line, "<cite>");

        if (start == NULL) {
            continue;
        }

        start += strlen("<cite>");

        end = strstr(start, "</cite>");

        if (end == NULL) {
            continue;
        }

        length = (size_t)(end - start);

        if (length >= sizeof(urls[0])) {
            continue;
        }

        memcpy(urls[count], start, length);
        urls[count][length] = '\0';

        count++;

        if (count >= max_results) {
            break;
        }
    }

    fclose(file);

    return count;
}

static int sourcebook_download(const char *title,
                               const char *url)
{
    char command[512];
    int max_size;
    int parts;
    int current_part;

snprintf(command, sizeof(command),
         "curl -L -s \"%s\" -o \"%s\"",
         url,
         SOURCEBOOK_RAW_FILE);

    if (system(command) != 0) {
        printf("Could not download the source.\n");
        return -1;
    }

if (clean_sourcebook_file(SOURCEBOOK_RAW_FILE,
                          SOURCEBOOK_TEXT_FILE,
                          title,
                          url) != 0) {
        return -1;
    }
    printf("\nInternet History Sourcebooks\n\n");
    printf("Retrieving:\n");
    printf("%s\n\n", title);
    printf("Source retrieved successfully.\n\n");

    printf("How much free PowerNote memory is available? ");

    if (scanf("%d", &max_size) != 1 || max_size <= 0) {
        printf("Invalid memory size.\n");
        return -1;
    }

    getchar();

    parts = split_file(SOURCEBOOK_TEXT_FILE,
                       "/tmp/powertools-sourcebook",
                       (size_t)max_size);

    if (parts <= 0) {
        printf("Could not split sourcebook text.\n");
        return -1;
    }

    printf("\nSourcebook text split into %d part%s.\n",
           parts,
           parts == 1 ? "" : "s");

    current_part = 1;

    while (current_part <= parts) {
        char part_filename[512];

        snprintf(part_filename,
                 sizeof(part_filename),
                 "/tmp/powertools-sourcebook_%02d.txt",
                 current_part);

        if (send_transfer_part(part_filename,
                               current_part,
                               parts) != 0) {
            return -1;
        }

        if (current_part == parts) {
            printf("\nAll sourcebook parts transferred.\n");
            break;
        }

        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|            Sourcebook Transfer              |\n"
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

static int sourcebook_get_title(const char *url,
                                char *title,
                                size_t title_size);

void sourcebooks_lookup(void)
{
    char query[256];
    char urls[5][512];
    int count;
    int i;
    int choice;

    printf("\nInternet History Sourcebooks\n\n");
    printf("Search for: ");

    if (fgets(query, sizeof(query), stdin) == NULL) {
        return;
    }

    query[strcspn(query, "\n")] = '\0';

    if (query[0] == '\0') {
        printf("Search cancelled.\n");
        return;
    }

    count = sourcebook_search(query, urls, 5);

    if (count <= 0) {
        printf("\nNo results found.\n");
        return;
    }

    printf("\nSearch results:\n\n");

    for (i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, urls[i]);
    }

    printf("\nSelect a source (1-%d, or 0 to cancel): ", count);

    if (scanf("%d", &choice) != 1) {
        printf("Invalid selection.\n");
        return;
    }

    getchar();

    if (choice == 0) {
        printf("Search cancelled.\n");
        return;
    }

    if (choice < 1 || choice > count) {
        printf("Invalid selection.\n");
        return;
    }



    char title[256];

if (sourcebook_get_title(urls[choice - 1],
                         title,
                         sizeof(title)) != 0) {
    snprintf(title,
             sizeof(title),
             "Internet History Sourcebook document");
}

sourcebook_download(
    title,
    urls[choice - 1]
);
}

static int sourcebook_get_title(const char *url,
                                char *title,
                                size_t title_size)
{
    char command[1024];
    FILE *file;
    char line[8192];
    int title_count = 0;

    snprintf(command, sizeof(command),
             "curl -L -s \"%s\" -o \"%s\"",
             url,
             SOURCEBOOK_RAW_FILE);

    if (system(command) != 0) {
        return -1;
    }

    file = fopen(SOURCEBOOK_RAW_FILE, "r");

    if (file == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *start;
        char *end;

        start = strstr(line, "<p class=\"H_Title\">");

        if (start == NULL) {
            continue;
        }

        title_count++;

        if (title_count != 2) {
            continue;
        }

        start += strlen("<p class=\"H_Title\">");

        end = strstr(start, "</p>");

        if (end == NULL) {
            fclose(file);
            return -1;
        }

        while (*start == ' ' || *start == '\t') {
            start++;
        }

        *end = '\0';

        snprintf(title, title_size, "%s", start);

        fclose(file);

        return 0;
    }

    fclose(file);

    return -1;
}