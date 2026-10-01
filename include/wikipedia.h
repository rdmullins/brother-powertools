#ifndef WIKIPEDIA_H
#define WIKIPEDIA_H

#define WIKIPEDIA_MAX_LINKS 500
#define WIKIPEDIA_MAX_LINK_TITLE 256

typedef struct
{
    char title[WIKIPEDIA_MAX_LINK_TITLE];
} WikipediaLink;

void wikipedia_lookup(void);

int wikipedia_clean(
    const char *input,
    const char *output);

int wikipedia_fetch_article(
    const char *article);

int wikipedia_load_links(
    const char *article,
    WikipediaLink links[],
    int max_links);

#endif