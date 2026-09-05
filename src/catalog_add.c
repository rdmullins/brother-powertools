#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog_add.h"
#include "catalog.h"
#include "openlibrary.h"
#include "card_set.h"
#include "cards.h"

static void clear_input_line(void)
{
    int character;

    while ((character = getchar()) != '\n' &&
           character != EOF) {
        /* discard remaining input */
    }
}

static void read_field(const char *prompt,
                       char *buffer,
                       size_t buffer_size)
{
    printf("%s", prompt);

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    if (strchr(buffer, '\n') == NULL) {
        clear_input_line();
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';
}

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
    CatalogRecord record;
    char location[CATALOG_MAX_LOCATION];
    char id[32];
    char answer;

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

static void catalog_add_manual(void)
{
    CatalogRecord record;
    char id[32];
    char answer;

    memset(&record, 0, sizeof(record));

    printf("\n");
    printf("Add Book Manually\n");
    printf("-----------------\n");

    /*
     * catalog_add_menu() entered this function via scanf(),
     * so consume its trailing newline before using fgets().
     */
    clear_input_line();

    read_field("ISBN: ",
               record.isbn,
               sizeof(record.isbn));

    read_field("Author: ",
               record.author,
               sizeof(record.author));

    read_field("Title: ",
               record.title,
               sizeof(record.title));

    read_field("Place: ",
               record.place,
               sizeof(record.place));

    read_field("Publisher: ",
               record.publisher,
               sizeof(record.publisher));

    read_field("Year: ",
               record.year,
               sizeof(record.year));

    read_field("Subjects: ",
               record.subjects,
               sizeof(record.subjects));

    read_field("Physical location: ",
               record.location,
               sizeof(record.location));

    if (record.title[0] == '\0') {
        printf("\nA title is required. Book not added.\n");
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
            catalog_add_manual();
            break;

        case 3:
            break;

        default:
            printf("\nInvalid selection.\n");
            break;
        }

    } while (choice != 3);
}