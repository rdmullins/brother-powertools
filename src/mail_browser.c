#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "mail_browser.h"

#define MAX_MESSAGES 1000

/*
 * Archive a list of MH message numbers.
 */
static void archive_messages(int messages[], int count)
{
    char command[4096];
    int offset = 0;

    if (count <= 0) {
        printf("\nNo messages selected.\n");
        return;
    }

    offset += snprintf(
        command + offset,
        sizeof(command) - offset,
        "/usr/bin/mh/refile -src +inbox"
    );

    for (int i = 0; i < count && offset < (int)sizeof(command) - 32; i++) {
        offset += snprintf(
            command + offset,
            sizeof(command) - offset,
            " %d",
            messages[i]
        );
    }

    snprintf(
        command + offset,
        sizeof(command) - offset,
        " +archive"
    );

    if (system(command) != 0) {
        printf("\nUnable to archive messages.\n");
    } else {
        printf(
            "\n%d message%s archived.\n",
            count,
            count == 1 ? "" : "s"
        );
    }
}

/*
 * Determine whether a message with an MM/DD date is older than
 * the requested number of days.
 *
 * scan currently gives us month/day but not the year.
 * We assume the current year, unless that would put the date
 * in the future, in which case we treat it as December of
 * the previous year.
 */
static int message_is_older_than(
    int month,
    int day,
    int days
)
{
    time_t now;
    struct tm current;
    struct tm message_date;
    time_t message_time;
    double age;

    now = time(NULL);

    if (localtime_r(&now, &current) == NULL) {
        return 0;
    }

    memset(&message_date, 0, sizeof(message_date));

    message_date.tm_year = current.tm_year;
    message_date.tm_mon = month - 1;
    message_date.tm_mday = day;
    message_date.tm_hour = 12;

    message_time = mktime(&message_date);

    /*
     * If the resulting date is in the future, it probably
     * belongs to the previous calendar year.
     */
    if (message_time > now) {
        message_date.tm_year--;
        message_time = mktime(&message_date);
    }

    age = difftime(now, message_time);

    return age >= ((double)days * 86400.0);
}

void mail_browser(void)
{
    FILE *fp;
    char line[256];
    char command;
    int message_number;
    int messages[MAX_MESSAGES];
    int message_month[MAX_MESSAGES];
    int message_day[MAX_MESSAGES];
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

    /*
     * Keep the existing scan format, but capture month/day as well
     * as the actual MH message number.
     */
    fp = popen(
        "/usr/bin/mh/scan +inbox -width 76 "
        "-format '%4(msg) %02(mon{date})/%02(mday{date}) "
        "%-20(friendly{from}) %{subject}\\n'",
        "r"
    );

    if (fp == NULL) {
        printf("Unable to access mailbox.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        int msg;
        int month;
        int day;

        printf("%s", line);

        if (sscanf(line, "%d %d/%d", &msg, &month, &day) == 3) {
            if (message_count < MAX_MESSAGES) {
                messages[message_count] = msg;
                message_month[message_count] = month;
                message_day[message_count] = day;
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

        /*
         * READ
         */
        case 'r':
        case 'R':
        {
            int found = 0;

            printf("Message number: ");

            if (scanf("%d", &message_number) != 1) {
                printf("Invalid message number.\n");
                return;
            }

            for (int i = 0; i < message_count; i++) {
                if (messages[i] == message_number) {
                    current_index = i;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf(
                    "Message %d does not exist in the inbox.\n",
                    message_number
                );
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
                 * Display message headers.
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
                 * Create temporary file for decoded message body.
                 */
                body_fd = mkstemps(body_file, 5);

                if (body_fd == -1) {
                    printf("Unable to create temporary message file.\n");
                    break;
                }

                close(body_fd);

                /*
                 * Extract preferred MIME representation.
                 */
                snprintf(
                    command_buffer,
                    sizeof(command_buffer),
                    "/usr/bin/mh/mhstore +inbox %d "
                    "-prefer text/plain -outfile \"%s\"",
                    message_number,
                    body_file
                );

                system(command_buffer);

                /*
                 * Render body.
                 */
                snprintf(
                    command_buffer,
                    sizeof(command_buffer),
                    "lynx -dump -nonumbers -nolist \"%s\"",
                    body_file
                );

                system(command_buffer);

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
                        if (current_index < message_count - 1) {
                            current_index++;
                            message_number =
                                messages[current_index];
                        } else {
                            printf(
                                "\nAlready at the last message.\n"
                            );
                        }
                        break;

                    case 'p':
                    case 'P':
                        if (current_index > 0) {
                            current_index--;
                            message_number =
                                messages[current_index];
                        } else {
                            printf(
                                "\nAlready at the first message.\n"
                            );
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
        }

        /*
         * ARCHIVE
         */
        case 'a':
        case 'A':
        {
            int archive_choice;

            printf(
                "\n"
                "+------------------------------------------------------------------------------+\n"
                "|                                  ARCHIVE                                     |\n"
                "+------------------------------------------------------------------------------+\n"
                "|  1. Single message                                                           |\n"
                "|  2. Range                                                                     |\n"
                "|  3. Older than...                                                             |\n"
                "|  4. All messages                                                              |\n"
                "|  5. Cancel                                                                     |\n"
                "+------------------------------------------------------------------------------+\n"
            );

            printf("\nChoice: ");

            if (scanf("%d", &archive_choice) != 1) {
                printf("Invalid choice.\n");
                return;
            }

            /*
             * SINGLE MESSAGE
             */
            if (archive_choice == 1) {

                printf("Message number: ");

                if (scanf("%d", &message_number) != 1) {
                    printf("Invalid message number.\n");
                    return;
                }

                printf(
                    "Archive message %d? [y/N]: ",
                    message_number
                );

                {
                    char confirm;
                    scanf(" %c", &confirm);

                    if (confirm != 'y' && confirm != 'Y') {
                        printf("Message not archived.\n");
                        break;
                    }
                }

                {
                    int selected[1];
                    selected[0] = message_number;

                    archive_messages(selected, 1);
                }

                printf("\nPress Enter to return...");
                getchar();
                getchar();
            }

            /*
             * RANGE
             *
             * Range refers to DISPLAY POSITION, not MH message
             * number. This is important because the inbox can
             * contain gaps such as 1, 2, 3, 7, 9, 10.
             */
            else if (archive_choice == 2) {

                int first;
                int last;
                int selected_count;
                int selected[MAX_MESSAGES];

                printf(
                    "\nArchive messages by displayed position.\n"
                );

                printf("First message: ");

                if (scanf("%d", &first) != 1) {
                    printf("Invalid position.\n");
                    break;
                }

                printf("Last message: ");

                if (scanf("%d", &last) != 1) {
                    printf("Invalid position.\n");
                    break;
                }

                if (first < 1 ||
                    last < 1 ||
                    first > message_count ||
                    last > message_count ||
                    first > last) {

                    printf(
                        "\nInvalid range. Inbox contains "
                        "%d displayed messages.\n",
                        message_count
                    );

                    break;
                }

                selected_count = last - first + 1;

                for (int i = 0; i < selected_count; i++) {
                    selected[i] = messages[(first - 1) + i];
                }

                printf(
                    "\n%d message%s selected.\n",
                    selected_count,
                    selected_count == 1 ? "" : "s"
                );

                printf(
                    "Archive displayed messages %d through %d? [y/N]: ",
                    first,
                    last
                );

                {
                    char confirm;
                    scanf(" %c", &confirm);

                    if (confirm != 'y' && confirm != 'Y') {
                        printf("Messages not archived.\n");
                        break;
                    }
                }

                archive_messages(selected, selected_count);

                printf("\nPress Enter to return...");
                getchar();
                getchar();
            }

            /*
             * OLDER THAN N DAYS
             */
            else if (archive_choice == 3) {

                int days;
                int selected_count = 0;
                int selected[MAX_MESSAGES];

                printf(
                    "\nArchive messages older than how many days? "
                );

                if (scanf("%d", &days) != 1) {
                    printf("Invalid number of days.\n");
                    break;
                }

                if (days < 1) {
                    printf("Number of days must be at least 1.\n");
                    break;
                }

                for (int i = 0; i < message_count; i++) {

                    if (message_is_older_than(
                            message_month[i],
                            message_day[i],
                            days)) {

                        selected[selected_count] =
                            messages[i];

                        selected_count++;
                    }
                }

                if (selected_count == 0) {
                    printf(
                        "\nNo messages are older than %d days.\n",
                        days
                    );
                    printf("\nPress Enter to return...");
                    getchar();
                    getchar();
                    break;
                }

                printf(
                    "\n%d message%s older than %d days.\n",
                    selected_count,
                    selected_count == 1 ? "" : "s",
                    days
                );

                printf(
                    "Archive these messages? [y/N]: "
                );

                {
                    char confirm;
                    scanf(" %c", &confirm);

                    if (confirm != 'y' && confirm != 'Y') {
                        printf("Messages not archived.\n");
                        break;
                    }
                }

                archive_messages(selected, selected_count);

                printf("\nPress Enter to return...");
                getchar();
                getchar();
            }

            /*
             * ALL
             */
            else if (archive_choice == 4) {

                int selected[MAX_MESSAGES];

                printf(
                    "\nArchive all %d messages? [y/N]: ",
                    message_count
                );

                {
                    char confirm;
                    scanf(" %c", &confirm);

                    if (confirm != 'y' && confirm != 'Y') {
                        printf("Messages not archived.\n");
                        break;
                    }
                }

                for (int i = 0; i < message_count; i++) {
                    selected[i] = messages[i];
                }

                archive_messages(selected, message_count);

                printf("\nPress Enter to return...");
                getchar();
                getchar();
            }

            /*
             * CANCEL
             */
            else if (archive_choice == 5) {
                printf("\nArchive cancelled.\n");
            }

            else {
                printf("\nInvalid archive option.\n");
            }

            break;
        }

        /*
         * DELETE
         */
        case 'd':
        case 'D':
        {
            printf("Message number: ");

            if (scanf("%d", &message_number) != 1) {
                printf("Invalid message number.\n");
                return;
            }

            printf(
                "Delete message %d? [y/N]: ",
                message_number
            );

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

            printf(
                "Message %d deleted.\n",
                message_number
            );

            printf("\nPress Enter to return...");
            getchar();
            getchar();
            break;
        }

        /*
         * QUIT
         */
        case 'q':
        case 'Q':
            return;

        /*
         * NEXT
         */
        case 'n':
        case 'N':
            if (current_index < message_count - 1) {
                current_index++;
                message_number =
                    messages[current_index];

                printf(
                    "\nNext message: %d\n",
                    message_number
                );
            } else {
                printf(
                    "\nAlready at the last message.\n"
                );
            }

            printf("Press Enter to return...");
            getchar();
            getchar();
            break;

        /*
         * PREVIOUS
         */
        case 'p':
        case 'P':
            if (current_index > 0) {
                current_index--;
                message_number =
                    messages[current_index];

                printf(
                    "\nPrevious message: %d\n",
                    message_number
                );
            } else {
                printf(
                    "\nAlready at the first message.\n"
                );
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