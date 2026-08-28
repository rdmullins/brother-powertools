#include <stdio.h>
#include <string.h>

#include "catalog.h"

static int expect_match(const char *query)
{
    CatalogRecord results[10];
    int count;

    count = catalog_search("data/catalog.db",
                           query,
                           results,
                           10);

    if (count < 1) {
        fprintf(stderr,
                "Expected a match for \"%s\".\n",
                query);
        return 1;
    }

    printf("PASS: \"%s\" found %d record(s).\n",
           query,
           count);

    return 0;
}

static int expect_no_match(const char *query)
{
    CatalogRecord results[10];
    int count;

    count = catalog_search("data/catalog.db",
                           query,
                           results,
                           10);

    if (count != 0) {
        fprintf(stderr,
                "Expected no match for \"%s\", found %d.\n",
                query,
                count);
        return 1;
    }

    printf("PASS: \"%s\" found nothing.\n",
           query);

    return 0;
}

int main(void)
{
    if (expect_match("Faragher") != 0) {
        return 1;
    }

    if (expect_match("overland") != 0) {
        return 1;
    }

    if (expect_match("9780451532244") != 0) {
        return 1;
    }

    if (expect_match("Office Shelf") != 0) {
        return 1;
    }

    if (expect_no_match("This Book Absolutely Does Not Exist") != 0) {
        return 1;
    }

    printf("Catalog search test passed.\n");

    return 0;
}