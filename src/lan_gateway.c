#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lan_gateway.h"
#include "weather.h"

#define BUFFER_SIZE 256

static void send_text(int client_fd, const char *text)
{
    send(client_fd, text, strlen(text), 0);
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
                send_weather(client_fd);
                break;

            case '2':
                send_text(client_fd,
                          "\r\nNPR News selected.\r\n");
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