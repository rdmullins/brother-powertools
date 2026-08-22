#include <stdio.h>

#include "internet.h"
#include "wikipedia.h"

void internet_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|                  Internet                   |\n"
            "+---------------------------------------------+\n"
            "|  1. Wikipedia                               |\n"
            "|  2. Back                                    |\n"
            "+---------------------------------------------+\n"
            "\n"
            "Enter your choice: "
        );

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");

            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard invalid input */
            }

            continue;
        }

        getchar();

        switch (choice) {
            case 1:
                wikipedia_lookup();
                break;

            case 2:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}