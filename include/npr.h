#ifndef NPR_H
#define NPR_H

#define NPR_MAX_STORIES 20
#define NPR_MAX_TITLE 512
#define NPR_MAX_URL 256

typedef struct
{
    char title[NPR_MAX_TITLE];
    char url[NPR_MAX_URL];
} NPRStory;

void npr_lookup(void);

int npr_get_headlines(NPRStory stories[], int max_stories);
int npr_read_article(const NPRStory *story);
int npr_fetch_article(const NPRStory *story);

#endif