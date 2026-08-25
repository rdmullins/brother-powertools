#include <stdio.h>
#include <string.h>

#include "citation.h"

int citation_generate(const BibliographyRecord *record,
                      char *output,
                      size_t output_size)
{
    if (record == NULL ||
        output == NULL ||
        output_size == 0) {
        return -1;
    }

    output[0] = '\0';

    /*
     * An explicit citation override always wins.
     */
    if (record->citation[0] != '\0') {
        if (strlen(record->citation) >= output_size) {
            return -1;
        }

        strcpy(output, record->citation);
        return 0;
    }

    /*
     * First automatic formatter: journal article.
     */
    if (strcmp(record->type, "ARTICLE") == 0) {
        int written;

        written = snprintf(
            output,
            output_size,
            "%s. \"%s.\" %s %s, no. %s (%s): %s.",
            record->author,
            record->title,
            record->journal,
            record->volume,
            record->issue,
            record->date,
            record->pages
        );

        if (written < 0 ||
            (size_t)written >= output_size) {
            output[0] = '\0';
            return -1;
        }

        return 0;
    }

    /*
     * We don't know how to automatically cite this
     * source type yet.
     */
    return 1;
}