#include <stdio.h>
#include <string.h>

#include "cards.h"
#include "citation.h"
#include "card_set.h"

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

void card_print_file(const Card *card, FILE *output)
{
    int i;

    if (card == NULL || output == NULL) {
        return;
    }

    for (i = 0; i < card->line_count; i++) {
        fprintf(output, "%s\n", card->lines[i]);
    }
}

void card_print(const Card *card)
{
    card_print_file(card, stdout);
}

static int build_author_card(const CatalogRecord *record,
                             Card *card)
{
    if (record == NULL || card == NULL) {
        return -1;
    }

    card_init(card);

    if (card_add_line(card, record->author) != 0) {
        return -1;
    }

    if (card_add_line(card, "") != 0) {
        return -1;
    }

    if (card_add_wrapped_text(card, record->title) != 0) {
        return -1;
    }

    return 0;
}

static int build_title_card(const CatalogRecord *record,
                            Card *card)
{
    if (record == NULL || card == NULL) {
        return -1;
    }

    card_init(card);

    if (card_add_wrapped_text(card, record->title) != 0) {
        return -1;
    }

    if (card_add_line(card, "") != 0) {
        return -1;
    }

    if (card_add_line(card, record->author) != 0) {
        return -1;
    }

    return 0;
}

static int build_subject_cards(const CatalogRecord *record,
                               CardSet *set)
{
    char subjects[SUBJECT_BUFFER_SIZE];
    char *subject;

    if (record == NULL || set == NULL) {
        return -1;
    }

    strncpy(subjects,
            record->subjects,
            sizeof(subjects) - 1);

    subjects[sizeof(subjects) - 1] = '\0';

    subject = strtok(subjects, ";");

    while (subject != NULL) {
        Card card;

        card_init(&card);

        if (card_add_line(&card, subject) != 0) {
            return -1;
        }

        if (card_add_line(&card, "") != 0) {
            return -1;
        }

        if (card_add_line(&card, record->author) != 0) {
            return -1;
        }

        if (card_add_line(&card, record->title) != 0) {
            return -1;
        }

        if (card_set_add(set, &card) != 0) {
            return -1;
        }

        subject = strtok(NULL, ";");
    }

    return 0;
}

int cards_build_catalog(const CatalogRecord *record,
                        CardSet *set)
{
    Card card;

    if (record == NULL || set == NULL) {
        return -1;
    }

    card_set_init(set);

    if (build_author_card(record, &card) != 0) {
        return -1;
    }

    if (card_set_add(set, &card) != 0) {
        return -1;
    }

    if (build_title_card(record, &card) != 0) {
        return -1;
    }

    if (card_set_add(set, &card) != 0) {
        return -1;
    }

    if (build_subject_cards(record, set) != 0) {
        return -1;
    }

    return 0;
}

void cards_print_catalog(const CatalogRecord *record)
{
    CardSet set;

    if (record == NULL) {
        return;
    }

    if (cards_build_catalog(record, &set) != 0) {
        return;
    }

    for (int i = 0; i < set.count; i++) {
        card_print(&set.cards[i]);
    }
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

int card_build_note(const NoteRecord *record, Card *card)
{

    int reserved_lines = 2;
    int available_lines;

    if (record == NULL) {
        return -1;
    }

    card_init(card);

    /*
     * The final two lines are reserved for the source
     * and locator.
     */
    available_lines = CARD_HEIGHT - reserved_lines;

    /*
     * Optional title.
     */
    if (record->title[0] != '\0') {
        if (card_add_wrapped_text(card,
                                  record->title) != 0) {
            return -1;
        }

        /*
         * Leave one blank line between the title and
         * the note body.
         */
        if (card->line_count >= available_lines) {
            return -1;
        }

        if (card_add_line(card, "") != 0) {
            return -1;
        }
    }

    /*
     * Add the note itself.
     */
    if (card_add_wrapped_text(card,
                              record->text) != 0) {
        return -1;
    }

    /*
     * The note body must leave two lines for the
     * source/locator block.
     */
    if (card->line_count > available_lines) {
        return -1;
    }

    /*
     * Pad until we reach the source/locator area.
     */
    while (card->line_count < available_lines) {
        if (card_add_line(card, "") != 0) {
            return -1;
        }
    }

    /*
     * Source and locator occupy the final two lines.
     * For now, right-align them against the 47-column
     * card width.
     */
    {
        char source_line[CARD_WIDTH + 1];
        char locator_line[CARD_WIDTH + 1];

        int written;

        written = snprintf(source_line,
                           sizeof(source_line),
                           "Source %s",
                           record->source);

        if (written < 0 || written > CARD_WIDTH) {
            return -1;
        }

        written = snprintf(locator_line,
                           sizeof(locator_line),
                           "%s",
                           record->locator);

        if (written < 0 || written > CARD_WIDTH) {
            return -1;
        }

        /*
         * Right-align both attribution lines.
         */
        {
            char formatted_source[CARD_WIDTH + 1];
            char formatted_locator[CARD_WIDTH + 1];
            size_t source_length = strlen(source_line);
            size_t locator_length = strlen(locator_line);

            memset(formatted_source, ' ', CARD_WIDTH);
            memset(formatted_locator, ' ', CARD_WIDTH);

            memcpy(formatted_source +
                       CARD_WIDTH - source_length,
                   source_line,
                   source_length);

            memcpy(formatted_locator +
                       CARD_WIDTH - locator_length,
                   locator_line,
                   locator_length);

            formatted_source[CARD_WIDTH] = '\0';
            formatted_locator[CARD_WIDTH] = '\0';

            if (card_add_line(card,
                              formatted_source) != 0) {
                return -1;
            }

            if (card_add_line(card,
                              formatted_locator) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

int cards_print_note(const NoteRecord *record)
{
    Card card;

    if (card_build_note(record, &card) != 0) {
        return -1;
    }

    card_print(&card);

    return 0;
}

int cards_write_note(const NoteRecord *record,
                     const char *filename)
{
    Card card;
    FILE *output;

    if (record == NULL || filename == NULL) {
        return -1;
    }

    if (card_build_note(record, &card) != 0) {
        return -1;
    }

    output = fopen(filename, "w");

    if (output == NULL) {
        perror("Unable to open card file");
        return -1;
    }

    card_print_file(&card, output);

    fclose(output);

    return 0;
}