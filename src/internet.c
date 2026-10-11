#include <stdio.h>
#include <stdlib.h>

#include "internet.h"
#include "wikipedia.h"
#include "wiktionary.h"
#include "gutenberg.h"
#include "sourcebooks.h"
#include "npr.h"
#include "gopher.h"
#include "infokat.h"


/* ---------------------------------------------------------
 * BBS Menu
 * --------------------------------------------------------- */

static void bbs_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|             brother PowerTools              |\n"
            "|                    BBSes                    |\n"
            "+---------------------------------------------+\n"
            "|  1. Private Heaven II BBS                   |\n"
            "|  2. Level29 BBS                              |\n"
            "|  3. Particles! BBS                           |\n"
            "|  4. Back                                     |\n"
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
                system("telnet bbs.privateheaven.com 6400");
                break;

            case 2:
                system("telnet bbs.fozztexx.com");
                break;

            case 3:
                system("telnet particlesbbs.dyndns.org 6400");
                break;

            case 4:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}


/* ---------------------------------------------------------
 * Internet Menu
 * --------------------------------------------------------- */

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
            "|  5. NPR News                                |\n"
            "|  6. InfoKat (UK Library Catalog)             |\n"
            "|  7. Gopher                                   |\n"
            "|  8. BBSes                                    |\n"
            "|  9. Back                                     |\n"
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
                npr_lookup();
                break;

            case 6:
                infokat_lookup();
                break;

            case 7:
                gopher_menu();
                break;

            case 8:
                bbs_menu();
                break;

            case 9:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}