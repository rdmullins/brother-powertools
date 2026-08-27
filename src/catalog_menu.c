#include <stdio.h>

#include "catalog_menu.h"
#include "catalog_add.h"
#include "catalog.h"

void catalog_menu(void)
{
    int choice;

    do {
        printf("\n");
        printf("Catalog\n");
        printf("-------\n");
        printf("1. Add Book\n");
        printf("2. Find Book\n");
        printf("3. Print Catalog Cards\n");
        printf("4. Back\n");
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
            catalog_add_menu();
            break;

        case 2:
        {
            char id[32];
            CatalogRecord record;

            printf("\nFind Book\n");
            printf("---------\n");
            printf("Catalog ID: ");

            if (scanf("%31s", id) != 1) {
                printf("Invalid input.\n");
                break;
            }

            if (catalog_load("data/catalog.db",
                            id,
                            &record) == 0) {
                catalog_display(&record);
            } else {
                printf("Book not found.\n");
            }

            break;
        }

        case 3:
            printf("\nCatalog card printing coming soon.\n");
            break;

        case 4:
            break;

        default:
            printf("\nInvalid selection.\n");
            break;
        }

    } while (choice != 4);
}