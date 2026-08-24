#ifndef BIBLIOGRAPHY_H
#define BIBLIOGRAPHY_H

#define BIB_MAX_ID 32
#define BIB_MAX_TYPE 32
#define BIB_MAX_AUTHOR 256
#define BIB_MAX_TITLE 512
#define BIB_MAX_JOURNAL 256
#define BIB_MAX_PLACE 128
#define BIB_MAX_PUBLISHER 256
#define BIB_MAX_INSTITUTION 256
#define BIB_MAX_VOLUME 32
#define BIB_MAX_ISSUE 32
#define BIB_MAX_DATE 64
#define BIB_MAX_PAGES 64
#define BIB_MAX_SOURCE 128
#define BIB_MAX_LOCATION 256
#define BIB_MAX_CITATION 2048
#define BIB_MAX_REPORT_NUMBER 64
#define BIB_MAX_DIVISION 256

typedef struct {
    char id[BIB_MAX_ID];
    char type[BIB_MAX_TYPE];

    char author[BIB_MAX_AUTHOR];
    char title[BIB_MAX_TITLE];

    char journal[BIB_MAX_JOURNAL];
    char place[BIB_MAX_PLACE];
    char publisher[BIB_MAX_PUBLISHER];
    char institution[BIB_MAX_INSTITUTION];

    char volume[BIB_MAX_VOLUME];
    char issue[BIB_MAX_ISSUE];
    char date[BIB_MAX_DATE];
    char pages[BIB_MAX_PAGES];

    char source[BIB_MAX_SOURCE];
    char location[BIB_MAX_LOCATION];
    char report_number[BIB_MAX_REPORT_NUMBER];
    char division[BIB_MAX_DIVISION];
    char citation[BIB_MAX_CITATION];
} BibliographyRecord;

int bibliography_load(const char *filename,
                      const char *wanted_id,
                      BibliographyRecord *record);

#endif