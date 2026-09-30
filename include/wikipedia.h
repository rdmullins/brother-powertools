#ifndef WIKIPEDIA_H
#define WIKIPEDIA_H

void wikipedia_lookup(void);
int wikipedia_clean(const char *input, const char *output);
int wikipedia_fetch_article(const char *article);

#endif