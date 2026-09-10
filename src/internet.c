#include <stdio.h>
#include <stdlib.h>

#include "internet.h"
#include "wikipedia.h"
#include "wiktionary.h"

void internet_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|             brother PowerTools              |\n"
            "|                  Internet                   |\n"
            "+---------------------------------------------+\n"
            "|  1. Wikipedia                               |\n"
            "|  2. Wiktionary                              |\n"
            "|  3. Private Heaven II BBS                   |\n"
            "|  4. Level29 BBS                             |\n"
            "|  5. Particles! BBS                          |\n"
            "|  6. Back                                    |\n"
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
                wiktionary_lookup();
                break;

            case 3:
                system("nc bbs.sydbolton.ca 6502");
                break;

            case 4:
                system("telnet bbs.fozztexx.com"); 
                break;

            case 5:     
                system("telnet bbs.particles.net 6400");
                break;
            
            case 6:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}