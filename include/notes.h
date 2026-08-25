#ifndef NOTES_H
#define NOTES_H

#define NOTE_MAX_ID 32
#define NOTE_MAX_SOURCE 32
#define NOTE_MAX_TITLE 256
#define NOTE_MAX_LOCATOR 128
#define NOTE_MAX_USE 512
#define NOTE_MAX_TEXT 4096

typedef struct {
    char id[NOTE_MAX_ID];
    char source[NOTE_MAX_SOURCE];
    char title[NOTE_MAX_TITLE];
    char locator[NOTE_MAX_LOCATOR];
    char use[NOTE_MAX_USE];
    char text[NOTE_MAX_TEXT];
} NoteRecord;

int notes_load(const char *filename,
               const char *wanted_id,
               NoteRecord *record);

#endif