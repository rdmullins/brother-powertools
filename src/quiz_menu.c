#include <stdio.h>
#include <stdlib.h>

#include "quiz_menu.h"

void quiz_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|              brother PowerTools             |\n"
            "|                     Quiz                    |\n"
            "+---------------------------------------------+\n"
            "|  1. State Capitals                          |\n"
            "|  2. U.S. Presidents                         |\n"
            "|  3. Star Trek                               |\n"
            "|  4. Morse Code                              |\n"
            "|  5. European Capitals                       |\n"
            "|  6. Asian Capitals                          |\n"
            "|  7. Shakespeare                             |\n"
            "|  8. Middle-earth                            |\n"
            "|  9. Retrocomputing: Computer -> CPU         |\n"
            "| 10. Retrocomputing: CPU -> Computer         |\n"
            "| 11. Return to Games                         |\n"
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
                system("quiz state capital");
                break;

            case 2:
                system("quiz president term");
                break;

            case 3:
                system("quiz star trek");
                break;

            case 4:
                system("quiz clear morse");
                break;

            case 5:
                system("quiz european capital");
                break;

            case 6:
                system("quiz asian capital");
                break;

            case 7:
                system("quiz lines work");
                break;

            case 8:
                system("quiz middle-earth capital");
                break;

            case 9:
    system("quiz -i $HOME/brother-powertools/data/quiz/index computer cpu");
    break;

case 10:
    system("quiz -i $HOME/brother-powertools/data/quiz/index cpu computer");
    break;

case 11:
    return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}