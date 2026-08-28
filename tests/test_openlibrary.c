#include <stdio.h>

#include "openlibrary.h"

int main(void)
{
    CatalogRecord record;

    if (openlibrary_lookup_isbn("9780451532244",
                                &record) != 0) {
        fprintf(stderr,
                "Open Library lookup failed.\n");
        return 1;
    }

    printf("Open Library lookup succeeded.\n");
    printf("\n");
    printf("ISBN:      %s\n", record.isbn);
    printf("Author:    %s\n", record.author);
    printf("Title:     %s\n", record.title);
    printf("Publisher: %s\n", record.publisher);
    printf("Year:      %s\n", record.year);
    printf("Subjects:  %s\n", record.subjects);

    return 0;
}