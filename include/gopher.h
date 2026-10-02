#ifndef GOPHER_H
#define GOPHER_H

#define GOPHER_MAX_ENTRIES 100
#define GOPHER_MAX_DISPLAY 256
#define GOPHER_MAX_SELECTOR 512
#define GOPHER_MAX_HOST 256

typedef struct
{
    char type;
    char display[GOPHER_MAX_DISPLAY];
    char selector[GOPHER_MAX_SELECTOR];
    char host[GOPHER_MAX_HOST];
    int port;
} GopherEntry;

typedef struct
{
    char host[GOPHER_MAX_HOST];
    char selector[GOPHER_MAX_SELECTOR];
    int port;
} GopherLocation;

void gopher_menu(void);

char *gopher_fetch(
    const char *host,
    int port,
    const char *selector);

int gopher_parse_directory(
    char *response,
    GopherEntry entries[],
    int max_entries);

#endif