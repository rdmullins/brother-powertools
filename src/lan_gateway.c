#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "lan_gateway.h"
#include "weather.h"
#include "npr.h"

#define BUFFER_SIZE 256
#define NPR_LAN_STORIES_PER_PAGE 5
#define NPR_LAN_STORIES_PER_PAGE 5
#define NPR_LAN_LINES_PER_PAGE 11

static void send_text(int client_fd, const char *text)
{
    send(client_fd, text, strlen(text), 0);
}

static void send_weather_menu(int client_fd)
{
    send_text(
        client_fd,
        "\r\n"
        "POWERTOOLS WEATHER\r\n"
        "==================\r\n"
        "\r\n"
        "1. Current Conditions\r\n"
        "2. Extended Forecast\r\n"
        "3. Return\r\n"
        "\r\n"
        "Selection: "
    );
}

static void send_menu(int client_fd)
{
    send_text(
        client_fd,
        "\r\n"
        "POWERTOOLS LAN\r\n"
        "==============\r\n"
        "\r\n"
        "1. Weather\r\n"
        "2. NPR News\r\n"
        "3. Library\r\n"
        "4. Notes\r\n"
        "5. Wikipedia\r\n"
        "6. Wiktionary\r\n"
        "7. Gutenberg\r\n"
        "8. Games\r\n"
        "\r\n"
        "Q. Disconnect\r\n"
        "\r\n"
        "Selection: "
    );
}

static int receive_line(int client_fd, char *line, size_t size)
{
    size_t pos = 0;

    while (pos < size - 1)
    {
        char ch;
        ssize_t received = recv(client_fd, &ch, 1, 0);

        if (received <= 0)
            return 0;

        if (ch == '\r' || ch == '\n')
        {
            if (pos == 0)
            {
                /* Ignore a second half of CRLF. */
                continue;
            }

            break;
        }

        line[pos++] = ch;
    }

    line[pos] = '\0';

    return 1;
}

static void send_weather(int client_fd)
{
    WeatherData weather;

    if (!weather_get_current(&weather))
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to retrieve weather.\r\n"
        );
        return;
    }

    char output[1024];

    snprintf(
        output,
        sizeof(output),
        "\r\n"
        "POWERTOOLS WEATHER\r\n"
        "==================\r\n"
        "\r\n"
        "%s, %s\r\n"
        "\r\n"
        "%d F\r\n"
        "Feels like %d F\r\n"
        "\r\n"
        "%s\r\n"
        "\r\n"
        "Humidity: %d%%\r\n"
        "Pressure: %d hPa\r\n"
        "Wind: %.1f mph\r\n",
        weather.city,
        weather.state,
        weather.temperature,
        weather.feels_like,
        weather.description,
        weather.humidity,
        weather.pressure,
        weather.wind_speed
    );

    send_text(client_fd, output);
}

static void send_weather_forecast(int client_fd)
{
    WeatherForecast forecast;

    if (!weather_get_forecast(&forecast))
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to retrieve forecast.\r\n"
        );
        return;
    }

    char output[2048];

    snprintf(
        output,
        sizeof(output),
        "\r\n"
        "EXTENDED FORECAST\r\n"
        "=================\r\n"
        "\r\n"
        "%s, %s\r\n"
        "\r\n",
        forecast.city,
        forecast.state
    );

    send_text(client_fd, output);

    for (int i = 0; i < forecast.count; i++)
    {
        char line[256];

        snprintf(
            line,
            sizeof(line),
            "%s  %-16s %d/%d F  %d%%\r\n",
            forecast.days[i].day,
            forecast.days[i].description,
            forecast.days[i].high,
            forecast.days[i].low,
            forecast.days[i].precipitation_chance
        );

        send_text(client_fd, line);
    }
}

static void send_npr_article(int client_fd)
{
    FILE *fp;

    char lines[512][82];
    int line_count = 0;
    int page = 0;

    fp = fopen("/tmp/powertools-npr.txt", "r");

    if (fp == NULL)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to open article.\r\n"
        );

        return;
    }

    /*
     * Read the article and wrap it to fit the Brother's
     * 80-column display.
     */
    char input[512];

    while (fgets(input, sizeof(input), fp) != NULL)
    {
        char *word = input;

        while (*word != '\0')
        {
            while (*word == ' ' || *word == '\t')
                word++;

            if (*word == '\0' || *word == '\n')
                break;

            char wrapped[80];
            int width = 0;

            while (*word != '\0' &&
                   *word != '\n' &&
                   width < 76)
            {
                if (*word == ' ')
                {
                    char *next = word + 1;

                    while (*next == ' ')
                        next++;

                    if (*next == '\0' ||
                        *next == '\n')
                    {
                        word = next;
                        break;
                    }
                }

                wrapped[width++] = *word;
                word++;
            }

            wrapped[width] = '\0';

            /*
             * If we stopped in the middle of a word,
             * back up to the last space.
             */
            if (*word != '\0' &&
                *word != '\n' &&
                width >= 76)
            {
                int split = width - 1;

                while (split >= 0 &&
                       wrapped[split] != ' ')
                {
                    split--;
                }

                if (split > 0)
                {
                    word -= width - split - 1;
                    wrapped[split] = '\0';
                    width = split;
                }
            }

            while (width > 0 &&
                   wrapped[width - 1] == ' ')
            {
                wrapped[--width] = '\0';
            }

            if (line_count < 512)
            {
                snprintf(
                    lines[line_count],
                    sizeof(lines[line_count]),
                    "%s\r\n",
                    wrapped
                );

                line_count++;
            }

            if (*word == '\n')
                break;
        }

        /*
         * Preserve paragraph breaks.
         */
        if (line_count < 512)
        {
            if (input[0] == '\n' ||
                input[0] == '\r')
            {
                snprintf(
                    lines[line_count],
                    sizeof(lines[line_count]),
                    "\r\n"
                );

                line_count++;
            }
        }
    }

    fclose(fp);

    if (line_count == 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Article is empty.\r\n"
        );

        return;
    }

    while (1)
    {
        int start =
            page * NPR_LAN_LINES_PER_PAGE;

        int end =
            start + NPR_LAN_LINES_PER_PAGE;

        if (end > line_count)
            end = line_count;

        send_text(
            client_fd,
            "\r\n"
            "NPR ARTICLE\r\n"
            "===========\r\n"
            "\r\n"
        );

        for (int i = start; i < end; i++)
            send_text(client_fd, lines[i]);

        send_text(
            client_fd,
            "\r\n"
            "N=Next  P=Previous  Q=Back\r\n"
        );

        char input_command[32];

        if (!receive_line(
                client_fd,
                input_command,
                sizeof(input_command)))
        {
            return;
        }

        if (input_command[0] == '\0')
            return;

        char command =
            (char)tolower(
                (unsigned char)input_command[0]);

        if (command == 'q')
            return;

        if (command == 'n')
        {
            if (end < line_count)
                page++;

            continue;
        }

        if (command == 'p')
        {
            if (page > 0)
                page--;

            continue;
        }

send_text(
    client_fd,
    "\r\n"
    "Enter N, P, or Q.\r\n"
);
    }
}

static void send_npr_headlines(int client_fd)
{
    NPRStory stories[NPR_MAX_STORIES];

    int story_count =
        npr_get_headlines(stories, NPR_MAX_STORIES);

    if (story_count <= 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to retrieve NPR News.\r\n"
        );

        return;
    }

    int page = 0;

    while (1)
    {
        int start =
            page * NPR_LAN_STORIES_PER_PAGE;

        int end =
            start + NPR_LAN_STORIES_PER_PAGE;

        if (end > story_count)
            end = story_count;

        send_text(
            client_fd,
            "\r\n"
            "NPR NEWS\r\n"
            "========\r\n"
            "\r\n"
        );

        for (int i = start; i < end; i++)
        {
            char line[128];

            snprintf(
                line,
                sizeof(line),
                "%2d. %.55s\r\n",
                i + 1,
                stories[i].title
            );

            send_text(client_fd, line);
        }

        send_text(
            client_fd,
            "\r\n"
        );

        if (story_count > NPR_LAN_STORIES_PER_PAGE)
        {
            send_text(
                client_fd,
                "Enter a Story Number | N=Next  P=Previous  Q=Quit\r\n"
            );
        }
        else
        {
            send_text(
                client_fd,
                "Q=Quit\r\n"
            );
        }

        char input[32];

        if (!receive_line(
                client_fd,
                input,
                sizeof(input)))
        {
            return;
        }

        if (input[0] == '\0')
            return;

        char command =
            (char)tolower(
                (unsigned char)input[0]);

        if (command == 'q')
            return;

        if (command == 'n')
        {
            if (end < story_count)
                page++;

            continue;
        }

        if (command == 'p')
        {
            if (page > 0)
                page--;

            continue;
        }

int choice = atoi(input);

if (choice >= 1 && choice <= story_count)
{
    send_text(
        client_fd,
        "\r\n"
        "Fetching article...\r\n"
    );

    if (!npr_fetch_article(&stories[choice - 1]))
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to retrieve article.\r\n"
        );

        continue;
    }

    send_npr_article(client_fd);

    continue;
}

        send_text(
    client_fd,
    "\r\n"
    "Enter a story number, N, P, or Q.\r\n"
);
    }
}

static void weather_menu(int client_fd)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        send_weather_menu(client_fd);

        if (!receive_line(client_fd, buffer, sizeof(buffer)))
            return;

        if (buffer[0] == '\0')
            continue;

        switch (buffer[0])
        {
            case '1':
                send_weather(client_fd);
                break;

case '2':
    send_weather_forecast(client_fd);
    break;

            case '3':
                return;

            default:
                send_text(
                    client_fd,
                    "\r\n"
                    "Invalid selection.\r\n"
                );
                break;
        }
    }
}

void lan_gateway(void)
{
    int server_fd;
    int client_fd;
    int opt = 1;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("LAN gateway: socket");
        return;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        perror("LAN gateway: setsockopt");
        close(server_fd);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(LAN_GATEWAY_PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("LAN gateway: bind");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0)
    {
        perror("LAN gateway: listen");
        close(server_fd);
        return;
    }

    printf("\nLAN Gateway\n");
    printf("===========\n");
    printf("Listening on TCP port %d\n", LAN_GATEWAY_PORT);
    printf("Waiting for Brother connection...\n");

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd < 0)
    {
        perror("LAN gateway: accept");
        close(server_fd);
        return;
    }

    printf("Brother connected!\n");

    send_text(
        client_fd,
        "\r\n"
        "POWERTOOLS LAN GATEWAY\r\n"
        "======================\r\n"
    );

    send_menu(client_fd);

    while (1)
    {
        if (!receive_line(client_fd, buffer, sizeof(buffer)))
            break;

        if (buffer[0] == '\0')
            continue;

        char choice = buffer[0];

        switch (choice)
        {
case '1':
    weather_menu(client_fd);
    break;

case '2':
    send_npr_headlines(client_fd);
    break;
            case '3':
                send_text(client_fd,
                          "\r\nLibrary selected.\r\n");
                break;

            case '4':
                send_text(client_fd,
                          "\r\nNotes selected.\r\n");
                break;

            case '5':
                send_text(client_fd,
                          "\r\nWikipedia selected.\r\n");
                break;

            case '6':
                send_text(client_fd,
                          "\r\nWiktionary selected.\r\n");
                break;

            case '7':
                send_text(client_fd,
                          "\r\nGutenberg selected.\r\n");
                break;

            case '8':
                send_text(client_fd,
                          "\r\nGames selected.\r\n");
                break;

            case 'q':
            case 'Q':
                send_text(client_fd,
                          "\r\nDisconnecting...\r\n");
                close(client_fd);
                close(server_fd);
                return;

            default:
                send_text(client_fd,
                          "\r\nInvalid selection.\r\n");
                break;
        }

        send_menu(client_fd);
    }


    printf("Brother disconnected.\n");

    close(client_fd);
    close(server_fd);
}