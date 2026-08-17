#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mail_browser.h"

#define MAX_MESSAGES 1000

void mail_browser(void)
{
    FILE *fp;
    char line[256];
    char command;
    int message_number;
    int messages[MAX_MESSAGES];
    int message_count = 0;
    int current_index = 0;
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
        "/usr/bin/mh/scan +inbox -width 76 -format '%4(msg) %02(mon{date})/%02(mday{date}) %-20(friendly{from}) %{subject}\\n'",
        "r"
    );

    if (fp == NULL) {
        printf("Unable to access mailbox.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        int msg;

        printf("%s", line);

        if (sscanf(line, "%d", &msg) == 1) {
            if (message_count < MAX_MESSAGES) {
                messages[message_count] = msg;
                message_count++;
            }
        }
    }

    pclose(fp);

    if (message_count == 0) {
        printf(
            "+------------------------------------------------------------------------------+\n"
            "| No messages in inbox.                                                       |\n"
            "+------------------------------------------------------------------------------+\n"
        );

        printf("\nPress Enter to return...");
        getchar();
        return;
    }

    printf(
        "+------------------------------------------------------------------------------+\n"
        "|                       R Read   A Archive   D Delete   Q Quit                |\n"
        "+------------------------------------------------------------------------------+\n"
    );

    printf("\nCommand: ");
    scanf(" %c", &command);

    switch (command) {

        case 'r':
        case 'R':
                  /*
                * Display the selected message's useful headers.
                */  
        printf("Message number: ");

        if (scanf("%d", &message_number) != 1) {
            printf("Invalid message number.\n");
            return;
        }

        int found = 0;

        for (int i = 0; i < message_count; i++) {
            if (messages[i] == message_number) {
                current_index = i;
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Message %d does not exist in the inbox.\n", message_number);
            printf("\nPress Enter to return...");
            getchar();
            getchar();
            break;
        }
            while (1) {


                char command_buffer[4096];
                char message_path[1024];
                char body_file[] = "/tmp/powertools-body-XXXXXX.html";
                int body_fd;
                const char *home;

                /*
                * Determine the user's home directory.
                */
                home = getenv("HOME");

                if (home == NULL) {
                    printf("\nUnable to determine home directory.\n");
                    break;
                }

                snprintf(
                    message_path,
                    sizeof(message_path),
                    "%s/Mail/inbox/%d",
                    home,
                    message_number
                );

                /*
                * Display the message headers.
                */
                snprintf(
                    command_buffer,
                    sizeof(command_buffer),
                    "/usr/lib/mh/mhl -form \"%s\" \"%s\"",
                    format_path,
                    message_path
                );

                system(command_buffer);

                printf("\n");

                /*
                * Create a temporary file for the decoded message body.
                */
                body_fd = mkstemps(body_file, 5);

                if (body_fd == -1) {
                    printf("Unable to create temporary message file.\n");
                    break;
                }

                close(body_fd);

                /*
                * Extract the preferred MIME representation.
                * nmh prefers text/plain and falls back to text/html.
                */
                snprintf(
                    command_buffer,
                    sizeof(command_buffer),
                    "/usr/bin/mh/mhstore +inbox %d -prefer text/plain -outfile \"%s\"",
                    message_number,
                    body_file
                );

                system(command_buffer);

                /*
                * Render the message body for the terminal.
                */
                snprintf(
                    command_buffer,
                    sizeof(command_buffer),
                    "lynx -dump -nonumbers -nolist \"%s\"",
                    body_file
                );

                system(command_buffer);

                /*
                * Remove the temporary file.
                */
                unlink(body_file);


                printf(
                    "\n"
                    "N Next   P Previous   Q Quit\n"
                    "Command: "
                );

                scanf(" %c", &command);

                switch (command) {

                    case 'n':
                    case 'N':
                        /*
                         * Find the current message in the scan list,
                         * then move to the next actual message.
                         */
                        for (int i = 0; i < message_count; i++) {
                            if (messages[i] == message_number) {
                                if (i < message_count - 1) {
                                    message_number = messages[i + 1];
                                } else {
                                    printf("\nAlready at the last message.\n");
                                }
                                break;
                            }
                        }
                        break;

                    case 'p':
                    case 'P':
                        /*
                         * Find the current message in the scan list,
                         * then move to the previous actual message.
                         */
                        for (int i = 0; i < message_count; i++) {
                            if (messages[i] == message_number) {
                                if (i > 0) {
                                    message_number = messages[i - 1];
                                } else {
                                    printf("\nAlready at the first message.\n");
                                }
                                break;
                            }
                        }
                        break;

                    case 'q':
                    case 'Q':
                        goto done_reading;

                    default:
                        printf("\nUnknown command.\n");
                        break;
                }
            }

        done_reading:
            break;

        case 'a':
        case 'A':
            printf("Message number: ");

            if (scanf("%d", &message_number) != 1) {
                printf("Invalid message number.\n");
                return;
            }

            printf("Archive message %d? [y/N]: ", message_number);

            {
                char confirm;
                scanf(" %c", &confirm);

                if (confirm != 'y' && confirm != 'Y') {
                    printf("Message not archived.\n");
                    break;
                }
            }

            {
                char refile_command[256];

                snprintf(
                    refile_command,
                    sizeof(refile_command),
                    "/usr/bin/mh/refile +inbox %d +archive",
                    message_number
                );

                system(refile_command);
            }

            printf("Message %d archived.\n", message_number);

            printf("\nPress Enter to return...");
            getchar();
            getchar();
            break;

        case 'd':
        case 'D':
            printf("Message number: ");

            if (scanf("%d", &message_number) != 1) {
                printf("Invalid message number.\n");
                return;
            }

            printf("Delete message %d? [y/N]: ", message_number);

            {
                char confirm;
                scanf(" %c", &confirm);

                if (confirm != 'y' && confirm != 'Y') {
                    printf("Message not deleted.\n");
                    break;
                }
            }

            {
                char rmm_command[256];

                snprintf(
                    rmm_command,
                    sizeof(rmm_command),
                    "/usr/bin/mh/rmm +inbox %d",
                    message_number
                );

                system(rmm_command);
            }

            printf("Message %d deleted.\n", message_number);

            printf("\nPress Enter to return...");
            getchar();
            getchar();
            break;

        case 'q':
        case 'Q':
            return;

        case 'n':
        case 'N':
            if (current_index < message_count - 1) {
                current_index++;
                message_number = messages[current_index];

                printf("\nNext message: %d\n", message_number);
            } else {
                printf("\nAlready at the last message.\n");
            }

            printf("Press Enter to return...");
            getchar();
            getchar();
            break;

        case 'p':
        case 'P':
            if (current_index > 0) {
                current_index--;
                message_number = messages[current_index];

                printf("\nPrevious message: %d\n", message_number);
            } else {
                printf("\nAlready at the first message.\n");
            }

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