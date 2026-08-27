#include <stdio.h>

#include "card_set.h"
#include "notes.h"

int main(void)
{
    NoteRecord note;
    Card card1;
    Card card2;
    CardSet set;
    FILE *file;
    int character;
    int form_feeds = 0;

    if (notes_load("data/notes.db",
                   "NC000051",
                   &note) != 0) {
        fprintf(stderr, "Unable to load NC000051.\n");
        return 1;
    }

    if (card_build_note(&note, &card1) != 0) {
        fprintf(stderr, "Unable to build first note card.\n");
        return 1;
    }

    if (card_build_note(&note, &card2) != 0) {
        fprintf(stderr, "Unable to build second note card.\n");
        return 1;
    }

    card_set_init(&set);

    if (card_set_add(&set, &card1) != 0) {
        return 1;
    }

    if (card_set_add(&set, &card2) != 0) {
        return 1;
    }

    if (card_set_write(&set,
                       "/tmp/test-card-set.wpt") != 0) {
        fprintf(stderr, "Unable to write card set.\n");
        return 1;
    }

    file = fopen("/tmp/test-card-set.wpt", "rb");

    if (file == NULL) {
        perror("Unable to open test card set");
        return 1;
    }

    while ((character = fgetc(file)) != EOF) {
        if (character == '\f') {
            form_feeds++;
        }
    }

    fclose(file);

    if (form_feeds != 1) {
        fprintf(stderr,
                "Expected 1 form-feed, found %d\n",
                form_feeds);
        return 1;
    }

    if (set.count != 2) {
        fprintf(stderr,
                "Expected 2 cards, found %d\n",
                set.count);
        return 1;
    }

    printf("Real card set test passed.\n");

    return 0;
}