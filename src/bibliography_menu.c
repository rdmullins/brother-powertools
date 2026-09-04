#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "notes.h"
#include "bibliography.h"
#include "cards.h"
#include "card_set.h"
#include "transfer.h"

#define BIB_DB "data/bibliography.db"
#define BIB_MAX_RESULTS 50
#define NOTES_DB "data/notes.db"
#define NOTE_MAX_RESULTS 50

static void read_line(const char *prompt,
                      char *buffer,
                      size_t buffer_size)
{
    printf("%s", prompt);

    if (fgets(buffer, buffer_size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

static void display_search_result(const BibliographyRecord *record,
                                  int number)
{
    printf("\n%d. %s\n", number, record->id);
    printf("   %s\n", record->author);
    printf("   %s\n", record->title);
}

static void view_notes(const char *bib_id)
{
    NoteRecord results[NOTE_MAX_RESULTS];
    char selection[32];
    char *end;
    long choice;
    int count;

    count = notes_search_by_bibliography(NOTES_DB,
                                         bib_id,
                                         results,
                                         NOTE_MAX_RESULTS);

    if (count < 0) {
        printf("\nUnable to search research notes.\n");
        return;
    }

    printf("\nRESEARCH NOTES\n");
    printf("--------------\n");

    if (count == 0) {
        printf("\nNo notes found for this source.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("\n%d. %s", i + 1, results[i].id);

        if (results[i].title[0] != '\0') {
            printf(" - %s", results[i].title);
        }

        if (results[i].locator[0] != '\0') {
            printf(" (%s)", results[i].locator);
        }

        printf("\n");
    }

    printf("\nSelect note (1-%d, or 0 to cancel): ", count);

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

    {
        NoteRecord *record = &results[choice - 1];

        printf("\n");
        printf("RESEARCH NOTE\n");
        printf("-------------\n");
        printf("ID:      %s\n", record->id);
        printf("Source:  %s\n", record->bib_id);
        printf("Title:   %s\n", record->title);
        printf("Locator: %s\n", record->locator);
        printf("Use:     %s\n", record->use);
        printf("\n");
        printf("Note:\n");
        printf("%s\n", record->text);

        
    }

    
}

static void print_note_cards(const BibliographyRecord *record)
{
    CardSet set;
    char filename[256];

    if (record == NULL) {
        printf("\nUnable to print note cards.\n");
        return;
    }

    if (cards_build_source_set(record, &set) != 0) {
        printf("\nUnable to build source card set.\n");
        return;
    }

    if (set.count == 0) {
        printf("\nNo cards found for this source.\n");
        return;
    }

    snprintf(filename,
             sizeof(filename),
             "/tmp/%s-card-set.wpt",
             record->id);

    if (card_set_write(&set, filename) != 0) {
        printf("\nUnable to create card set file.\n");
        return;
    }

    printf("\n");
    printf("Prepared %d cards for %s.\n",
           set.count,
           record->id);

    if (send_ascii_file(filename) != 0) {
        printf("\nUnable to transfer card set to the Brother.\n");
        return;
    }

    printf("\nCard set transferred successfully.\n");
}

static void bibliography_search_menu(void)
{
    BibliographyRecord results[BIB_MAX_RESULTS];
    char query[256];
    char selection[32];
    char *end;
    long choice;
    int result_count;

    read_line("\nSearch bibliography: ",
              query,
              sizeof(query));

    if (query[0] == '\0') {
        printf("\nSearch cancelled.\n");
        return;
    }

    result_count = bibliography_search(BIB_DB,
                                       query,
                                       results,
                                       BIB_MAX_RESULTS);

    if (result_count < 0) {
        printf("\nUnable to search bibliography.\n");
        return;
    }

    if (result_count == 0) {
        printf("\nNo bibliography entries found.\n");
        return;
    }

    printf("\nBIBLIOGRAPHY RESULTS\n");
    printf("--------------------\n");

    for (int i = 0; i < result_count; i++) {
        display_search_result(&results[i], i + 1);
    }

    printf("\nSelect entry (1-%d, or 0 to cancel): ",
           result_count);

    if (fgets(selection, sizeof(selection), stdin) == NULL) {
        return;
    }

    choice = strtol(selection, &end, 10);

    if (end == selection || choice < 0 ||
        choice > result_count) {
        printf("\nInvalid selection.\n");
        return;
    }

    if (choice == 0) {
        return;
    }

    for (;;) {
        BibliographyRecord *record = &results[choice - 1];

        bibliography_display(record);

        printf("\n");
        printf("1. Add Note\n");
        printf("2. View Notes\n");
        printf("3. Print Bibliography Card\n");
        printf("4. Print Note Cards\n");
        printf("5. Back\n");
        printf("\nChoice: ");

        read_line("", selection, sizeof(selection));

    switch (selection[0]) {
    case '1':
        if (notes_add_for_bibliography(record->id) == 0) {
            printf("\nNote saved.\n");
        } else {
            printf("\nUnable to save note.\n");
        }
        break;

    case '2':
        view_notes(record->id);
        break;

    case '3':
        if (cards_print_bibliography(record) != 0) {
            printf("\nUnable to print bibliography card.\n");
        }
        break;

    case '4':
        print_note_cards(record);
        break;

    case '5':
        return;

    default:
        printf("\nInvalid choice.\n");
        break;
    }
}
}

void bibliography_menu(void)
{
    char choice[16];

    for (;;) {
        printf("\n");
        printf("Bibliography\n");
        printf("------------\n");
        printf("1. Search\n");
        printf("2. Back\n");
        printf("\n");
        printf("Select: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            return;
        }

        switch (choice[0]) {
        case '1':
            bibliography_search_menu();
            break;

        case '2':
            return;

        default:
            printf("\nInvalid selection.\n");
            break;
        }
    }
}