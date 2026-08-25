#include <stdio.h>
#include <string.h>

#include "cards.h"
#include "citation.h"

#define SUBJECT_BUFFER_SIZE 1024

void card_init(Card *card)
{
    if (card == NULL) {
        return;
    }

    memset(card, 0, sizeof(*card));
}

int card_add_line(Card *card, const char *text)
{
    if (card == NULL || text == NULL) {
        return -1;
    }

    if (card->line_count >= CARD_HEIGHT) {
        return -1;
    }

    if (strlen(text) > CARD_WIDTH) {
        return -1;
    }

    strcpy(card->lines[card->line_count], text);
    card->line_count++;

    return 0;
}

int card_add_wrapped_text(Card *card, const char *text)
{
    const char *start;
    const char *word;
    size_t current_length = 0;

    if (card == NULL || text == NULL) {
        return -1;
    }

    start = text;

    while (*start != '\0') {
        size_t word_length;
        size_t space_needed;

        while (*start == ' ') {
            start++;
        }

        if (*start == '\0') {
            break;
        }

        word = start;

        while (*start != '\0' && *start != ' ') {
            start++;
        }

        word_length = (size_t)(start - word);

        /*
         * A word longer than the physical card width cannot
         * fit on one line.
         */
        if (word_length > CARD_WIDTH) {
            return -1;
        }

        space_needed = current_length == 0 ? 0 : 1;

        if (current_length + space_needed + word_length > CARD_WIDTH) {
            char line[CARD_WIDTH + 1];

            /*
             * The current line is already stored in the card.
             * Start a new line for this word.
             */
            current_length = 0;

            if (card->line_count >= CARD_HEIGHT) {
                return -1;
            }

            line[0] = '\0';

            strncpy(line, word, word_length);
            line[word_length] = '\0';

            if (card_add_line(card, line) != 0) {
                return -1;
            }

            current_length = word_length;
        } else {
            /*
             * If this is the first word on a line, create the
             * line. Otherwise append to the existing line.
             */
            if (current_length == 0) {
                char line[CARD_WIDTH + 1];

                strncpy(line, word, word_length);
                line[word_length] = '\0';

                if (card_add_line(card, line) != 0) {
                    return -1;
                }
            } else {
                char *line = card->lines[card->line_count - 1];

                line[current_length] = ' ';
                strncpy(line + current_length + 1,
                        word,
                        word_length);

                line[current_length + 1 + word_length] = '\0';
            }

            current_length += space_needed + word_length;
        }
    }

    return 0;
}

int card_add_hanging_text(Card *card,
                          const char *text,
                          int indent)
{
    const char *start;
    int first_line = 1;
    char line[CARD_WIDTH + 1];
    size_t line_length = 0;

    if (card == NULL || text == NULL) {
        return -1;
    }

    if (indent < 0 || indent >= CARD_WIDTH) {
        return -1;
    }

    start = text;

    while (1) {
        const char *word_start;
        size_t word_length;
        size_t available_width;

        /*
         * Skip spaces between words.
         */
        while (*start == ' ') {
            start++;
        }

        if (*start == '\0') {
            break;
        }

        word_start = start;

        while (*start != '\0' && *start != ' ') {
            start++;
        }

        word_length = (size_t)(start - word_start);

        available_width = first_line
                        ? CARD_WIDTH
                        : CARD_WIDTH - (size_t)indent;

        /*
         * A single word cannot fit on a card line.
         */
        if (word_length > available_width) {
            return -1;
        }

        /*
         * If the word won't fit on the current line,
         * finish that line and start a new one.
         */
        if (line_length > 0 &&
            line_length + 1 + word_length > available_width) {

            char output_line[CARD_WIDTH + 1];

            if (first_line) {
                strcpy(output_line, line);
            } else {
                memset(output_line, ' ', (size_t)indent);
                memcpy(output_line + indent,
                       line,
                       line_length);

                output_line[indent + line_length] = '\0';
            }

            if (card_add_line(card, output_line) != 0) {
                return -1;
            }

            first_line = 0;
            line_length = 0;
        }

        /*
         * Add the word to the current line.
         */
        if (line_length > 0) {
            line[line_length++] = ' ';
        }

        memcpy(line + line_length,
               word_start,
               word_length);

        line_length += word_length;
        line[line_length] = '\0';
    }

    /*
     * Add the final line.
     */
    if (line_length > 0) {
        char output_line[CARD_WIDTH + 1];

        if (first_line) {
            strcpy(output_line, line);
        } else {
            memset(output_line, ' ', (size_t)indent);
            memcpy(output_line + indent,
                   line,
                   line_length);

            output_line[indent + line_length] = '\0';
        }

        if (card_add_line(card, output_line) != 0) {
            return -1;
        }
    }

    return 0;
}

void card_print(const Card *card)
{
    if (card == NULL) {
        return;
    }

    for (int i = 0; i < CARD_HEIGHT; i++) {
        printf("%s\n", card->lines[i]);
    }

    /*
     * Separate physical cards during testing.
     */
    printf("\n");
}

static void print_author_card(const CatalogRecord *record)
{
    Card card;

    card_init(&card);

    card_add_line(&card, record->author);
    card_add_line(&card, "");
    card_add_wrapped_text(&card, record->title);

    card_print(&card);
}

static void print_title_card(const CatalogRecord *record)
{
    Card card;

    card_init(&card);

    card_add_wrapped_text(&card, record->title);

    card_add_line(&card, "");
    card_add_line(&card, record->author);

    card_print(&card);
}

static void print_subject_cards(const CatalogRecord *record)
{
    char subjects[SUBJECT_BUFFER_SIZE];
    char *subject;

    strncpy(subjects,
            record->subjects,
            sizeof(subjects) - 1);

    subjects[sizeof(subjects) - 1] = '\0';

    subject = strtok(subjects, ";");

    while (subject != NULL) {
        Card card;

        card_init(&card);

        card_add_line(&card, subject);
        card_add_line(&card, "");
        card_add_line(&card, record->author);
        card_add_line(&card, record->title);

        card_print(&card);

        subject = strtok(NULL, ";");
    }
}

void cards_print_catalog(const CatalogRecord *record)
{
    if (record == NULL) {
        return;
    }

    print_author_card(record);
    print_title_card(record);
    print_subject_cards(record);
}

int cards_print_bibliography(const BibliographyRecord *record)
{
    Card card;
    char citation[CITATION_MAX_LENGTH];

    if (record == NULL) {
        return -1;
    }

    if (citation_generate(record,
                           citation,
                           sizeof(citation)) != 0) {
        return -1;
    }

    card_init(&card);

    if (card_add_hanging_text(&card,
                              citation,
                              4) != 0) {
        return -1;
    }

    card_print(&card);

    return 0;
}