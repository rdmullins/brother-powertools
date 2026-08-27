#include <stdio.h>

#include "catalog_add.h"

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
            printf("\nISBN lookup coming soon.\n");
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