
#ifndef WIKIPEDIA_H
#define WIKIPEDIA_H

#define WIKIPEDIA_MAX_LINKS 500
#define WIKIPEDIA_MAX_LINK_TITLE 256

#define WIKIPEDIA_MAX_SEARCH_RESULTS 50
#define WIKIPEDIA_MAX_SEARCH_TITLE 256
#define WIKIPEDIA_MAX_SEARCH_SNIPPET 512

typedef struct
{
    char title[WIKIPEDIA_MAX_LINK_TITLE];
} WikipediaLink;

typedef struct
{
    char title[WIKIPEDIA_MAX_SEARCH_TITLE];
    char snippet[WIKIPEDIA_MAX_SEARCH_SNIPPET];
} WikipediaSearchResult;

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

int wikipedia_search(
    const char *query,
    WikipediaSearchResult results[],
    int max_results);

#endif
