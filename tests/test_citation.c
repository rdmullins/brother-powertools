#include <stdio.h>
#include <string.h>

#include "citation.h"

static int test_article(void)
{
    BibliographyRecord record;
    char output[CITATION_MAX_LENGTH];

    memset(&record, 0, sizeof(record));

    strcpy(record.type, "ARTICLE");
    strcpy(record.author, "Boyer, Paul");
    strcpy(record.title,
           "From Activism to Apathy: The American People and Nuclear Weapons, 1963-1980");
    strcpy(record.journal, "The Journal of American History");
    strcpy(record.volume, "70");
    strcpy(record.issue, "4");
    strcpy(record.date, "March 1984");
    strcpy(record.pages, "821-844");

    if (citation_generate(&record,
                          output,
                          sizeof(output)) != 0) {
        fprintf(stderr, "ARTICLE citation generation failed.\n");
        return 1;
    }

    if (strcmp(output,
               "Boyer, Paul. \"From Activism to Apathy: The American People and Nuclear Weapons, 1963-1980.\" The Journal of American History 70, no. 4 (March 1984): 821-844.") != 0) {
        fprintf(stderr, "ARTICLE citation is incorrect.\n");
        fprintf(stderr, "Got: %s\n", output);
        return 1;
    }

    return 0;
}

static int test_explicit_citation(void)
{
    BibliographyRecord record;
    char output[CITATION_MAX_LENGTH];

    memset(&record, 0, sizeof(record));

    strcpy(record.type, "REPORT");
    strcpy(record.citation,
           "Jordan, Nehemiah. U.S. Civil Defense Before 1950.");

    if (citation_generate(&record,
                          output,
                          sizeof(output)) != 0) {
        fprintf(stderr, "Explicit citation generation failed.\n");
        return 1;
    }

    if (strcmp(output, record.citation) != 0) {
        fprintf(stderr, "Explicit citation was not preserved.\n");
        return 1;
    }

    return 0;
}

static int test_report(void)
{
    BibliographyRecord record;
    char output[CITATION_MAX_LENGTH];

    memset(&record, 0, sizeof(record));

    strcpy(record.type, "REPORT");
    strcpy(record.author, "Jordan, Nehemiah");
    strcpy(record.title,
           "U.S. Civil Defense Before 1950: The Roots of Public Law 920");
    strcpy(record.institution, "Institute for Defense Analyses");
    strcpy(record.report_number, "Study S-212");
    strcpy(record.division,
           "Economic and Political Studies Division");
    strcpy(record.date, "May 1966");

    if (citation_generate(&record,
                          output,
                          sizeof(output)) != 0) {
        fprintf(stderr, "REPORT citation generation failed.\n");
        return 1;
    }

    if (strcmp(output,
               "Jordan, Nehemiah. U.S. Civil Defense Before 1950: The Roots of Public Law 920. Study S-212. Institute for Defense Analyses, Economic and Political Studies Division, May 1966.") != 0) {
        fprintf(stderr, "REPORT citation is incorrect.\n");
        fprintf(stderr, "Got: %s\n", output);
        return 1;
    }

    return 0;
}

static int test_unknown_type(void)
{
    BibliographyRecord record;
    char output[CITATION_MAX_LENGTH];

    memset(&record, 0, sizeof(record));

    strcpy(record.type, "UNKNOWN");

    if (citation_generate(&record,
                          output,
                          sizeof(output)) != 1) {
        fprintf(stderr, "Unknown type should return 1.\n");
        return 1;
    }

    return 0;
}

static int test_small_buffer(void)
{
    BibliographyRecord record;
    char output[16];

    memset(&record, 0, sizeof(record));

    strcpy(record.type, "ARTICLE");
    strcpy(record.author, "Boyer, Paul");
    strcpy(record.title, "A Very Long Article Title");
    strcpy(record.journal, "The Journal of American History");
    strcpy(record.volume, "70");
    strcpy(record.issue, "4");
    strcpy(record.date, "March 1984");
    strcpy(record.pages, "821-844");

    if (citation_generate(&record,
                          output,
                          sizeof(output)) != -1) {
        fprintf(stderr, "Small buffer should return -1.\n");
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_article() != 0)
        return 1;

    if (test_report() != 0)
        return 1;

    if (test_explicit_citation() != 0)
        return 1;

    if (test_unknown_type() != 0)
        return 1;

    if (test_small_buffer() != 0)
        return 1;

    printf("Citation tests passed.\n");

    return 0;
}