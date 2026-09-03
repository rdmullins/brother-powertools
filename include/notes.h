#include <stddef.h>

#ifndef NOTES_H
#define NOTES_H

#define NOTE_MAX_ID 32
#define NOTE_MAX_TITLE 256
#define NOTE_MAX_LOCATOR 128
#define NOTE_MAX_USE 512
#define NOTE_MAX_TEXT 4096

typedef struct {
    char id[NOTE_MAX_ID];
    char bib_id[NOTE_MAX_ID];
    char title[NOTE_MAX_TITLE];
    char locator[NOTE_MAX_LOCATOR];
    char use[NOTE_MAX_USE];
    char text[NOTE_MAX_TEXT];
} NoteRecord;

int notes_load(const char *filename,
               const char *wanted_id,
               NoteRecord *record);

int notes_next_id(const char *filename,
                  char *id,
                  size_t id_size);

int notes_save(const char *filename,
               const NoteRecord *record);

int notes_add_for_bibliography(const char *bib_id);

void notes_menu(void);

int notes_search_by_bibliography(const char *filename,
                                 const char *bib_id,
                                 NoteRecord *results,
                                 size_t max_results);
                                 

#endif