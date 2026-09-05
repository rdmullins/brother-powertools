#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bibliography_add.h"
#include "bibliography.h"
#include "catalog.h"
#include "citation.h"

#define BIB_DB "data/bibliography.db"
#define CATALOG_DB "data/catalog.db"
#define CATALOG_SEARCH_MAX_RESULTS 20

static void clear_input_line(void)
{
    int character;

    while ((character = getchar()) != '\n' &&
           character != EOF) {
        /* discard remaining input */
    }
}

static void copy_field(char *destination,
                       size_t destination_size,
                       const char *value)
{
    if (destination_size == 0) {
        return;
    }

    strncpy(destination,
            value,
            destination_size - 1);

    destination[destination_size - 1] = '\0';
}

static void read_field(const char *prompt,
                       char *buffer,
                       size_t buffer_size)
{
    printf("%s", prompt);

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    if (strchr(buffer, '\n') == NULL) {
        clear_input_line();
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';
}

static void display_catalog_result(const CatalogRecord *record,
                                   int number)
{
    printf("\n%d. %s\n",
           number,
           record->title);

    printf("   %s\n",
           record->author);

    printf("   %s, %s\n",
           record->publisher,
           record->year);

    printf("   Catalog ID: %s\n",
           record->id);
}

static void display_edit_value(const char *label,
                               const char *value)
{
    printf("%-13s %s\n", label, value);
}

static int bibliography_edit_field(BibliographyRecord *record)
{
    char input[4096];
    int choice;

    if (record == NULL) {
        return -1;
    }

    for (;;) {
        printf("\n");
        printf("EDIT BIBLIOGRAPHY ENTRY\n");
        printf("-----------------------\n");

        display_edit_value("1. Type:", record->type);
        display_edit_value("2. Author:", record->author);
        display_edit_value("3. Title:", record->title);
        display_edit_value("4. Journal:", record->journal);
        display_edit_value("5. Place:", record->place);
        display_edit_value("6. Publisher:", record->publisher);
        display_edit_value("7. Institution:", record->institution);
        display_edit_value("8. Report No.:", record->report_number);
        display_edit_value("9. Division:", record->division);
        display_edit_value("10. Volume:", record->volume);
        display_edit_value("11. Issue:", record->issue);
        display_edit_value("12. Date:", record->date);
        display_edit_value("13. Pages:", record->pages);
        display_edit_value("14. Source:", record->source);
        display_edit_value("15. Location:", record->location);

        printf("\n");
        printf("16. Done\n");
        printf("17. Cancel\n");
        printf("\n");
        printf("Select field: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            return -1;
        }

        choice = atoi(input);

        switch (choice) {
        case 1:
            read_field("Type: ",
                       record->type,
                       sizeof(record->type));
            break;

        case 2:
            read_field("Author: ",
                       record->author,
                       sizeof(record->author));
            break;

        case 3:
            read_field("Title: ",
                       record->title,
                       sizeof(record->title));
            break;

        case 4:
            read_field("Journal: ",
                       record->journal,
                       sizeof(record->journal));
            break;

        case 5:
            read_field("Place: ",
                       record->place,
                       sizeof(record->place));
            break;

        case 6:
            read_field("Publisher: ",
                       record->publisher,
                       sizeof(record->publisher));
            break;

        case 7:
            read_field("Institution: ",
                       record->institution,
                       sizeof(record->institution));
            break;

        case 8:
            read_field("Report Number: ",
                       record->report_number,
                       sizeof(record->report_number));
            break;

        case 9:
            read_field("Division: ",
                       record->division,
                       sizeof(record->division));
            break;

        case 10:
            read_field("Volume: ",
                       record->volume,
                       sizeof(record->volume));
            break;

        case 11:
            read_field("Issue: ",
                       record->issue,
                       sizeof(record->issue));
            break;

        case 12:
            read_field("Date: ",
                       record->date,
                       sizeof(record->date));
            break;

        case 13:
            read_field("Pages: ",
                       record->pages,
                       sizeof(record->pages));
            break;

        case 14:
            read_field("Source: ",
                       record->source,
                       sizeof(record->source));
            break;

        case 15:
            read_field("Location: ",
                       record->location,
                       sizeof(record->location));
            break;

        case 16:
            return 0;

        case 17:
            return 1;

        default:
            printf("\nInvalid selection.\n");
            break;
        }
    }
}

void bibliography_add_from_catalog(void)
{
    CatalogRecord results[CATALOG_SEARCH_MAX_RESULTS];
    BibliographyRecord record;
    char query[256];
    char selection[32];
    char id[32];
    char answer;
    char *end;
    long choice;
    int count;
    int edit_result;

    memset(&record, 0, sizeof(record));

    printf("\n");
    printf("Add from Catalog\n");
    printf("----------------\n");

    printf("Search catalog: ");

    if (fgets(query, sizeof(query), stdin) == NULL) {
        return;
    }

    query[strcspn(query, "\r\n")] = '\0';

    if (query[0] == '\0') {
        printf("\nSearch cancelled.\n");
        return;
    }

    count = catalog_search(CATALOG_DB,
                           query,
                           results,
                           CATALOG_SEARCH_MAX_RESULTS);

    if (count < 0) {
        printf("\nUnable to search catalog.\n");
        return;
    }

    if (count == 0) {
        printf("\nNo catalog entries found.\n");
        return;
    }

    printf("\nCATALOG RESULTS\n");
    printf("----------------\n");

    for (int i = 0; i < count; i++) {
        display_catalog_result(&results[i], i + 1);
    }

    printf("\nSelect book (1-%d, or 0 to cancel): ",
           count);

    if (fgets(selection, sizeof(selection), stdin) == NULL) {
        return;
    }

    choice = strtol(selection, &end, 10);

    if (end == selection ||
        choice < 0 ||
        choice > count) {
        printf("\nInvalid selection.\n");
        return;
    }

    if (choice == 0) {
        return;
    }

    CatalogRecord *catalog = &results[choice - 1];

    if (bibliography_exists_for_catalog(BIB_DB,
                                        catalog->id) == 1) {
        printf("\nThis catalog item is already in the bibliography.\n");
        return;
    }

    /*
     * Populate the bibliography record from the catalog.
     */
    copy_field(record.catalog_id,
               sizeof(record.catalog_id),
               catalog->id);

    copy_field(record.type,
               sizeof(record.type),
               "BOOK");

    copy_field(record.author,
               sizeof(record.author),
               catalog->author);

    copy_field(record.title,
               sizeof(record.title),
               catalog->title);

    copy_field(record.place,
               sizeof(record.place),
               catalog->place);

    copy_field(record.publisher,
               sizeof(record.publisher),
               catalog->publisher);

    copy_field(record.date,
               sizeof(record.date),
               catalog->year);

    copy_field(record.location,
               sizeof(record.location),
               catalog->location);

    if (bibliography_next_id(BIB_DB,
                             id,
                             sizeof(id)) != 0) {
        printf("\nUnable to determine the next bibliography ID.\n");
        return;
    }

    copy_field(record.id,
               sizeof(record.id),
               id);

    /*
     * Give the user a chance to correct or complete
     * anything imported from the catalog.
     */
    edit_result = bibliography_edit_field(&record);

    if (edit_result < 0) {
        printf("\nUnable to edit bibliography entry.\n");
        return;
    }

if (edit_result == 1) {
    printf("\nBibliography entry cancelled.\n");
    return;
}

if (record.title[0] == '\0') {
    printf("\nA title is required. Entry not added.\n");
    return;
}

if (citation_generate(&record,
                      record.citation,
                      sizeof(record.citation)) != 0) {
    printf("\nUnable to generate citation.\n");
    return;
}

    printf("\nBibliography Record\n");
    printf("-------------------\n");

    bibliography_display(&record);

    printf("\nAdd this entry? (y/n): ");

    if (fgets(selection, sizeof(selection), stdin) == NULL) {
        return;
    }

    answer = selection[0];

    if (answer != 'y' && answer != 'Y') {
        printf("\nBibliography entry not added.\n");
        return;
    }

    if (bibliography_save(BIB_DB,
                          &record) != 0) {
        printf("\nUnable to save bibliography entry.\n");
        return;
    }

    printf("\nBibliography entry %s created.\n",
           record.id);
}