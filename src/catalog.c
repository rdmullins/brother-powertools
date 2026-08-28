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

int catalog_load(const char *filename,
                 const char *wanted_id,
                 CatalogRecord *record)
{
    FILE *file;
    char line[2048];
    CatalogRecord current;
    int in_record = 0;

    if (filename == NULL ||
        wanted_id == NULL ||
        record == NULL) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open catalog");
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

        } else if (strcmp(key, "ISBN") == 0) {
            copy_field(current.isbn,
                       sizeof(current.isbn),
                       value);

        } else if (strcmp(key, "AUTHOR") == 0) {
            copy_field(current.author,
                       sizeof(current.author),
                       value);

        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(current.title,
                       sizeof(current.title),
                       value);

        } else if (strcmp(key, "PLACE") == 0) {
            copy_field(current.place,
                       sizeof(current.place),
                       value);

        } else if (strcmp(key, "PUBLISHER") == 0) {
            copy_field(current.publisher,
                       sizeof(current.publisher),
                       value);

        } else if (strcmp(key, "YEAR") == 0) {
            copy_field(current.year,
                       sizeof(current.year),
                       value);

        } else if (strcmp(key, "SUBJECTS") == 0) {
            copy_field(current.subjects,
                       sizeof(current.subjects),
                       value);
        } else if (strcmp(key, "LOCATION") == 0) {
            copy_field(current.location,
                       sizeof(current.location),
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

void catalog_display(const CatalogRecord *record)
{
    if (record == NULL) {
        return;
    }

    printf("\n");
    printf("ID:        %s\n", record->id);
    printf("ISBN:      %s\n", record->isbn);
    printf("Author:    %s\n", record->author);
    printf("Title:     %s\n", record->title);
    printf("Place:     %s\n", record->place);
    printf("Publisher: %s\n", record->publisher);
    printf("Year:      %s\n", record->year);
    printf("Subjects:  %s\n", record->subjects);
    printf("Location:  %s\n", record->location);
}

int catalog_save(const char *filename,
                 const CatalogRecord *record)
{
    FILE *file;

    if (filename == NULL || record == NULL) {
        return -1;
    }

    file = fopen(filename, "a");

    if (file == NULL) {
        perror("Unable to open catalog");
        return -1;
    }

    fprintf(file, "ID=%s\n", record->id);
    fprintf(file, "ISBN=%s\n", record->isbn);
    fprintf(file, "AUTHOR=%s\n", record->author);
    fprintf(file, "TITLE=%s\n", record->title);
    fprintf(file, "PLACE=%s\n", record->place);
    fprintf(file, "PUBLISHER=%s\n", record->publisher);
    fprintf(file, "YEAR=%s\n", record->year);
    fprintf(file, "SUBJECTS=%s\n", record->subjects);
    fprintf(file, "LOCATION=%s\n", record->location);
    fprintf(file, "---\n");

    fclose(file);

    return 0;
}

int catalog_next_id(const char *filename,
                    char *id,
                    size_t id_size)
{
    FILE *file;
    char line[2048];
    int highest_id = 0;

    if (filename == NULL ||
        id == NULL ||
        id_size == 0) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open catalog");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int current_id;

        if (sscanf(line, "ID=%d", &current_id) == 1) {
            if (current_id > highest_id) {
                highest_id = current_id;
            }
        }
    }

    fclose(file);

    snprintf(id,
             id_size,
             "%06d",
             highest_id + 1);

    return 0;
}