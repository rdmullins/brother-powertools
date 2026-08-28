#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "catalog_add.h"
#include "catalog.h"
#include "openlibrary.h"

static int catalog_add_select_subjects(const OpenLibraryBook *book,
                                       CatalogRecord *record)
{
    char input[512];
    char *token;

    if (book == NULL || record == NULL) {
        return -1;
    }

    if (book->subject_count == 0) {
        printf("\nNo subjects were returned by Open Library.\n");
        return 0;
    }

    printf("\nSubjects found\n");
    printf("--------------\n");

    for (int i = 0; i < book->subject_count; i++) {
        printf("%2d. %s\n",
               i + 1,
               book->subjects[i]);
    }

    printf("\n");
    printf("Select subjects (e.g. 5,6,8): ");

    if (scanf("%511s", input) != 1) {
        return -1;
    }

    token = strtok(input, ",");

    while (token != NULL) {
        int number = atoi(token);

        if (number >= 1 &&
            number <= book->subject_count) {

            const char *subject =
                book->subjects[number - 1];

            if (record->subjects[0] != '\0') {
                strncat(record->subjects,
                        ";",
                        sizeof(record->subjects) -
                        strlen(record->subjects) - 1);
            }

            strncat(record->subjects,
                    subject,
                    sizeof(record->subjects) -
                    strlen(record->subjects) - 1);
        }

        token = strtok(NULL, ",");
    }

    return 0;
}

static void catalog_add_by_isbn(void)
{
    char isbn[32];
    OpenLibraryBook book;

    printf("\n");
    printf("Add Book by ISBN\n");
    printf("----------------\n");
    printf("ISBN: ");

    if (scanf("%31s", isbn) != 1) {
        printf("\nInvalid ISBN.\n");
        return;
    }

    printf("\nLooking up ISBN...\n");

    if (openlibrary_lookup_isbn(isbn, &book) != 0) {
        printf("\nNo book found for ISBN %s.\n", isbn);
        return;
    }

    printf("\nBook Found\n");
    printf("----------\n");
    printf("ISBN:      %s\n", book.isbn);
    printf("Author:    %s\n", book.author);
    printf("Title:     %s\n", book.title);
    printf("Publisher: %s\n", book.publisher);
    printf("Date:      %s\n", book.publish_date);
    printf("Subjects found: %d\n", book.subject_count);

    printf("\n");
    printf("This is the information returned by Open Library.\n");

        CatalogRecord record;

    memset(&record, 0, sizeof(record));

    strncpy(record.isbn,
            book.isbn,
            sizeof(record.isbn) - 1);

    strncpy(record.author,
            book.author,
            sizeof(record.author) - 1);

    strncpy(record.title,
            book.title,
            sizeof(record.title) - 1);

    strncpy(record.publisher,
            book.publisher,
            sizeof(record.publisher) - 1);

    strncpy(record.year,
            book.publish_date,
            sizeof(record.year) - 1);

    if (catalog_add_select_subjects(&book, &record) != 0) {
        printf("\nUnable to select subjects.\n");
        return;
    }

    printf("\nSelected subjects:\n%s\n",
        record.subjects);

            char location[CATALOG_MAX_LOCATION];
    char id[32];
    char answer;

    printf("\nPhysical location: ");

    if (scanf(" %127[^\n]", location) != 1) {
        printf("\nInvalid location.\n");
        return;
    }

    if (catalog_next_id("data/catalog.db",
                        id,
                        sizeof(id)) != 0) {
        printf("\nUnable to determine the next catalog ID.\n");
        return;
    }

    strncpy(record.id,
            id,
            sizeof(record.id) - 1);

    record.id[sizeof(record.id) - 1] = '\0';

    strncpy(record.location,
            location,
            sizeof(record.location) - 1);

    record.location[sizeof(record.location) - 1] = '\0';

    printf("\nCatalog Record\n");
    printf("--------------\n");
    catalog_display(&record);

    printf("\nSave this record? (y/n): ");

    if (scanf(" %c", &answer) != 1) {
        return;
    }

    if (answer != 'y' && answer != 'Y') {
        printf("\nBook not added.\n");
        return;
    }

    if (catalog_save("data/catalog.db",
                     &record) != 0) {
        printf("\nUnable to save book.\n");
        return;
    }

    printf("\nBook added to catalog as %s.\n",
           record.id);

}

void catalog_add_menu(void)
{
    int choice;

    do {
        printf("\n");
        printf("Add Book\n");
        printf("--------\n");
        printf("1. By ISBN\n");
        printf("2. Enter Manually\n");
        printf("3. Back\n");
        printf("\n");
        printf("Select: ");

        if (scanf("%d", &choice) != 1) {
            int character;

            while ((character = getchar()) != '\n' &&
                   character != EOF) {
                /* discard invalid input */
            }

            choice = 0;
        }

        switch (choice) {
        case 1:
            catalog_add_by_isbn();
            break;

        case 2:
            printf("\nManual entry coming soon.\n");
            break;

        case 3:
            break;

        default:
            printf("\nInvalid selection.\n");
            break;
        }

    } while (choice != 3);
}