#include <stdio.h>

#include "openlibrary.h"

int main(void)
{
    OpenLibraryBook book;

    if (openlibrary_lookup_isbn("9780451532244",
                                &book) != 0) {
        fprintf(stderr,
                "Open Library lookup failed.\n");
        return 1;
    }

    printf("Open Library lookup succeeded.\n");
    printf("\n");
    printf("ISBN:      %s\n", book.isbn);
    printf("Author:    %s\n", book.author);
    printf("Title:     %s\n", book.title);
    printf("Publisher: %s\n", book.publisher);
    printf("Date:      %s\n", book.publish_date);
    printf("Subjects:  %d found\n", book.subject_count);

    return 0;
}