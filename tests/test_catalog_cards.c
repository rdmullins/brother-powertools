#include <stdio.h>
#include <string.h>

#include "cards.h"
#include "card_set.h"

int main(void)
{
    CatalogRecord record;
    CardSet set;

    memset(&record, 0, sizeof(record));

    strcpy(record.author, "Faragher, John Mack");
    strcpy(record.title, "Women and Men on the Overland Trail");
    strcpy(record.place, "New Haven");
    strcpy(record.publisher, "Yale University Press");
    strcpy(record.year, "2001");
    strcpy(record.subjects,
           "Oregon Trail;Western Migration;Women");

    if (cards_build_catalog(&record, &set) != 0) {
        fprintf(stderr,
                "cards_build_catalog failed.\n");
        return 1;
    }

    if (set.count != 5) {
        fprintf(stderr,
                "Expected 5 cards, got %d.\n",
                set.count);
        return 1;
    }

    /*
     * Title card.
     */

if (strcmp(set.cards[0].lines[0],
           "Women and Men on the Overland Trail") != 0) {
    fprintf(stderr,
            "Title card heading is incorrect.\n");
    fprintf(stderr,
            "Got:     [%s]\n",
            set.cards[0].lines[0]);
    fprintf(stderr,
            "Expected:[Women and Men on the Overland Trail]\n");
    return 1;
}

    if (strcmp(set.cards[0].lines[2],
               "Faragher, John Mack") != 0) {
        fprintf(stderr,
                "Title card author is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[0].lines[3],
               "New Haven : Yale University Press, 2001.") != 0) {
        fprintf(stderr,
                "Title card publication line is incorrect.\n");
        return 1;
    }

    /*
     * Author card.
     */

    if (strcmp(set.cards[1].lines[0],
               "Faragher, John Mack") != 0) {
        fprintf(stderr,
                "Author card heading is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[1].lines[2],
               "Women and Men on the Overland Trail") != 0) {
        fprintf(stderr,
                "Author card title is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[1].lines[3],
               "New Haven : Yale University Press, 2001.") != 0) {
        fprintf(stderr,
                "Author card publication line is incorrect.\n");
        return 1;
    }

    /*
     * Subject cards.
     */

    if (strcmp(set.cards[2].lines[0],
               "OREGON TRAIL") != 0) {
        fprintf(stderr,
                "First subject card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[3].lines[0],
               "WESTERN MIGRATION") != 0) {
        fprintf(stderr,
                "Second subject card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[4].lines[0],
               "WOMEN") != 0) {
        fprintf(stderr,
                "Third subject card is incorrect.\n");
        return 1;
    }

    /*
     * Verify subject-card bibliographic information.
     */

    if (strcmp(set.cards[2].lines[2],
               "Faragher, John Mack") != 0) {
        fprintf(stderr,
                "Subject card author is incorrect.\n");
        return 1;
    }

if (strcmp(set.cards[0].lines[0],
           "Women and Men on the Overland Trail") != 0) {
    fprintf(stderr,
            "Title card heading is incorrect.\n");
    fprintf(stderr,
            "Got:     [%s]\n",
            set.cards[0].lines[0]);
    fprintf(stderr,
            "Expected:[Women and Men on the Overland Trail]\n");
    return 1;
}

    if (strcmp(set.cards[2].lines[4],
               "New Haven : Yale University Press, 2001.") != 0) {
        fprintf(stderr,
                "Subject card publication line is incorrect.\n");
        return 1;
    }

    printf("Catalog card set test passed.\n");

    return 0;
}