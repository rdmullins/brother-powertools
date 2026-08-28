#ifndef CARD_SET_H
#define CARD_SET_H

#include <stdio.h>

#include "cards.h"

#define CARD_SET_MAX_CARDS 256

typedef struct CardSet {
    Card cards[CARD_SET_MAX_CARDS];
    int count;
} CardSet;

void card_set_init(CardSet *set);

int card_set_add(CardSet *set, const Card *card);

int card_set_write(const CardSet *set,
                   const char *filename);

#endif