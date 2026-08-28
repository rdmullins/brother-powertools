#include <stdio.h>
#include <string.h>

#include "catalog.h"

int main(void)
{
    CatalogRecord record;
    CatalogRecord loaded;
    char id[32];
    const char *test_file = "/tmp/test-catalog.db";

    remove(test_file);

    memset(&record, 0, sizeof(record));

    strcpy(record.id, "000001");
    strcpy(record.isbn, "9780451532244");
    strcpy(record.author, "Mary Shelley");
    strcpy(record.title, "Frankenstein");
    strcpy(record.place, "New York");
    strcpy(record.publisher, "Signet");
    strcpy(record.year, "Oct 01, 2013");
    strcpy(record.subjects, "Gothic fiction;Horror");
    strcpy(record.location, "Office Shelf");

    if (catalog_save(test_file, &record) != 0) {
        fprintf(stderr, "catalog_save failed.\n");
        return 1;
    }

    if (catalog_next_id(test_file, id, sizeof(id)) != 0) {
        fprintf(stderr, "catalog_next_id failed.\n");
        return 1;
    }

    if (strcmp(id, "000002") != 0) {
        fprintf(stderr,
                "Expected next ID 000002, got %s\n",
                id);
        return 1;
    }

    memset(&loaded, 0, sizeof(loaded));

    if (catalog_load(test_file,
                     "000001",
                     &loaded) != 0) {
        fprintf(stderr, "catalog_load failed.\n");
        return 1;
    }

    if (strcmp(loaded.isbn, record.isbn) != 0 ||
        strcmp(loaded.author, record.author) != 0 ||
        strcmp(loaded.title, record.title) != 0 ||
        strcmp(loaded.publisher, record.publisher) != 0 ||
        strcmp(loaded.year, record.year) != 0 ||
        strcmp(loaded.subjects, record.subjects) != 0 ||
        strcmp(loaded.location, record.location) != 0) {
        fprintf(stderr, "Catalog round-trip verification failed.\n");
        return 1;
    }

    remove(test_file);

    printf("Catalog persistence test passed.\n");

    return 0;
}