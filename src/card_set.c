#include <stdio.h>
#include <string.h>

#include "card_set.h"

void card_set_init(CardSet *set)
{
    if (set == NULL) {
        return;
    }

    memset(set, 0, sizeof(*set));
}

int card_set_add(CardSet *set, const Card *card)
{
    if (set == NULL || card == NULL) {
        return -1;
    }

    if (set->count >= CARD_SET_MAX_CARDS) {
        return -1;
    }

    set->cards[set->count] = *card;
    set->count++;

    return 0;
}

int card_set_write(const CardSet *set,
                   const char *filename)
{
    FILE *output;
    int i;

    if (set == NULL || filename == NULL) {
        return -1;
    }

    output = fopen(filename, "w");

    if (output == NULL) {
        perror("Unable to open card set");
        return -1;
    }

    for (i = 0; i < set->count; i++) {
        card_print_file(&set->cards[i], output);

        if (i < set->count - 1) {
            fputc('\f', output);
        }
    }

    fclose(output);

    return 0;
}