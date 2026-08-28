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

    if (strcmp(set.cards[0].lines[0],
               "Faragher, John Mack") != 0) {
        fprintf(stderr,
                "Author card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[1].lines[0],
               "Women and Men on the Overland Trail") != 0) {
        fprintf(stderr,
                "Title card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[2].lines[0],
               "Oregon Trail") != 0) {
        fprintf(stderr,
                "First subject card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[3].lines[0],
               "Western Migration") != 0) {
        fprintf(stderr,
                "Second subject card is incorrect.\n");
        return 1;
    }

    if (strcmp(set.cards[4].lines[0],
               "Women") != 0) {
        fprintf(stderr,
                "Third subject card is incorrect.\n");
        return 1;
    }

    printf("Catalog card set test passed.\n");

    return 0;
}