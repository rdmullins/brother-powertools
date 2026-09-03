#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "notes.h"

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

int notes_load(const char *filename,
               const char *wanted_id,
               NoteRecord *record)
{
    FILE *file;
    char line[8192];
    NoteRecord current;
    int in_record = 0;

    if (filename == NULL ||
        wanted_id == NULL ||
        record == NULL) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Unable to open notes");
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

        } else if (strcmp(key, "BIB_ID") == 0) {
            copy_field(current.bib_id,
                       sizeof(current.bib_id),
                       value);

        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(current.title,
                       sizeof(current.title),
                       value);

        } else if (strcmp(key, "LOCATOR") == 0) {
            copy_field(current.locator,
                       sizeof(current.locator),
                       value);

        } else if (strcmp(key, "USE") == 0) {
            copy_field(current.use,
                       sizeof(current.use),
                       value);

        } else if (strcmp(key, "TEXT") == 0) {
            copy_field(current.text,
                       sizeof(current.text),
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

int notes_search_by_bibliography(const char *filename,
                                 const char *bib_id,
                                 NoteRecord *results,
                                 size_t max_results)
{
    FILE *file;
    char line[8192];
    NoteRecord current;
    int in_record = 0;
    int count = 0;

    if (filename == NULL ||
        bib_id == NULL ||
        results == NULL ||
        max_results == 0) {
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
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
                strcmp(current.bib_id, bib_id) == 0 &&
                (size_t)count < max_results) {

                results[count] = current;
                count++;
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

        } else if (strcmp(key, "BIB_ID") == 0) {
            copy_field(current.bib_id,
                       sizeof(current.bib_id),
                       value);

        } else if (strcmp(key, "TITLE") == 0) {
            copy_field(current.title,
                       sizeof(current.title),
                       value);

        } else if (strcmp(key, "LOCATOR") == 0) {
            copy_field(current.locator,
                       sizeof(current.locator),
                       value);

        } else if (strcmp(key, "USE") == 0) {
            copy_field(current.use,
                       sizeof(current.use),
                       value);

        } else if (strcmp(key, "TEXT") == 0) {
            copy_field(current.text,
                       sizeof(current.text),
                       value);
        }
    }

    /*
     * Handle a final record even if the file does not end
     * with a --- separator.
     */
    if (in_record &&
        strcmp(current.bib_id, bib_id) == 0 &&
        (size_t)count < max_results) {

        results[count] = current;
        count++;
    }

    fclose(file);

    return count;
}

int notes_next_id(const char *filename,
                  char *id,
                  size_t id_size)
{
    FILE *fp;
    char line[8192];
    unsigned long highest = 0;

    if (!filename || !id || id_size == 0)
        return -1;

    fp = fopen(filename, "r");

    if (!fp) {
        if (id_size < 9)
            return -1;

        snprintf(id, id_size, "NC%06lu", 1UL);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "ID=NC", 5) == 0) {
            unsigned long value = strtoul(line + 5, NULL, 10);

            if (value > highest)
                highest = value;
        }
    }

    fclose(fp);

    if (highest >= 999999)
        return -1;

    if (id_size < 9)
        return -1;

    snprintf(id, id_size, "NC%06lu", highest + 1);

    return 0;
}


int notes_save(const char *filename,
               const NoteRecord *record)
{
    FILE *fp;
    int last_char;

    if (!filename || !record)
        return -1;

    fp = fopen(filename, "a+");

    if (!fp)
        return -1;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    if (ftell(fp) > 0) {
        if (fseek(fp, -1, SEEK_END) != 0) {
            fclose(fp);
            return -1;
        }

        last_char = fgetc(fp);

        if (last_char != '\n') {
            fputc('\n', fp);
        }
    }

    fseek(fp, 0, SEEK_END);

    fprintf(fp, "ID=%s\n", record->id);
    fprintf(fp, "BIB_ID=%s\n", record->bib_id);
    fprintf(fp, "TITLE=%s\n", record->title);
    fprintf(fp, "LOCATOR=%s\n", record->locator);
    fprintf(fp, "USE=%s\n", record->use);
    fprintf(fp, "TEXT=%s\n", record->text);
    fprintf(fp, "---\n");

    fclose(fp);

    return 0;
}