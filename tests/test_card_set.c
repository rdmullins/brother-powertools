#include <stdio.h>
#include <string.h>

#include "card_set.h"
#include "cards.h"
#include "notes.h"
#include "bibliography.h"

int main(void)
{
    NoteRecord note;
    Card card1;
    Card card2;
    CardSet set;
    CardSet source_set;
    BibliographyRecord bibliography;
    FILE *file;
    int character;
    int form_feeds = 0;

    /*
     * Basic card-set test.
     */

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

    /*
     * Source-set test.
     *
     * BC000001 currently has two research notes.
     * The resulting set should contain:
     *
     *   1. bibliography card
     *   2. NC000005 note
     *   3. NC000052 note
     */

    if (bibliography_load("data/bibliography.db",
                          "BC000001",
                          &bibliography) != 0) {
        fprintf(stderr, "Unable to load BC000001.\n");
        return 1;
    }

    if (cards_build_source_set(&bibliography,
                               &source_set) != 0) {
        fprintf(stderr, "Unable to build source card set.\n");
        return 1;
    }

    if (source_set.count != 3) {
        fprintf(stderr,
                "Expected 3 cards in source set, found %d\n",
                source_set.count);
        return 1;
    }

    /*
     * First card is the bibliography card.
     */

    if (strcmp(source_set.cards[0].lines[0],
               "BC000001") != 0) {
        fprintf(stderr,
                "First card is not bibliography card BC000001.\n");
        return 1;
    }

    /*
     * Second card is NC000005.
     * The note ID is not printed on the card, so verify
     * its distinctive text.
     */

    if (strstr(source_set.cards[1].lines[0],
               "Paul Boyer notes") == NULL) {
        fprintf(stderr,
                "Second card does not contain NC000005 note text.\n");
        return 1;
    }

    /*
     * Third card is NC000052, whose title is printed first.
     */

    if (strcmp(source_set.cards[2].lines[0],
               "Fallout Shelter Sales") != 0) {
        fprintf(stderr,
                "Third card does not contain NC000052 title.\n");
        return 1;
    }

    /*
     * Verify the source attribution without assuming
     * its left/right alignment.
     */

    if (strstr(source_set.cards[1].lines[CARD_HEIGHT - 2],
               "Source BC000001") == NULL) {
        fprintf(stderr,
                "Second card has incorrect source.\n");
        return 1;
    }

    if (strstr(source_set.cards[2].lines[CARD_HEIGHT - 2],
               "Source BC000001") == NULL) {
        fprintf(stderr,
                "Third card has incorrect source.\n");
        return 1;
    }

    /*
     * Write the complete source set and verify that three
     * cards produce two form-feeds.
     */

    if (card_set_write(&source_set,
                       "/tmp/test-source-set.wpt") != 0) {
        fprintf(stderr, "Unable to write source card set.\n");
        return 1;
    }

    file = fopen("/tmp/test-source-set.wpt", "rb");

    if (file == NULL) {
        perror("Unable to open source card set");
        return 1;
    }

    form_feeds = 0;

    while ((character = fgetc(file)) != EOF) {
        if (character == '\f') {
            form_feeds++;
        }
    }

    fclose(file);

    if (form_feeds != 2) {
        fprintf(stderr,
                "Expected 2 form-feeds in source set, found %d\n",
                form_feeds);
        return 1;
    }

    printf("Real card set test passed.\n");
    printf("Source card set test passed.\n");

    return 0;
}