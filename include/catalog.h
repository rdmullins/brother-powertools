#ifndef CATALOG_H
#define CATALOG_H

#define CATALOG_MAX_AUTHOR 256
#define CATALOG_MAX_TITLE 512
#define CATALOG_MAX_PLACE 128
#define CATALOG_MAX_PUBLISHER 256
#define CATALOG_MAX_SUBJECTS 1024

typedef struct {
    char id[32];
    char author[CATALOG_MAX_AUTHOR];
    char title[CATALOG_MAX_TITLE];
    char place[CATALOG_MAX_PLACE];
    char publisher[CATALOG_MAX_PUBLISHER];
    char year[16];
    char subjects[CATALOG_MAX_SUBJECTS];
} CatalogRecord;

int catalog_load(const char *filename, CatalogRecord *record);

#endif