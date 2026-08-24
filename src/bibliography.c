#include <stdio.h>
#include <string.h>

#include "bibliography.h"

static void copy_field(char *destination,
                       size_t destination_size,
                       const char *value)
{
    if (destination_size == 0) {
        return;
    }

    strncpy(destination, value, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

int bibliography_load(const char *filename,
                      const char *wanted_id,
                      BibliographyRecord *record)
{
    FILE *file;
    char line[4096];
    BibliographyRecord current;
    int in_record = 0;

    if (filename == NULL ||
        wanted_id == NULL ||
        record == NULL) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open bibliography");
        return -1;
    }

    memset(&current, 0, sizeof(current));

    while (fgets(line, sizeof(line), file) != NULL) {
        char *equals;
        char *key;
        char *value;

        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "---") == 0) {
            if (in_record &&
                strcmp(current.id, wanted_id) == 0) {

                *record = current;
                fclose(file);
                return 0;
            }

            memset(&current, 0, sizeof(current));
            in_record = 0;
            continue;
        }

        equals = strchr(line, '=');

        if (equals == NULL) {
            continue;
        }

        *equals = '\0';

        key = line;
        value = equals + 1;

        in_record = 1;

        if (strcmp(key, "ID") == 0) {
            copy_field(current.id,
                       sizeof(current.id),
                       value);

        } else if (strcmp(key, "TYPE") == 0) {
            copy_field(current.type,
                       sizeof(current.type),
                       value);

        } else if (strcmp(key, "AUTHOR") == 0) {
            copy_field(current.author,
                       sizeof(current.author),
                       value);

        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(current.title,
                       sizeof(current.title),
                       value);

        } else if (strcmp(key, "JOURNAL") == 0) {
            copy_field(current.journal,
                       sizeof(current.journal),
                       value);

        } else if (strcmp(key, "PLACE") == 0) {
            copy_field(current.place,
                       sizeof(current.place),
                       value);

        } else if (strcmp(key, "PUBLISHER") == 0) {
            copy_field(current.publisher,
                       sizeof(current.publisher),
                       value);

        } else if (strcmp(key, "INSTITUTION") == 0) {
            copy_field(current.institution,
                       sizeof(current.institution),
                       value);

        } else if (strcmp(key, "REPORT_NUMBER") == 0) {
            copy_field(current.report_number,
                       sizeof(current.report_number),
                       value);

        } else if (strcmp(key, "DIVISION") == 0) {
            copy_field(current.division,
                       sizeof(current.division),
                       value);

        } else if (strcmp(key, "VOLUME") == 0) {
            copy_field(current.volume,
                       sizeof(current.volume),
                       value);

        } else if (strcmp(key, "ISSUE") == 0) {
            copy_field(current.issue,
                       sizeof(current.issue),
                       value);

        } else if (strcmp(key, "DATE") == 0) {
            copy_field(current.date,
                       sizeof(current.date),
                       value);

        } else if (strcmp(key, "PAGES") == 0) {
            copy_field(current.pages,
                       sizeof(current.pages),
                       value);

        } else if (strcmp(key, "SOURCE") == 0) {
            copy_field(current.source,
                       sizeof(current.source),
                       value);

        } else if (strcmp(key, "LOCATION") == 0) {
            copy_field(current.location,
                       sizeof(current.location),
                       value);

        } else if (strcmp(key, "CITATION") == 0) {
            copy_field(current.citation,
                       sizeof(current.citation),
                       value);
        }
    }

    /*
     * Handle a final record even if the file does not end
     * with a --- separator.
     */
    if (in_record &&
        strcmp(current.id, wanted_id) == 0) {

        *record = current;
        fclose(file);
        return 0;
    }

    fclose(file);

    return 1;
}