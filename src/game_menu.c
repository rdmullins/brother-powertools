#include <stdio.h>
#include <stdlib.h>

#include "games.h"
#include "quiz_menu.h"
#include "game_2048.h"

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
            "|  5. Zork                                    |\n"
            "|  6. Quiz                                    |\n"
            "|  7. Textle                                  |\n"
            "|  8. Dopewars                                |\n"
            "|  9. 2048                                    |\n"
            "|  10. Return to Main Menu                     |\n"
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
                printf ("  __________________          __\n");
                printf ("  \\_____NCC-1701___|)____.---'--'---.____\n");
                printf ("               ||    \\----.________.----/\n");
                printf ("               ||     / /    '--'\n");
                printf ("               ||    / /\n");
                printf ("             __||___/ /_\n");
                printf ("            |___         \\\(>\n");
                printf ("                '--------'\n");

                system("$HOME/.local/bin/trek"); 
                break;

            case 3:
                system("wump");
                break;

            case 4:
                system("$HOME/.local/bin/oregon-local");
                break;

            case 5:
                system("/usr/games/dfrotz $HOME/Zork/Zork1.dat");
                break;

            case 6:
                quiz_menu();
                break;

            case 7:
                system("cd /home/roger/textle && ./textle");
                break;

            case 8:
                system("cd /home/roger/src/dopewars && ./src/dopewars -T -f /tmp/dopewars.sco");
                break;

            case 9:
                game_2048();
                break;

            case 10:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
