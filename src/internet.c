#include <stdio.h>
#include <stdlib.h>

#include "internet.h"
#include "wikipedia.h"

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
            "|  2. Private Heaven II BBS                   |\n"
            "|  3. Level29 BBS                             |\n"
            "|  4. Back                                    |\n"
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
                system("nc bbs.sydbolton.ca 6502");
                break;

            case 3:
                system("telnet bbs.fozztexx.com"); 
                break;

            case 4:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}