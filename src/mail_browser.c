#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mail_browser.h"

void mail_browser(void)
{
    FILE *fp;
    char line[256];
    char command;
    int message_number;
    char cwd[512];
    char format_path[1024];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("Unable to determine PowerTools directory.\n");
        return;
    }

    snprintf(
        format_path,
        sizeof(format_path),
        "%s/config/mhl.headers",
        cwd
    );

    printf(
        "\n"
        "+------------------------------------------------------------------------------+\n"
        "|                                  INBOX                                       |\n"
        "+------------------------------------------------------------------------------+\n"
    );

    fp = popen(
        "/usr/bin/mh/scan -format '%4(msg) %02(mon{date})/%02(mday{date}) %-20(friendly{from}) %{subject}\\n'",
        "r"
    );

    if (fp == NULL) {
        printf("Unable to access mailbox.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
    }

    pclose(fp);

    printf(
        "+------------------------------------------------------------------------------+\n"
        "| N Next   P Previous   R Read   Q Quit                                        |\n"
        "+------------------------------------------------------------------------------+\n"
    );

    printf("\nCommand: ");
    scanf(" %c", &command);

    switch (command) {

        case 'r':
        case 'R':
            printf("Message number: ");

            if (scanf("%d", &message_number) != 1) {
                printf("Invalid message number.\n");
                return;
            }

            {
                char show_command[2048];

                snprintf(
                    show_command,
                    sizeof(show_command),
                    "/usr/bin/mh/show -noheader -prefer text/plain -form \"%s\" %d",
                    format_path,
                    message_number
                );

                system(show_command);
            }

            printf("\nPress Enter to return...");
            getchar();
            getchar();
            break;

        case 'q':
        case 'Q':
            return;

        case 'n':
        case 'N':
            printf("\nPagination isn't implemented yet.\n");
            printf("Press Enter to return...");
            getchar();
            getchar();
            break;

        case 'p':
        case 'P':
            printf("\nPagination isn't implemented yet.\n");
            printf("Press Enter to return...");
            getchar();
            getchar();
            break;

        default:
            printf("\nUnknown command.\n");
            printf("Press Enter to return...");
            getchar();
            getchar();
            break;
    }
}