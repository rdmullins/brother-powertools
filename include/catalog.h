#include <stddef.h>

#ifndef CATALOG_H
#define CATALOG_H

#define CATALOG_MAX_AUTHOR 256
#define CATALOG_MAX_TITLE 512
#define CATALOG_MAX_PLACE 128
#define CATALOG_MAX_PUBLISHER 256
#define CATALOG_MAX_SUBJECTS 1024
#define CATALOG_MAX_ISBN 32
#define CATALOG_MAX_LOCATION 128

typedef struct {
    char id[32];
    char author[CATALOG_MAX_AUTHOR];
    char title[CATALOG_MAX_TITLE];
    char place[CATALOG_MAX_PLACE];
    char publisher[CATALOG_MAX_PUBLISHER];
    char year[16];
    char subjects[CATALOG_MAX_SUBJECTS];
    char isbn[CATALOG_MAX_ISBN];
    char location[CATALOG_MAX_LOCATION];
    
} CatalogRecord;

int catalog_load(const char *filename,
                 const char *wanted_id,
                 CatalogRecord *record);

void catalog_display(const CatalogRecord *record);

int catalog_save(const char *filename,
                 const CatalogRecord *record);

int catalog_next_id(const char *filename,
                    char *id,
                    size_t id_size);

#endif