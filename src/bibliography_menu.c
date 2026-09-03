#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "notes.h"
#include "bibliography.h"

#define BIB_DB "data/bibliography.db"
#define BIB_MAX_RESULTS 50

static void read_line(const char *prompt,
                      char *buffer,
                      size_t buffer_size)
{
    printf("%s", prompt);

    if (fgets(buffer, buffer_size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

static void display_search_result(const BibliographyRecord *record,
                                  int number)
{
    printf("\n%d. %s\n", number, record->id);
    printf("   %s\n", record->author);
    printf("   %s\n", record->title);
}

static void bibliography_search_menu(void)
{
    BibliographyRecord results[BIB_MAX_RESULTS];
    char query[256];
    char selection[32];
    char *end;
    long choice;
    int result_count;

    read_line("\nSearch bibliography: ",
              query,
              sizeof(query));

    if (query[0] == '\0') {
        printf("\nSearch cancelled.\n");
        return;
    }

    result_count = bibliography_search(BIB_DB,
                                       query,
                                       results,
                                       BIB_MAX_RESULTS);

    if (result_count < 0) {
        printf("\nUnable to search bibliography.\n");
        return;
    }

    if (result_count == 0) {
        printf("\nNo bibliography entries found.\n");
        return;
    }

    printf("\nBIBLIOGRAPHY RESULTS\n");
    printf("--------------------\n");

    for (int i = 0; i < result_count; i++) {
        display_search_result(&results[i], i + 1);
    }

    printf("\nSelect entry (1-%d, or 0 to cancel): ",
           result_count);

    if (fgets(selection, sizeof(selection), stdin) == NULL) {
        return;
    }

    choice = strtol(selection, &end, 10);

    if (end == selection || choice < 0 ||
        choice > result_count) {
        printf("\nInvalid selection.\n");
        return;
    }

    if (choice == 0) {
        return;
    }

    for (;;) {
        BibliographyRecord *record = &results[choice - 1];

        bibliography_display(record);

        printf("\n");
        printf("1. Add Note\n");
        printf("2. View Notes\n");
        printf("3. Back\n");
        printf("\nChoice: ");

        read_line("", selection, sizeof(selection));

    switch (selection[0]) {
    case '1':
        if (notes_add_for_bibliography(record->id) == 0) {
            printf("\nNote saved.\n");
        } else {
            printf("\nUnable to save note.\n");
        }
        break;

    case '2':
        printf("\nView Notes: coming soon.\n");
        break;

    case '3':
        return;

    default:
        printf("\nInvalid choice.\n");
        break;
    }
}
}

void bibliography_menu(void)
{
    char choice[16];

    for (;;) {
        printf("\n");
        printf("Bibliography\n");
        printf("------------\n");
        printf("1. Search\n");
        printf("2. Back\n");
        printf("\n");
        printf("Select: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            return;
        }

        switch (choice[0]) {
        case '1':
            bibliography_search_menu();
            break;

        case '2':
            return;

        default:
            printf("\nInvalid selection.\n");
            break;
        }
    }
}