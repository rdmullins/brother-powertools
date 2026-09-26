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
#include "catalog.h"
#include "bibliography.h"
#include "notes.h"
#include "openlibrary.h"

#define BUFFER_SIZE 256
#define NPR_LAN_STORIES_PER_PAGE 5
#define NPR_LAN_LINES_PER_PAGE 11

#define LIBRARY_SEARCH_MAX_RESULTS 20
#define LIBRARY_RESULTS_PER_PAGE 3
#define BIBLIOGRAPHY_RESULTS_PER_PAGE 3
#define NOTES_RESULTS_PER_PAGE 3
#define OPENLIBRARY_SUBJECTS_PER_PAGE 8

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

static void send_catalog_record(int client_fd,
                                const CatalogRecord *record)
{
    char output[4096];

    snprintf(
        output,
        sizeof(output),
        "\r\n"
        "BOOK RECORD\r\n"
        "===========\r\n"
        "\r\n"
        "ID:        %s\r\n"
        "ISBN:      %s\r\n"
        "Author:    %s\r\n"
        "Title:     %s\r\n"
        "Place:     %s\r\n"
        "Publisher: %s\r\n"
        "Year:      %s\r\n"
        "Subjects:  %s\r\n"
        "Location:  %s\r\n",
        record->id,
        record->isbn,
        record->author,
        record->title,
        record->place,
        record->publisher,
        record->year,
        record->subjects,
        record->location
    );

    send_text(client_fd, output);
}


static void send_library_find(int client_fd)
{
    char query[BUFFER_SIZE];
    CatalogRecord results[LIBRARY_SEARCH_MAX_RESULTS];

    send_text(
        client_fd,
        "\r\n"
        "FIND BOOK\r\n"
        "=========\r\n"
        "\r\n"
        "Search: "
    );

    if (!receive_line(client_fd,
                      query,
                      sizeof(query)))
    {
        return;
    }

    if (query[0] == '\0')
        return;

    int count =
        catalog_search(
            "data/catalog.db",
            query,
            results,
            LIBRARY_SEARCH_MAX_RESULTS
        );

    if (count < 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to search catalog.\r\n"
        );

        return;
    }

    if (count == 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "No books found.\r\n"
        );

        return;
    }

    int page = 0;

    while (1)
    {
        int start =
            page * LIBRARY_RESULTS_PER_PAGE;

        int end =
            start + LIBRARY_RESULTS_PER_PAGE;

        if (end > count)
            end = count;

        send_text(
            client_fd,
            "\r\n"
            "CATALOG RESULTS\r\n"
            "===============\r\n"
            "\r\n"
        );

        for (int i = start; i < end; i++)
        {
            char line[768];

            snprintf(
                line,
                sizeof(line),
                "%2d. %.55s\r\n"
                "    %.50s\r\n"
                "    %s, %s\r\n"
                "    Location: %s\r\n"
                "\r\n",
                i + 1,
                results[i].title,
                results[i].author,
                results[i].publisher,
                results[i].year,
                results[i].location
            );

            send_text(client_fd, line);
        }

        if (count > LIBRARY_RESULTS_PER_PAGE)
        {
            send_text(
                client_fd,
                "Enter a Book Number | "
                "N=Next  P=Previous  Q=Back\r\n"
            );
        }
        else
        {
            send_text(
                client_fd,
                "Enter a Book Number | Q=Back\r\n"
            );
        }

        char input[BUFFER_SIZE];

        if (!receive_line(client_fd,
                          input,
                          sizeof(input)))
        {
            return;
        }

        if (input[0] == '\0')
            return;

        char command =
            (char)tolower(
                (unsigned char)input[0]
            );

        if (command == 'q')
            return;

        if (command == 'n')
        {
            if (end < count)
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

        if (choice >= 1 &&
            choice <= count)
        {
            send_catalog_record(
                client_fd,
                &results[choice - 1]
            );

            send_text(
                client_fd,
                "\r\n"
                "Press Enter to return to results.\r\n"
            );

            if (!receive_line(client_fd,
                              input,
                              sizeof(input)))
            {
                return;
            }

            continue;
        }

        send_text(
            client_fd,
            "\r\n"
            "Enter a book number, N, P, or Q.\r\n"
        );
    }
}

static void send_note_record(
    int client_fd,
    const NoteRecord *note)
{
    char line[768];

    send_text(
        client_fd,
        "\r\n"
        "RESEARCH NOTE\r\n"
        "=============\r\n"
        "\r\n"
    );

    snprintf(line, sizeof(line),
             "ID:       %s\r\n", note->id);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "BIB ID:   %s\r\n", note->bib_id);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Title:    %s\r\n", note->title);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Locator:  %s\r\n", note->locator);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Use:      %s\r\n", note->use);
    send_text(client_fd, line);

    send_text(
        client_fd,
        "\r\n"
        "Note:\r\n"
    );

    send_text(client_fd, note->text);
    send_text(client_fd, "\r\n");
}

static void send_bibliography_notes(
    int client_fd,
    const BibliographyRecord *record)
{
    NoteRecord results[LIBRARY_SEARCH_MAX_RESULTS];

    int count =
        notes_search_by_bibliography(
            "data/notes.db",
            record->id,
            results,
            LIBRARY_SEARCH_MAX_RESULTS
        );

    if (count < 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to retrieve research notes.\r\n"
        );

        return;
    }

    if (count == 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "No research notes found for this source.\r\n"
        );

        return;
    }

    int page = 0;

    while (1)
    {
        int start =
            page * NOTES_RESULTS_PER_PAGE;

        int end =
            start + NOTES_RESULTS_PER_PAGE;

        if (end > count)
            end = count;

        send_text(
            client_fd,
            "\r\n"
            "RESEARCH NOTES\r\n"
            "==============\r\n"
            "\r\n"
        );

        for (int i = start; i < end; i++)
        {
            char line[768];

            snprintf(
                line,
                sizeof(line),
                "%2d. %.55s\r\n"
                "    Locator: %.50s\r\n"
                "    Use: %.50s\r\n"
                "\r\n",
                i + 1,
                results[i].title,
                results[i].locator,
                results[i].use
            );

            send_text(client_fd, line);
        }

        if (count > NOTES_RESULTS_PER_PAGE)
        {
            send_text(
                client_fd,
                "Enter a Note Number | "
                "N=Next  P=Previous  Q=Back\r\n"
            );
        }
        else
        {
            send_text(
                client_fd,
                "Enter a Note Number | Q=Back\r\n"
            );
        }

        char input[BUFFER_SIZE];

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
            if (end < count)
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

        if (choice >= 1 && choice <= count)
        {
            send_note_record(
                client_fd,
                &results[choice - 1]
            );

            send_text(
                client_fd,
                "\r\n"
                "Press Enter to return to notes.\r\n"
            );

            if (!receive_line(
                    client_fd,
                    input,
                    sizeof(input)))
            {
                return;
            }

            continue;
        }

        send_text(
            client_fd,
            "\r\n"
            "Enter a note number, N, P, or Q.\r\n"
        );
    }
}

static void send_add_note(
    int client_fd,
    const BibliographyRecord *record)
{
    NoteRecord note;
    char input[NOTE_MAX_TEXT];
    char id[NOTE_MAX_ID];

    memset(&note, 0, sizeof(note));

    if (notes_next_id(
            "data/notes.db",
            id,
            sizeof(id)) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to generate a note ID.\r\n"
        );

        return;
    }

    strcpy(note.id, id);
    strcpy(note.bib_id, record->id);

    send_text(
        client_fd,
        "\r\n"
        "ADD RESEARCH NOTE\r\n"
        "=================\r\n"
        "\r\n"
        "Title: "
    );

    if (!receive_line(
            client_fd,
            note.title,
            sizeof(note.title)))
    {
        return;
    }

    if (note.title[0] == '\0')
        return;

    send_text(client_fd, "Locator: ");

    if (!receive_line(
            client_fd,
            note.locator,
            sizeof(note.locator)))
    {
        return;
    }

    send_text(client_fd, "Use: ");

    if (!receive_line(
            client_fd,
            note.use,
            sizeof(note.use)))
    {
        return;
    }

    send_text(client_fd, "Note: ");

    if (!receive_line(
            client_fd,
            input,
            sizeof(input)))
    {
        return;
    }

    if (input[0] == '\0')
        return;

    strcpy(note.text, input);

    send_text(
        client_fd,
        "\r\n"
        "1. Save Note\r\n"
        "2. Cancel\r\n"
        "\r\n"
        "Selection: "
    );

    if (!receive_line(
            client_fd,
            input,
            sizeof(input)))
    {
        return;
    }

    if (input[0] != '1')
    {
        send_text(
            client_fd,
            "\r\n"
            "Note cancelled.\r\n"
        );

        return;
    }

    if (notes_save(
            "data/notes.db",
            &note) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to save note.\r\n"
        );

        return;
    }

    send_text(
        client_fd,
        "\r\n"
        "Note saved.\r\n"
    );
}

static void send_bibliography_record(
    int client_fd,
    const BibliographyRecord *record)
{
    char line[768];

    send_text(
        client_fd,
        "\r\n"
        "BIBLIOGRAPHY RECORD\r\n"
        "===================\r\n"
        "\r\n"
    );

    snprintf(line, sizeof(line),
             "ID:          %s\r\n", record->id);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Catalog ID:  %s\r\n", record->catalog_id);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Type:        %s\r\n", record->type);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Author:      %s\r\n", record->author);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Title:       %s\r\n", record->title);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Journal:     %s\r\n", record->journal);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Place:       %s\r\n", record->place);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Publisher:   %s\r\n", record->publisher);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Institution: %s\r\n", record->institution);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Volume:      %s\r\n", record->volume);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Issue:       %s\r\n", record->issue);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Date:        %s\r\n", record->date);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Pages:       %s\r\n", record->pages);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Source:      %s\r\n", record->source);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Location:    %s\r\n", record->location);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Report No.:  %s\r\n", record->report_number);
    send_text(client_fd, line);

    snprintf(line, sizeof(line),
             "Division:    %s\r\n", record->division);
    send_text(client_fd, line);

    send_text(
        client_fd,
        "\r\n"
        "Citation:\r\n"
    );

    send_text(client_fd, record->citation);
    send_text(client_fd, "\r\n");
}

static void send_bibliography_find(int client_fd)
{
    char query[BUFFER_SIZE];
    BibliographyRecord results[LIBRARY_SEARCH_MAX_RESULTS];

    send_text(
        client_fd,
        "\r\n"
        "FIND BIBLIOGRAPHY\r\n"
        "==================\r\n"
        "\r\n"
        "Search: "
    );

    if (!receive_line(
            client_fd,
            query,
            sizeof(query)))
    {
        return;
    }

    if (query[0] == '\0')
        return;

    int count =
        bibliography_search(
            "data/bibliography.db",
            query,
            results,
            LIBRARY_SEARCH_MAX_RESULTS
        );

    if (count < 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to search bibliography.\r\n"
        );

        return;
    }

    if (count == 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "No bibliography records found.\r\n"
        );

        return;
    }

    int page = 0;

    while (1)
    {
        int start =
            page * BIBLIOGRAPHY_RESULTS_PER_PAGE;

        int end =
            start + BIBLIOGRAPHY_RESULTS_PER_PAGE;

        if (end > count)
            end = count;

        send_text(
            client_fd,
            "\r\n"
            "BIBLIOGRAPHY RESULTS\r\n"
            "====================\r\n"
            "\r\n"
        );

        for (int i = start; i < end; i++)
        {
            char line[768];

            snprintf(
                line,
                sizeof(line),
                "%2d. %.55s\r\n"
                "    %.50s\r\n"
                "    %s, %s\r\n"
                "\r\n",
                i + 1,
                results[i].title,
                results[i].author,
                results[i].publisher,
                results[i].date
            );

            send_text(client_fd, line);
        }

        send_text(
            client_fd,
            "\r\n"
        );

        if (count > BIBLIOGRAPHY_RESULTS_PER_PAGE)
        {
            send_text(
                client_fd,
                "Enter a Bibliography Number | "
                "N=Next  P=Previous  Q=Back\r\n"
            );
        }
        else
        {
            send_text(
                client_fd,
                "Enter a Bibliography Number | Q=Back\r\n"
            );
        }

        char input[BUFFER_SIZE];

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
            if (end < count)
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

if (choice >= 1 && choice <= count)
{
    while (1)
    {
        send_bibliography_record(
            client_fd,
            &results[choice - 1]
        );

        send_text(
            client_fd,
            "\r\n"
"1. View Notes\r\n"
"2. Add Note\r\n"
"3. Back to Results\r\n"
            "\r\n"
            "Selection: "
        );

        if (!receive_line(
                client_fd,
                input,
                sizeof(input)))
        {
            return;
        }

        if (input[0] == '1')
        if (input[0] == '1')
{
    send_bibliography_notes(
        client_fd,
        &results[choice - 1]
    );

    continue;
}

if (input[0] == '2')
{
    send_add_note(
        client_fd,
        &results[choice - 1]
    );

    continue;
}

if (input[0] == '3' ||
    input[0] == '\0')
{
    break;
}

        send_text(
            client_fd,
            "\r\n"
            "Invalid selection.\r\n"
        );
    }

    continue;
}

        send_text(
            client_fd,
            "\r\n"
            "Enter a bibliography number, N, P, or Q.\r\n"
        );
    }
}

static void send_add_book_manual(int client_fd)
{
    CatalogRecord record;
    char input[BUFFER_SIZE];
    char id[32];

    memset(&record, 0, sizeof(record));

    send_text(
        client_fd,
        "\r\n"
        "ADD BOOK MANUALLY\r\n"
        "=================\r\n"
        "\r\n"
    );

    send_text(client_fd, "ISBN: ");
    if (!receive_line(client_fd, record.isbn, sizeof(record.isbn)))
        return;

    send_text(client_fd, "Author: ");
    if (!receive_line(client_fd, record.author, sizeof(record.author)))
        return;

    send_text(client_fd, "Title: ");
    if (!receive_line(client_fd, record.title, sizeof(record.title)))
        return;

    if (record.title[0] == '\0')
    {
        send_text(
            client_fd,
            "\r\n"
            "A title is required. Book not added.\r\n"
        );

        return;
    }

    send_text(client_fd, "Place: ");
    if (!receive_line(client_fd, record.place, sizeof(record.place)))
        return;

    send_text(client_fd, "Publisher: ");
    if (!receive_line(client_fd, record.publisher, sizeof(record.publisher)))
        return;

    send_text(client_fd, "Year: ");
    if (!receive_line(client_fd, record.year, sizeof(record.year)))
        return;

    send_text(client_fd, "Subjects: ");
    if (!receive_line(client_fd, record.subjects, sizeof(record.subjects)))
        return;

    send_text(client_fd, "Location: ");
    if (!receive_line(client_fd, record.location, sizeof(record.location)))
        return;

    if (catalog_next_id(
            "data/catalog.db",
            id,
            sizeof(id)) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to generate a catalog ID.\r\n"
        );

        return;
    }

    snprintf(
        record.id,
        sizeof(record.id),
        "%s",
        id
    );

    send_catalog_record(client_fd, &record);

    send_text(
        client_fd,
        "\r\n"
        "1. Save Book\r\n"
        "2. Cancel\r\n"
        "\r\n"
        "Selection: "
    );

    if (!receive_line(
            client_fd,
            input,
            sizeof(input)))
    {
        return;
    }

    if (input[0] != '1')
    {
        send_text(
            client_fd,
            "\r\n"
            "Book not saved.\r\n"
        );

        return;
    }

    if (catalog_save(
            "data/catalog.db",
            &record) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to save book.\r\n"
        );

        return;
    }

    char message[128];

    snprintf(
        message,
        sizeof(message),
        "\r\nBook added to catalog as %s.\r\n",
        record.id
    );

    send_text(client_fd, message);
}

static void send_add_book_isbn(int client_fd)
{
    OpenLibraryBook book;
    CatalogRecord record;
    char isbn[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    char id[32];

    memset(&book, 0, sizeof(book));
    memset(&record, 0, sizeof(record));

    send_text(
        client_fd,
        "\r\n"
        "ADD BOOK BY ISBN\r\n"
        "=================\r\n"
        "\r\n"
        "ISBN: "
    );

    if (!receive_line(
            client_fd,
            isbn,
            sizeof(isbn)))
    {
        return;
    }

    if (isbn[0] == '\0')
    {
        send_text(
            client_fd,
            "\r\n"
            "An ISBN is required.\r\n"
        );

        return;
    }

    send_text(
        client_fd,
        "\r\n"
        "Looking up ISBN...\r\n"
    );

    if (openlibrary_lookup_isbn(isbn, &book) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "No book found for that ISBN.\r\n"
            "\r\n"
            "1. Try Manual Entry\r\n"
            "2. Return\r\n"
            "\r\n"
            "Selection: "
        );

        if (!receive_line(
                client_fd,
                input,
                sizeof(input)))
        {
            return;
        }

        if (input[0] == '1')
        {
            send_add_book_manual(client_fd);
        }

        return;
    }

    snprintf(
        record.isbn,
        sizeof(record.isbn),
        "%s",
        book.isbn
    );

    snprintf(
        record.author,
        sizeof(record.author),
        "%s",
        book.author
    );

    snprintf(
        record.title,
        sizeof(record.title),
        "%s",
        book.title
    );

    snprintf(
        record.publisher,
        sizeof(record.publisher),
        "%s",
        book.publisher
    );

snprintf(
    record.year,
    sizeof(record.year),
    "%.15s",
    book.publish_date
);

    send_text(
        client_fd,
        "\r\n"
        "BOOK FOUND\r\n"
        "==========\r\n"
        "\r\n"
    );

 char line[768];

snprintf(
    line,
    sizeof(line),
    "ISBN:      %s\r\n",
    book.isbn
);
send_text(client_fd, line);

snprintf(
    line,
    sizeof(line),
    "Author:    %s\r\n",
    book.author
);
send_text(client_fd, line);

snprintf(
    line,
    sizeof(line),
    "Title:     %s\r\n",
    book.title
);
send_text(client_fd, line);

snprintf(
    line,
    sizeof(line),
    "Publisher: %s\r\n",
    book.publisher
);
send_text(client_fd, line);

snprintf(
    line,
    sizeof(line),
    "Published: %s\r\n",
    book.publish_date
);
send_text(client_fd, line);

snprintf(
    line,
    sizeof(line),
    "Subjects:  %d\r\n",
    book.subject_count
);
send_text(client_fd, line);

    if (book.subject_count > 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "SUBJECTS\r\n"
            "--------\r\n"
        );

        for (int i = 0; i < book.subject_count; i++)
        {
            snprintf(
                line,
                sizeof(line),
                "%2d. %s\r\n",
                i + 1,
                book.subjects[i]
            );

            send_text(client_fd, line);
        }

        send_text(
            client_fd,
            "\r\n"
            "Enter subject numbers separated by commas,\r\n"
            "or press Enter for none.\r\n"
            "Subjects: "
        );

        if (!receive_line(
                client_fd,
                input,
                sizeof(input)))
        {
            return;
        }

        if (input[0] != '\0')
        {
            char *token = strtok(input, ",");

            while (token != NULL)
            {
                int subject_number = atoi(token);

                if (subject_number >= 1 &&
                    subject_number <= book.subject_count)
                {
                    if (record.subjects[0] != '\0')
                    {
                        strncat(
                            record.subjects,
                            ";",
                            sizeof(record.subjects) -
                            strlen(record.subjects) -
                            1
                        );
                    }

                    strncat(
                        record.subjects,
                        book.subjects[subject_number - 1],
                        sizeof(record.subjects) -
                        strlen(record.subjects) -
                        1
                    );
                }

                token = strtok(NULL, ",");
            }
        }
    }

    send_text(
        client_fd,
        "\r\n"
        "Physical location: "
    );

    if (!receive_line(
            client_fd,
            record.location,
            sizeof(record.location)))
    {
        return;
    }

    if (catalog_next_id(
            "data/catalog.db",
            id,
            sizeof(id)) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to generate a catalog ID.\r\n"
        );

        return;
    }

    snprintf(
        record.id,
        sizeof(record.id),
        "%s",
        id
    );

    send_catalog_record(client_fd, &record);

    send_text(
        client_fd,
        "\r\n"
        "1. Save Book\r\n"
        "2. Cancel\r\n"
        "\r\n"
        "Selection: "
    );

    if (!receive_line(
            client_fd,
            input,
            sizeof(input)))
    {
        return;
    }

    if (input[0] != '1')
    {
        send_text(
            client_fd,
            "\r\n"
            "Book not saved.\r\n"
        );

        return;
    }

    if (catalog_save(
            "data/catalog.db",
            &record) != 0)
    {
        send_text(
            client_fd,
            "\r\n"
            "Unable to save book.\r\n"
        );

        return;
    }

    snprintf(
        line,
        sizeof(line),
        "\r\nBook added to catalog as %s.\r\n",
        record.id
    );

    send_text(client_fd, line);
}

static void send_add_book_menu(int client_fd)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        send_text(
            client_fd,
            "\r\n"
            "ADD BOOK\r\n"
            "========\r\n"
            "1. By ISBN\r\n"
            "2. Enter Manually\r\n"
            "3. Return\r\n"
            "\r\n"
            "Selection: "
        );

        if (!receive_line(
                client_fd,
                buffer,
                sizeof(buffer)))
        {
            return;
        }

        switch (buffer[0])
        {
            case '1':
                send_add_book_isbn(client_fd);
                break;

            case '2':
                send_add_book_manual(client_fd);
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

static void library_menu(int client_fd)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        send_text(
            client_fd,
            "\r\n"
            "POWERTOOLS LIBRARY\r\n"
            "==================\r\n"
            "1. Find Book\r\n"
            "2. Add Book\r\n"
            "3. Bibliography\r\n"
            "4. Return\r\n"
            "\r\n"
            "Selection: "
        );

        if (!receive_line(
                client_fd,
                buffer,
                sizeof(buffer)))
        {
            return;
        }

        switch (buffer[0])
        {
            case '1':
                send_library_find(client_fd);
                break;

            case '2':
                send_add_book_menu(client_fd);
                break;

            case '3':
                send_bibliography_find(client_fd);
                break;

            case '4':
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
    library_menu(client_fd);
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