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
 * Automatic formatter: book.
 */
if (strcmp(record->type, "BOOK") == 0) {
    int written;

    if (record->author[0] != '\0' &&
        record->place[0] != '\0' &&
        record->publisher[0] != '\0' &&
        record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s: %s, %s.",
            record->author,
            record->title,
            record->place,
            record->publisher,
            record->date
        );

    } else if (record->author[0] != '\0' &&
               record->place[0] != '\0' &&
               record->publisher[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s: %s.",
            record->author,
            record->title,
            record->place,
            record->publisher
        );

    } else if (record->author[0] != '\0' &&
               record->publisher[0] != '\0' &&
               record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s, %s.",
            record->author,
            record->title,
            record->publisher,
            record->date
        );

    } else if (record->author[0] != '\0' &&
               record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s.",
            record->author,
            record->title,
            record->date
        );

    } else if (record->author[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s.",
            record->author,
            record->title
        );

    } else if (record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s.",
            record->title,
            record->date
        );

    } else {

        written = snprintf(
            output,
            output_size,
            "%s.",
            record->title
        );
    }

    if (written < 0 ||
        (size_t)written >= output_size) {
        output[0] = '\0';
        return -1;
    }

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
     * Automatic formatter: report.
     */
    if (strcmp(record->type, "REPORT") == 0) {
        int written;

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s. %s, %s, %s.",
            record->author,
            record->title,
            record->report_number,
            record->institution,
            record->division,
            record->date
        );

        if (written < 0 ||
            (size_t)written >= output_size) {
            output[0] = '\0';
            return -1;
        }

        return 0;
    }

/*
 * Automatic formatter: thesis.
 */
if (strcmp(record->type, "THESIS") == 0) {
    int written;

    if (record->institution[0] != '\0' &&
        record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s, %s.",
            record->author,
            record->title,
            record->institution,
            record->date
        );

    } else if (record->institution[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s.",
            record->author,
            record->title,
            record->institution
        );

    } else if (record->date[0] != '\0') {

        written = snprintf(
            output,
            output_size,
            "%s. %s. %s.",
            record->author,
            record->title,
            record->date
        );

    } else {

        written = snprintf(
            output,
            output_size,
            "%s. %s.",
            record->author,
            record->title
        );
    }

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