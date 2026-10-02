#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "gopher.h"

#define GOPHER_HOST "gopher.floodgap.com"
#define GOPHER_PORT 70

#define MAX_HISTORY 16
#define MAX_TEXT_LINES 200

#define DISPLAY_WIDTH 43
#define PAGE_SIZE 12




/* ---------------------------------------------------------
 * Connect to a Gopher server
 * --------------------------------------------------------- */

static int connect_gopher(const char *host, int port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;
    char port_string[16];
    int sockfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(port_string, sizeof(port_string), "%d", port);

    if (getaddrinfo(host, port_string, &hints, &result) != 0) {
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family,
                        rp->ai_socktype,
                        rp->ai_protocol);

        if (sockfd == -1) {
            continue;
        }

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(result);

    return sockfd;
}


/* ---------------------------------------------------------
 * Read a Gopher response
 * --------------------------------------------------------- */

static char *fetch_gopher(const char *host,
                          int port,
                          const char *selector)
{
    int sockfd;
    char request[1024];
    char buffer[4096];
    char *response = NULL;
    size_t response_size = 0;
    ssize_t received;

    sockfd = connect_gopher(host, port);

    if (sockfd == -1) {
        return NULL;
    }

    snprintf(request, sizeof(request), "%s\r\n", selector);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        close(sockfd);
        return NULL;
    }

    while ((received = recv(sockfd,
                            buffer,
                            sizeof(buffer),
                            0)) > 0) {

        char *new_response;

        new_response = realloc(response,
                               response_size + received + 1);

        if (new_response == NULL) {
            free(response);
            close(sockfd);
            return NULL;
        }

        response = new_response;

        memcpy(response + response_size,
               buffer,
               received);

        response_size += received;
        response[response_size] = '\0';
    }

    close(sockfd);

    return response;
}

char *gopher_fetch(
    const char *host,
    int port,
    const char *selector)
{
    return fetch_gopher(host, port, selector);
}

/* ---------------------------------------------------------
 * Parse a Gopher directory
 * --------------------------------------------------------- */

static int parse_directory(char *response,
                           GopherEntry entries[],
                           int max_entries)
{
    char *line;
    char *saveptr;
    int count = 0;

    line = strtok_r(response, "\n", &saveptr);

    while (line != NULL && count < max_entries) {

        char *display;
        char *selector;
        char *host;
        char *port;

        /* Remove CR from CR/LF line endings. */
        line[strcspn(line, "\r")] = '\0';

        /* End of Gopher directory. */
        if (strcmp(line, ".") == 0) {
            break;
        }

        /*
         * Gopher item type is the FIRST CHARACTER of the line.
         * It is immediately followed by the display text.
         */
        if (line[0] == '\0') {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        entries[count].type = line[0];

        /*
         * The remaining fields are tab-delimited:
         *
         * display
         * selector
         * host
         * port
         */
        display = strtok(line + 1, "\t");
        selector = strtok(NULL, "\t");
        host = strtok(NULL, "\t");
        port = strtok(NULL, "\t");

        if (display == NULL) {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        /*
         * Informational records don't represent selectable
         * Gopher items, so ignore them for now.
         */
        if (entries[count].type == 'i') {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        /*
         * Error records aren't useful as menu choices.
         */
        if (entries[count].type == '3') {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        snprintf(entries[count].display,
                 sizeof(entries[count].display),
                 "%s",
                 display);

        if (selector != NULL) {
            snprintf(entries[count].selector,
                     sizeof(entries[count].selector),
                     "%s",
                     selector);
        } else {
            entries[count].selector[0] = '\0';
        }

        if (host != NULL) {
            snprintf(entries[count].host,
                     sizeof(entries[count].host),
                     "%s",
                     host);
        } else {
            entries[count].host[0] = '\0';
        }

        if (port != NULL) {
            entries[count].port = atoi(port);
        } else {
            entries[count].port = 70;
        }

        count++;

        line = strtok_r(NULL, "\n", &saveptr);
    }

    return count;
}

int gopher_parse_directory(
    char *response,
    GopherEntry entries[],
    int max_entries)
{
    return parse_directory(
        response,
        entries,
        max_entries
    );
}

/* ---------------------------------------------------------
 * Display a directory
 * --------------------------------------------------------- */

static void display_directory(GopherEntry entries[],
                              int count,
                              int page)
{
    int start = page * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    int i;

    if (end > count) {
        end = count;
    }

    printf("\n");
    printf("+-------------------------------------------+\n");
    printf("|                 GOPHER                    |\n");
    printf("+-------------------------------------------+\n");

    if (count == 0) {
        printf("| No menu entries found.                    |\n");
    }

    for (i = start; i < end; i++) {
        char marker = ' ';

        if (entries[i].type == '1') {
            marker = '>';
        } else if (entries[i].type == '0') {
            marker = '#';
        } else {
            marker = '?';
        }

        printf("| %2d. %c %-34.34s |\n",
               i + 1,
               marker,
               entries[i].display);
    }

    printf("+-------------------------------------------+\n");

    if (end < count) {
        printf("| N. Next page                              |\n");
    }

    printf("| B. Back                                   |\n");
    printf("| Q. Quit Gopher                            |\n");
    printf("+-------------------------------------------+\n");
}


/* ---------------------------------------------------------
 * Display a text document
 * --------------------------------------------------------- */

static void display_text(char *text)
{
    char *line;
    char *saveptr;
    int line_count = 0;
    int page_start = 0;

    line = strtok_r(text, "\n", &saveptr);

    while (line != NULL && line_count < MAX_TEXT_LINES) {
        line[strcspn(line, "\r")] = '\0';

        printf("%s\n", line);

        line_count++;

        if (line_count % 20 == 0) {
            printf("\n-- More -- Press Enter to continue --");
            getchar();
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }

    (void)page_start;

    printf("\n-- End of document --\n");
    printf("Press Enter to return...");
    getchar();
}


/* ---------------------------------------------------------
 * Gopher browser
 * --------------------------------------------------------- */

void gopher_menu(void)
{
    GopherLocation current;
    GopherLocation history[MAX_HISTORY];

    int history_count = 0;

    strcpy(current.host, GOPHER_HOST);
    strcpy(current.selector, "");
    current.port = GOPHER_PORT;

    while (1) {
        char *response;
        GopherEntry entries[GOPHER_MAX_ENTRIES];
        int entry_count;
        int page = 0;
        char input[32];

        response = fetch_gopher(current.host,
                                current.port,
                                current.selector);

        if (response == NULL) {
            printf("\nUnable to connect to Gopher server.\n");
            printf("Press Enter to return...");
            getchar();
            return;
        }

        /*
         * A Gopher directory begins with item records.
         * For now, determine whether this is a directory
         * by looking for tabs in the response.
         */
        if (strchr(response, '\t') == NULL) {
            printf("\n");
            printf("+-------------------------------------------+\n");
            printf("|                 GOPHER                    |\n");
            printf("+-------------------------------------------+\n\n");

            display_text(response);

            free(response);
            continue;
        }

        entry_count = parse_directory(response,
                                      entries,
                                      GOPHER_MAX_ENTRIES);

        free(response);

        while (1) {
            int choice;

            display_directory(entries,
                              entry_count,
                              page);

            printf("\nEnter choice: ");

            if (fgets(input, sizeof(input), stdin) == NULL) {
                return;
            }

            if (input[0] == 'q' || input[0] == 'Q') {
                return;
            }

            if (input[0] == 'b' || input[0] == 'B') {
                if (history_count == 0) {
                    return;
                }

                current = history[--history_count];
                break;
            }

            if (input[0] == 'n' || input[0] == 'N') {
                if ((page + 1) * PAGE_SIZE < entry_count) {
                    page++;
                }

                continue;
            }

            choice = atoi(input);

            if (choice < 1 || choice > entry_count) {
                printf("Invalid choice.\n");
                continue;
            }

            GopherEntry *entry = &entries[choice - 1];

            /*
             * Text document.
             */
            if (entry->type == '0') {
                char *text;

                text = fetch_gopher(entry->host,
                                    entry->port,
                                    entry->selector);

                if (text == NULL) {
                    printf("\nUnable to retrieve document.\n");
                    printf("Press Enter to continue...");
                    getchar();
                    continue;
                }

                printf("\n");
                printf("+-------------------------------------------+\n");
                printf("|                 GOPHER                    |\n");
                printf("+-------------------------------------------+\n\n");

                display_text(text);

                free(text);
                continue;
            }

            /*
             * Directory.
             */
            /*
 * Directory.
 */
if (entry->type == '1') {

    if (history_count >= MAX_HISTORY) {
        printf("\nGopher history is full.\n");
        printf("Press Enter to continue...");
        getchar();
        continue;
    }

    history[history_count++] = current;

    snprintf(current.host,
             sizeof(current.host),
             "%s",
             entry->host);

    snprintf(current.selector,
             sizeof(current.selector),
             "%s",
             entry->selector);

    current.port = entry->port;

    break;
}


/*
 * Index/search server.
 */
if (entry->type == '7') {
    char query[256];

    printf("\nSearch %s\n", entry->display);
    printf("Search term: ");

    if (fgets(query, sizeof(query), stdin) == NULL) {
        continue;
    }

    query[strcspn(query, "\r\n")] = '\0';

    if (query[0] == '\0') {
        continue;
    }

    if (history_count >= MAX_HISTORY) {
        printf("\nGopher history is full.\n");
        printf("Press Enter to continue...");
        getchar();
        continue;
    }

    history[history_count++] = current;

    snprintf(current.host,
             sizeof(current.host),
             "%s",
             entry->host);

size_t selector_len = strlen(entry->selector);
size_t query_len = strlen(query);

if (selector_len + query_len + 2 > sizeof(current.selector)) {
    printf("\nSearch query is too long.\n");
    printf("Press Enter to continue...");
    getchar();
    continue;
}

memcpy(current.selector,
        entry->selector,
        selector_len);

current.selector[selector_len] = '?';

memcpy(current.selector + selector_len + 1,
        query,
        query_len);

current.selector[selector_len + query_len + 1] = '\0';

current.port = entry->port;

break;

current.port = entry->port;

break;

            printf("\nUnsupported Gopher item type: %c\n",
                   entry->type);

            printf("Press Enter to continue...");
            getchar();
        }
    }
}}