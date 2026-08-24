#include <stdio.h>
#include <string.h>

#include "catalog.h"

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

int catalog_load(const char *filename, CatalogRecord *record)
{
    FILE *file;
    char line[2048];

    if (filename == NULL || record == NULL) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open catalog");
        return -1;
    }

    memset(record, 0, sizeof(*record));

    while (fgets(line, sizeof(line), file) != NULL) {
        char *equals;
        char *key;
        char *value;

        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "---") == 0) {
            break;
        }

        equals = strchr(line, '=');

        if (equals == NULL) {
            continue;
        }

        *equals = '\0';

        key = line;
        value = equals + 1;

        if (strcmp(key, "ID") == 0) {
            copy_field(record->id,
                       sizeof(record->id),
                       value);
        } else if (strcmp(key, "AUTHOR") == 0) {
            copy_field(record->author,
                       sizeof(record->author),
                       value);
        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(record->title,
                       sizeof(record->title),
                       value);
        } else if (strcmp(key, "PLACE") == 0) {
            copy_field(record->place,
                       sizeof(record->place),
                       value);
        } else if (strcmp(key, "PUBLISHER") == 0) {
            copy_field(record->publisher,
                       sizeof(record->publisher),
                       value);
        } else if (strcmp(key, "YEAR") == 0) {
            copy_field(record->year,
                       sizeof(record->year),
                       value);
        } else if (strcmp(key, "SUBJECTS") == 0) {
            copy_field(record->subjects,
                       sizeof(record->subjects),
                       value);
        }
    }

    fclose(file);

    return 0;
}