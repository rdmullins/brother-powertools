#include <stdio.h>

#include "catalog_find.h"
#include "catalog.h"

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

    printf("\n");
    printf("Catalog Record\n");
    printf("--------------\n");

    catalog_display(&results[choice - 1]);
}