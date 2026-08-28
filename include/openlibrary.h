#ifndef OPENLIBRARY_H
#define OPENLIBRARY_H

#include "catalog.h"

int openlibrary_lookup_isbn(const char *isbn,
                            CatalogRecord *record);

#endif