#include <stdio.h>
#include <stdlib.h>

#include "mail_browser.h"
#include "mail.h"

void mail_menu(void)
{
    int choice;

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|                 PowerTools Mail             |\n"
            "+---------------------------------------------+\n"
            "|  1. Check Email                             |\n"
            "|  2. Compose Email                           |\n"
            "|  3. Back                                    |\n"
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
                system("/usr/bin/mh/inc");
                mail_browser();
                break;

            case 2:
                system("comp");
                break;

            case 3:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
