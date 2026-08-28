#include <stdio.h>

#include "catalog_find.h"
#include "catalog.h"
#include "card_set.h"
#include "cards.h"

#define CATALOG_SEARCH_MAX_RESULTS 20

static void display_result_summary(const CatalogRecord *record,
                                   int number)
{
    printf("%d. %s\n",
           number,
           record->title);

    printf("   %s\n",
           record->author);

    printf("   %s, %s\n",
           record->publisher,
           record->year);

    printf("   %s\n",
           record->location);
}

static void catalog_write_cards(const CatalogRecord *record)
{
    CardSet set;
    char filename[256];

    if (record == NULL) {
        return;
    }

    if (cards_build_catalog(record, &set) != 0) {
        printf("\nUnable to build catalog cards.\n");
        return;
    }

    printf("\nOutput file: ");

    if (scanf(" %255[^\n]", filename) != 1) {
        printf("\nInvalid filename.\n");
        return;
    }

    if (card_set_write(&set, filename) != 0) {
        printf("\nUnable to write catalog cards.\n");
        return;
    }

    printf("\nCatalog cards written to %s.\n", filename);
}

void catalog_find_menu(void)
{
    char query[256];
    CatalogRecord results[CATALOG_SEARCH_MAX_RESULTS];
    int count;
    int choice;

    printf("\n");
    printf("Find Book\n");
    printf("---------\n");
    printf("Search: ");

    if (scanf(" %255[^\n]", query) != 1) {
        printf("\nInvalid search.\n");
        return;
    }

    count = catalog_search("data/catalog.db",
                           query,
                           results,
                           CATALOG_SEARCH_MAX_RESULTS);

    if (count < 0) {
        printf("\nUnable to search catalog.\n");
        return;
    }

    if (count == 0) {
        printf("\nNo books found.\n");
        return;
    }

    printf("\nMatches\n");
    printf("-------\n");

    for (int i = 0; i < count; i++) {
        display_result_summary(&results[i], i + 1);
        printf("\n");
    }

    printf("Select a book (0 to cancel): ");

    if (scanf("%d", &choice) != 1) {
        printf("\nInvalid selection.\n");
        return;
    }

    if (choice == 0) {
        return;
    }

    if (choice < 1 || choice > count) {
        printf("\nInvalid selection.\n");
        return;
    }

CatalogRecord *record = &results[choice - 1];
int action;

do {
    printf("\n");
    printf("Catalog Record\n");
    printf("--------------\n");

    catalog_display(record);

    printf("\n");
    printf("1. View Record\n");
    printf("2. Write Catalog Cards\n");
    printf("3. Back\n");
    printf("\n");
    printf("Select: ");

    if (scanf("%d", &action) != 1) {
        int character;

        while ((character = getchar()) != '\n' &&
               character != EOF) {
            /* discard invalid input */
        }

        action = 0;
    }

    switch (action) {
    case 1:
        catalog_display(record);
        break;

    case 2:
        catalog_write_cards(record);
        break;

    case 3:
        break;

    default:
        printf("\nInvalid selection.\n");
        break;
    }

} while (action != 3);
}