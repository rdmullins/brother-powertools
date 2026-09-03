#include <stdio.h>
#include <string.h>

#include "notes.h"

#define NOTES_DB "data/notes.db"

static void read_line(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);

    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

int notes_add_for_bibliography(const char *bib_id)
{
    NoteRecord record;

    memset(&record, 0, sizeof(record));

    if (notes_next_id(NOTES_DB, record.id, sizeof(record.id)) != 0) {
        printf("\nUnable to determine the next note ID.\n");
        return -1;
    }

    printf("\nADD RESEARCH NOTE\n");
    printf("-----------------\n\n");

    printf("Note ID: %s\n\n", record.id);

    if (bib_id == NULL || bib_id[0] == '\0') {
        printf("\nInvalid bibliography ID.\n");
        return -1;
    }

    strncpy(record.bib_id, bib_id, sizeof(record.bib_id) - 1);
    record.bib_id[sizeof(record.bib_id) - 1] = '\0';
    read_line("Title:   ", record.title, sizeof(record.title));
    read_line("Locator: ", record.locator, sizeof(record.locator));
    read_line("Use:     ", record.use, sizeof(record.use));

    printf("\nNote:\n");
    read_line("> ", record.text, sizeof(record.text));

    printf("\nSave this note? (y/n): ");

    {
        char answer[8];

        if (!fgets(answer, sizeof(answer), stdin))
            return -1;

        if (answer[0] != 'y' && answer[0] != 'Y') {
            printf("Note not saved.\n");
            return -1;
        }
    }

    if (notes_save(NOTES_DB, &record) != 0) {
        printf("Unable to save note.\n");
        return -1;
    }

    printf("Note %s saved.\n", record.id);
    return 0;
}

static void find_note(void)
{
    char id[NOTE_MAX_ID];
    NoteRecord record;

    memset(&record, 0, sizeof(record));

    read_line("\nNote ID: ", id, sizeof(id));

    if (notes_load(NOTES_DB, id, &record) != 0) {
        printf("\nNote %s not found.\n", id);
        return;
    }

    printf("\n");
    printf("RESEARCH NOTE\n");
    printf("-------------\n");
    printf("ID:      %s\n", record.id);
    printf("Bib ID:  %s\n", record.bib_id);
    printf("Title:   %s\n", record.title);
    printf("Locator: %s\n", record.locator);
    printf("Use:     %s\n", record.use);
    printf("\n");
    printf("Note:\n");
    printf("%s\n", record.text);
}

void notes_menu(void)
{
    char choice[8];

    for (;;) {
        printf("\n");
        printf("RESEARCH NOTES\n");
        printf("--------------\n");
        printf("1. Add Note\n");
        printf("2. Find Note\n");
        printf("3. Print Note Card\n");
        printf("4. Back\n");
        printf("\nChoice: ");

        if (!fgets(choice, sizeof(choice), stdin))
            return;

        switch (choice[0]) {
        case '1':
            printf("\nAdd Note from a bibliography record.\n");
            printf("Please select a bibliography record first.\n");
            break;

        case '2':
            find_note();
            break;

        case '3':
            printf("\nPrint Note Card: coming soon.\n");
            break;

        case '4':
            return;

        default:
            printf("Invalid choice.\n");
            break;
        }
    }
}