#include <stdio.h>
#include <stdlib.h>

#include "internet.h"
#include "wikipedia.h"
#include "wiktionary.h"
#include "gutenberg.h"
#include "sourcebooks.h"

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
            "|  3. Project Gutenberg                       |\n"
            "|  4. Internet History Sourcebooks            |\n"
            "|  5. Private Heaven II BBS                   |\n"
            "|  6. Level29 BBS                             |\n"
            "|  7. Particles! BBS                          |\n"
            "|  8. Back                                    |\n"
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
                gutenberg_lookup();
                break;

            case 4:
                sourcebooks_lookup();
                break;

            case 5:
                system("telnet bbs.privateheaven.com 6400");
                break;

            case 6: 
                system("telnet bbs.fozztexx.com"); 
                break;

            case 7:     
                system("telnet particlesbbs.dyndns.org 6400");
                break;
            
            case 8:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}