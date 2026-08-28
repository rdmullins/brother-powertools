#include <stdio.h>
#include <string.h>

#include "catalog_add.h"
#include "catalog.h"
#include "openlibrary.h"

static void catalog_add_by_isbn(void)
{
    char isbn[32];
    char location[CATALOG_MAX_LOCATION];
    char id[32];
    char answer;
    CatalogRecord record;

    printf("\n");
    printf("Add Book by ISBN\n");
    printf("----------------\n");
    printf("ISBN: ");

    if (scanf("%31s", isbn) != 1) {
        printf("\nInvalid ISBN.\n");
        return;
    }

    printf("\nLooking up ISBN...\n");

    if (openlibrary_lookup_isbn(isbn, &record) != 0) {
        printf("\nNo book found for ISBN %s.\n", isbn);
        return;
    }

    printf("\nBook Found\n");
    printf("----------\n");
    catalog_display(&record);

    printf("\n");
    printf("Add this book to the catalog? (y/n): ");

    if (scanf(" %c", &answer) != 1) {
        return;
    }

    if (answer != 'y' && answer != 'Y') {
        printf("\nBook not added.\n");
        return;
    }

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

    strcpy(record.id, id);
    strcpy(record.location, location);

    printf("\nReady to add:\n");
    catalog_display(&record);

    printf("\n");
    printf("Save this record? (y/n): ");

    if (scanf(" %c", &answer) != 1) {
        return;
    }

    if (answer != 'y' && answer != 'Y') {
        printf("\nBook not added.\n");
        return;
    }

    if (catalog_save("data/catalog.db", &record) != 0) {
        printf("\nUnable to save book.\n");
        return;
    }

    printf("\nBook added to catalog as %s.\n", record.id);
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