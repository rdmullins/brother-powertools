#ifndef CARDS_H
#define CARDS_H
#define CARD_WIDTH 47
#define CARD_HEIGHT 15

#include "catalog.h"
#include "bibliography.h"
void cards_print_catalog(const CatalogRecord *record);

typedef struct {
    char lines[CARD_HEIGHT][CARD_WIDTH + 1];
    int line_count;
} Card;

void card_init(Card *card);
int card_add_line(Card *card, const char *text);
void card_print(const Card *card);
int card_add_wrapped_text(Card *card, const char *text);
int card_add_hanging_text(Card *card,
                          const char *text,
                          int indent);
int cards_print_bibliography(const BibliographyRecord *record);                          
#endif