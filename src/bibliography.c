#include <stdio.h>
#include <string.h>
#include <strings.h>

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

        } else if (strcmp(key, "CATALOG_ID") == 0) {
            copy_field(current.catalog_id,
                    sizeof(current.catalog_id),
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

static int contains_case_insensitive(const char *text,
                                     const char *query)
{
    size_t query_length;

    if (text == NULL || query == NULL) {
        return 0;
    }

    query_length = strlen(query);

    if (query_length == 0) {
        return 1;
    }

    for (; *text != '\0'; text++) {
        if (strncasecmp(text, query, query_length) == 0) {
            return 1;
        }
    }

    return 0;
}

static int bibliography_record_matches(const BibliographyRecord *record,
                                        const char *query)
{
    return contains_case_insensitive(record->id, query) ||
           contains_case_insensitive(record->author, query) ||
           contains_case_insensitive(record->title, query) ||
           contains_case_insensitive(record->journal, query) ||
           contains_case_insensitive(record->institution, query) ||
           contains_case_insensitive(record->source, query) ||
           contains_case_insensitive(record->citation, query);
}

int bibliography_search(const char *filename,
                        const char *query,
                        BibliographyRecord *results,
                        size_t max_results)
{
    FILE *file;
    char line[4096];
    BibliographyRecord current;
    size_t result_count = 0;
    int in_record = 0;

    if (filename == NULL ||
        query == NULL ||
        results == NULL ||
        max_results == 0) {
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
                bibliography_record_matches(&current, query)) {

                if (result_count < max_results) {
                    results[result_count] = current;
                    result_count++;
                }
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
            copy_field(current.id, sizeof(current.id), value);
        } else if (strcmp(key, "TYPE") == 0) {
            copy_field(current.type, sizeof(current.type), value);
        } else if (strcmp(key, "AUTHOR") == 0) {
            copy_field(current.author, sizeof(current.author), value);
        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(current.title, sizeof(current.title), value);
        } else if (strcmp(key, "JOURNAL") == 0) {
            copy_field(current.journal, sizeof(current.journal), value);
            } else if (strcmp(key, "CATALOG_ID") == 0) {
    copy_field(current.catalog_id,
               sizeof(current.catalog_id),
               value);
        } else if (strcmp(key, "PLACE") == 0) {
            copy_field(current.place, sizeof(current.place), value);
        } else if (strcmp(key, "PUBLISHER") == 0) {
            copy_field(current.publisher, sizeof(current.publisher), value);
        } else if (strcmp(key, "INSTITUTION") == 0) {
            copy_field(current.institution, sizeof(current.institution), value);
        } else if (strcmp(key, "REPORT_NUMBER") == 0) {
            copy_field(current.report_number,
                       sizeof(current.report_number),
                       value);
        } else if (strcmp(key, "DIVISION") == 0) {
            copy_field(current.division, sizeof(current.division), value);
        } else if (strcmp(key, "VOLUME") == 0) {
            copy_field(current.volume, sizeof(current.volume), value);
        } else if (strcmp(key, "ISSUE") == 0) {
            copy_field(current.issue, sizeof(current.issue), value);
        } else if (strcmp(key, "DATE") == 0) {
            copy_field(current.date, sizeof(current.date), value);
        } else if (strcmp(key, "PAGES") == 0) {
            copy_field(current.pages, sizeof(current.pages), value);
        } else if (strcmp(key, "SOURCE") == 0) {
            copy_field(current.source, sizeof(current.source), value);
        } else if (strcmp(key, "LOCATION") == 0) {
            copy_field(current.location, sizeof(current.location), value);
        } else if (strcmp(key, "CITATION") == 0) {
            copy_field(current.citation, sizeof(current.citation), value);
        }
    }

    if (in_record &&
        result_count < max_results &&
        bibliography_record_matches(&current, query)) {

        results[result_count] = current;
        result_count++;
    }

    fclose(file);

    return (int)result_count;
}

void bibliography_display(const BibliographyRecord *record)
{
    if (record == NULL) {
        return;
    }

    printf("\n");
    printf("ID:          %s\n", record->id);
    printf("Type:        %s\n", record->type);
    printf("Author:      %s\n", record->author);
    printf("Title:       %s\n", record->title);

    if (record->journal[0] != '\0') {
        printf("Journal:     %s\n", record->journal);
    }

    if (record->volume[0] != '\0') {
        printf("Volume:      %s\n", record->volume);
    }

    if (record->issue[0] != '\0') {
        printf("Issue:       %s\n", record->issue);
    }

    if (record->date[0] != '\0') {
        printf("Date:        %s\n", record->date);
    }

    if (record->pages[0] != '\0') {
        printf("Pages:       %s\n", record->pages);
    }

    if (record->institution[0] != '\0') {
        printf("Institution: %s\n", record->institution);
    }

    if (record->report_number[0] != '\0') {
        printf("Report No.:  %s\n", record->report_number);
    }

    if (record->division[0] != '\0') {
        printf("Division:    %s\n", record->division);
    }

    if (record->publisher[0] != '\0') {
        printf("Publisher:   %s\n", record->publisher);
    }

    if (record->place[0] != '\0') {
        printf("Place:       %s\n", record->place);
    }

    if (record->source[0] != '\0') {
        printf("Source:      %s\n", record->source);
    }

    if (record->location[0] != '\0') {
        printf("Location:    %s\n", record->location);
    }

    if (record->citation[0] != '\0') {
        printf("Citation:    %s\n", record->citation);
    }
}

int bibliography_save(const char *filename,
                      const BibliographyRecord *record)
{
    FILE *file;

    if (filename == NULL || record == NULL) {
        return -1;
    }

    file = fopen(filename, "a");

    if (file == NULL) {
        perror("Unable to open bibliography");
        return -1;
    }

    fprintf(file, "ID=%s\n", record->id);
    fprintf(file, "CATALOG_ID=%s\n", record->catalog_id);
    fprintf(file, "TYPE=%s\n", record->type);
    fprintf(file, "AUTHOR=%s\n", record->author);
    fprintf(file, "TITLE=%s\n", record->title);
    fprintf(file, "JOURNAL=%s\n", record->journal);
    fprintf(file, "PLACE=%s\n", record->place);
    fprintf(file, "PUBLISHER=%s\n", record->publisher);
    fprintf(file, "INSTITUTION=%s\n", record->institution);
    fprintf(file, "REPORT_NUMBER=%s\n", record->report_number);
    fprintf(file, "DIVISION=%s\n", record->division);
    fprintf(file, "VOLUME=%s\n", record->volume);
    fprintf(file, "ISSUE=%s\n", record->issue);
    fprintf(file, "DATE=%s\n", record->date);
    fprintf(file, "PAGES=%s\n", record->pages);
    fprintf(file, "SOURCE=%s\n", record->source);
    fprintf(file, "LOCATION=%s\n", record->location);
    fprintf(file, "CITATION=%s\n", record->citation);
    fprintf(file, "---\n");

    fclose(file);

    return 0;
}

int bibliography_next_id(const char *filename,
                         char *id,
                         size_t id_size)
{
    FILE *file;
    char line[4096];
    int highest_id = 0;

    if (filename == NULL ||
        id == NULL ||
        id_size == 0) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open bibliography");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int current_id;

        if (sscanf(line, "ID=BC%d", &current_id) == 1) {
            if (current_id > highest_id) {
                highest_id = current_id;
            }
        }
    }

    fclose(file);

    snprintf(id,
             id_size,
             "BC%06d",
             highest_id + 1);

    return 0;
}

int bibliography_exists_for_catalog(const char *filename,
                                    const char *catalog_id)
{
    FILE *file;
    char line[4096];
    BibliographyRecord current;
    int in_record = 0;

    if (filename == NULL || catalog_id == NULL) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        /*
         * An absent bibliography database means that no
         * bibliography records exist yet.
         */
        return 0;
    }

    memset(&current, 0, sizeof(current));

    while (fgets(line, sizeof(line), file) != NULL) {
        char *equals;
        char *key;
        char *value;

        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "---") == 0) {
            if (in_record &&
                strcmp(current.catalog_id, catalog_id) == 0) {

                fclose(file);
                return 1;
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

        if (strcmp(key, "CATALOG_ID") == 0) {
            copy_field(current.catalog_id,
                       sizeof(current.catalog_id),
                       value);
        }
    }

    if (in_record &&
        strcmp(current.catalog_id, catalog_id) == 0) {

        fclose(file);
        return 1;
    }

    fclose(file);

    return 0;
}