#include <stdio.h>
#include <stdlib.h>

#include "games.h"

void game_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|              brother PowerTools             |\n"
            "|                    Games                    |\n"
            "+---------------------------------------------+\n"
            "|  1. Adventure                               |\n"
            "|  2. Star Trek                               |\n"
            "|  3. Hunt the Wumpus                         |\n"
            "|  4. Oregon Trail                            |\n"
            "|  5. Exit                                    |\n"
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

        switch (choice) {
            case 1:
                system("adventure");
                break;

            case 2:
                system("$HOME/.local/bin/trek");
                break;

            case 3:
                system("wump");
                break;

            case 4:
                system("$HOME/.local/bin/oregon-local");
                break;

            case 5:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
