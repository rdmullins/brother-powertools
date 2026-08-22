#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wikipedia.h"
#include "transfer.h"

#define WIKIPEDIA_API_URL \
    "https://en.wikipedia.org/w/api.php?action=query" \
    "&prop=extracts&explaintext=1&exsectionformat=wiki" \
    "&titles=%s&format=json"

#define WIKIPEDIA_JSON_FILE "/tmp/powertools-wikipedia.json"
#define WIKIPEDIA_TEXT_FILE "/tmp/powertools-wikipedia.txt"

static long get_file_size(const char *filename)
{
    FILE *file;
    long size;

    file = fopen(filename, "rb");

    if (file == NULL) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    size = ftell(file);

    fclose(file);

    return size;
}

static int json_extract_text(const char *input, const char *output)
{
    FILE *source;
    FILE *destination;
    int c;
    int in_extract = 0;
    int escaped = 0;

    source = fopen(input, "r");
    if (source == NULL) {
        perror("Unable to open Wikipedia API response");
        return -1;
    }

    destination = fopen(output, "w");
    if (destination == NULL) {
        perror("Unable to create Wikipedia text file");
        fclose(source);
        return -1;
    }

    /*
     * Find the "extract" JSON property.
     */
    while ((c = fgetc(source)) != EOF) {
        if (!in_extract) {
            if (c == '"') {
                char key[16];
                int i = 0;

                while ((c = fgetc(source)) != EOF &&
                       c != '"' &&
                       i < (int)sizeof(key) - 1) {
                    key[i++] = (char)c;
                }

                key[i] = '\0';

                if (strcmp(key, "extract") == 0) {
                    c = fgetc(source);

                    while (c != EOF && c != ':') {
                        c = fgetc(source);
                    }

                    if (c != EOF) {
                        c = fgetc(source);

                        while (c != EOF && c != '"') {
                            c = fgetc(source);
                        }

                        if (c == '"') {
                            in_extract = 1;
                        }
                    }
                }
            }
        } else {
            if (escaped) {
                switch (c) {
                    case '"':
                        fputc('"', destination);
                        break;

                    case '\\':
                        fputc('\\', destination);
                        break;

                    case '/':
                        fputc('/', destination);
                        break;

                    case 'n':
                        fputc('\n', destination);
                        break;

                    case 'r':
                        break;

                    case 't':
                        fputc('\t', destination);
                        break;

                    case 'u': {
                        char hex[5];
                        char *end;
                        unsigned long value;

                        for (int i = 0; i < 4; i++) {
                            c = fgetc(source);

                            if (c == EOF) {
                                fclose(destination);
                                fclose(source);
                                return -1;
                            }

                            hex[i] = (char)c;
                        }

                        hex[4] = '\0';

                        value = strtoul(hex, &end, 16);

                        if (end == hex + 4 && value <= 0x7f) {
                            fputc((int)value, destination);
                        } else if (end == hex + 4 &&
                                   value >= 0x80 &&
                                   value <= 0x7ff) {
                            fputc(0xc0 | (value >> 6), destination);
                            fputc(0x80 | (value & 0x3f), destination);
                        } else if (end == hex + 4 &&
                                   value >= 0x800 &&
                                   value <= 0xffff) {
                            fputc(0xe0 | (value >> 12), destination);
                            fputc(0x80 | ((value >> 6) & 0x3f), destination);
                            fputc(0x80 | (value & 0x3f), destination);
                        }

                        break;
                    }

                    default:
                        fputc(c, destination);
                        break;
                }

                escaped = 0;
                continue;
            }

            if (c == '\\') {
                escaped = 1;
                continue;
            }

            if (c == '"') {
                break;
            }

            fputc(c, destination);
        }
    }

    fclose(destination);
    fclose(source);

    return in_extract ? 0 : -1;
}

void wikipedia_lookup(void)
{
    char article[256];
    char encoded_article[256];
    char url[768];
    char command[1024];
    int choice;

    printf("\n");
    printf("+---------------------------------------------+\n");
    printf("|                  Wikipedia                  |\n");
    printf("+---------------------------------------------+\n");
    printf("\n");
    printf("Enter article name (use underscores for spaces):\n");
    printf("> ");

    if (fgets(article, sizeof(article), stdin) == NULL) {
        return;
    }

    article[strcspn(article, "\n")] = '\0';

    if (article[0] == '\0') {
        printf("No article name entered.\n");
        return;
    }

    /*
     * For Version 1, the user supplies a Wikipedia page title.
     * Underscores are already valid in Wikipedia URLs.
     */
    snprintf(encoded_article,
             sizeof(encoded_article),
             "%s",
             article);

    snprintf(url,
             sizeof(url),
             WIKIPEDIA_API_URL,
             encoded_article);

    snprintf(command,
             sizeof(command),
             "curl -L -s '%s' -o '%s'",
             url,
             WIKIPEDIA_JSON_FILE);

    printf("\nFetching Wikipedia article...\n");

    if (system(command) != 0) {
        printf("Unable to retrieve Wikipedia article.\n");
        return;
    }

    if (json_extract_text(WIKIPEDIA_JSON_FILE,
                           WIKIPEDIA_TEXT_FILE) != 0) {
        printf("Unable to extract article text.\n");
        return;
    }

    long article_size;
size_t max_size;
int parts;

article_size = get_file_size(WIKIPEDIA_TEXT_FILE);

if (article_size < 0) {
    printf("Unable to determine article size.\n");
    return;
}

printf("\n");
printf("Article retrieved successfully.\n");
printf("Article size: %ld bytes\n", article_size);
printf("\n");
printf("1. Send article to PowerNote\n");
printf("2. Back\n");
printf("\n");
printf("Enter your choice: ");

if (scanf("%d", &choice) != 1) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard invalid input */
    }

    printf("Invalid input.\n");
    return;
}

getchar();

switch (choice) {
    case 1:
        printf("\n");
        printf("How much free PowerNote memory is available?\n");
        printf("Enter maximum part size in bytes: ");

        if (scanf("%zu", &max_size) != 1) {
            int c;

            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard invalid input */
            }

            printf("Invalid size.\n");
            return;
        }

        getchar();

        if (max_size == 0) {
            printf("Size must be greater than zero.\n");
            return;
        }

        printf("\n");

        if ((long)max_size >= article_size) {
            printf("The article will fit in one part.\n");
        } else {
            parts = split_file(
                WIKIPEDIA_TEXT_FILE,
                "/tmp/powertools-wikipedia",
                max_size
            );

            if (parts < 0) {
                printf("Unable to split article.\n");
                return;
            }

            printf("The article will be sent in %d parts.\n", parts);
        }

        break;

    case 2:
        return;

    default:
        printf("Invalid option.\n");
        return;
}

    switch (choice) {
        case 1: {
            char part_filename[512];

            printf("\n");
            printf("How much free PowerNote memory is available?\n");
            printf("Enter maximum part size in bytes: ");

            if (scanf("%zu", &max_size) != 1) {
                int c;

                while ((c = getchar()) != '\n' && c != EOF) {
                    /* discard invalid input */
                }

                printf("Invalid size.\n");
                return;
            }

            getchar();

            if (max_size == 0) {
                printf("Size must be greater than zero.\n");
                return;
            }

            parts = split_file(
                WIKIPEDIA_TEXT_FILE,
                "/tmp/powertools-wikipedia",
                max_size
            );

            if (parts < 0) {
                printf("Unable to split article.\n");
                return;
            }

            printf("\n");
            printf("The article will be sent in %d parts.\n", parts);

            snprintf(part_filename,
                    sizeof(part_filename),
                    "/tmp/powertools-wikipedia_%02d.txt",
                    1);

            if (send_transfer_part(part_filename, 1, parts) != 0) {
                return;
            }

            printf("\n");
            printf("Part 1 is complete.\n");
            printf("Exit the Brother terminal and save the file now.\n");
            printf("\n");
            printf("Press ENTER to return to the Internet menu.");

            getchar();

            break;
        }

        case 2:
            break;

        default:
            printf("Invalid option.\n");
            break;
    }
}