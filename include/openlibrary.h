#ifndef OPENLIBRARY_H
#define OPENLIBRARY_H

#define OPENLIBRARY_MAX_AUTHOR 256
#define OPENLIBRARY_MAX_TITLE 512
#define OPENLIBRARY_MAX_PUBLISHER 256
#define OPENLIBRARY_MAX_SUBJECTS 100
#define OPENLIBRARY_MAX_SUBJECT_LENGTH 256
#define OPENLIBRARY_MAX_ISBN 32
#define OPENLIBRARY_MAX_DATE 64

typedef struct {
    char isbn[OPENLIBRARY_MAX_ISBN];
    char author[OPENLIBRARY_MAX_AUTHOR];
    char title[OPENLIBRARY_MAX_TITLE];
    char publisher[OPENLIBRARY_MAX_PUBLISHER];
    char publish_date[OPENLIBRARY_MAX_DATE];

    int subject_count;
    char subjects[OPENLIBRARY_MAX_SUBJECTS]
                  [OPENLIBRARY_MAX_SUBJECT_LENGTH];
} OpenLibraryBook;

int openlibrary_lookup_isbn(const char *isbn,
                            OpenLibraryBook *book);

#endif