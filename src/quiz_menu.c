#include <stdio.h>
#include <stdlib.h>

#include "quiz_menu.h"

void jeopardy_menu(void)
{
    int choice;


while (1) {
    printf(
        "\n"
        "+----------------------------------------------+\n"
        "|              brother PowerTools              |\n"
        "|                JEOPARDY! Prep                |\n"
        "+----------------------------------------------+\n"
        "|  1. Pavlovs                                  |\n"
        "|                                              |\n"
        "|  2. History              3. Geography        |\n"
        "|  4. Literature           5. Language & Words |\n"
        "|  6. Science & Nature     7. Film & TV        |\n"
        "|  8. People & Society     9. Wordplay & Mixed |\n"
        "| 10. Music & Performing  11. Art & Design     |\n"
        "| 12. Sports              13. Food & Drink     |\n"
        "| 14. Religion & Myth.    15. Culture & Gen.   |\n"
        "| 16. Business & Econ.    17. Math & Measure.  |\n"
        "| 18. Technology          19. Transportation   |\n"
        "| 20. General Knowledge                        |\n"
        "|                                              |\n"
        "| 21. Return to Quiz Menu                      |\n"
        "+----------------------------------------------+\n"
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
            system("quiz -i $HOME/brother-powertools/data/quiz/index -n 30 pavlov answer");
            break;

        case 2:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 history response");
            break;

        case 3:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 geography response");
            break;

        case 4:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 literature response");
            break;

        case 5:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 language_words response");
            break;

        case 6:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 science_nature response");
            break;

        case 7:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 film_television response");
            break;

        case 8:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 people_society response");
            break;

        case 9:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 wordplay_mixed response");
            break;

        case 10:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 music_performing_arts response");
            break;

        case 11:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 art_design response");
            break;

        case 12:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 sports response");
            break;

        case 13:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 food_drink response");
            break;

        case 14:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 religion_mythology response");
            break;

        case 15:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 culture_general response");
            break;

        case 16:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 business_economics response");
            break;

        case 17:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 math_measurement response");
            break;

        case 18:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 technology_inventions response");
            break;

        case 19:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 transportation response");
            break;

        case 20:
            system("quiz -i $HOME/brother-powertools/data/quiz/subjects.index -n 30 general_knowledge response");
            break;

        case 21:
            return;

        default:
            printf("Invalid option. Please try again.\n");
            break;
    }
}

}

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
            "|  0. JEOPARDY! Preparation                   |\n"
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
            case 0:
                jeopardy_menu();
                break;

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