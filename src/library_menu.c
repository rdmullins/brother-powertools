#include <stdio.h>

#include "library_menu.h"

void library_menu(void)
{
    int choice;

    do {
        printf("\n");
        printf("Library\n");
        printf("-------\n");
        printf("1. Catalog\n");
        printf("2. Bibliography\n");
        printf("3. Research Notes\n");
        printf("4. Print Cards\n");
        printf("5. Back\n");
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
            printf("\nCatalog functionality coming soon.\n");
            break;

        case 2:
            printf("\nBibliography functionality coming soon.\n");
            break;

        case 3:
            printf("\nResearch note functionality coming soon.\n");
            break;

        case 4:
            printf("\nCard printing functionality coming soon.\n");
            break;

        case 5:
            break;

        default:
            printf("\nInvalid selection.\n");
            break;
        }

    } while (choice != 5);
}