#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "viewer.h"

#define VIEWER_WIDTH 80
#define VIEWER_LINES 17
#define VIEWER_LINE_SIZE 4096
#define VIEWER_SEARCH_SIZE 256

typedef struct {
    char line[VIEWER_LINE_SIZE];
    size_t offset;
    int valid;
} viewer_position_t;

static struct termios original_terminal;
static int terminal_changed = 0;


/*
 * Put the terminal into character-at-a-time mode.
 *
 * This lets the viewer respond immediately to SPACE, /, and Q
 * without requiring the user to press ENTER.
 */
static int enable_raw_mode(void)
{
    struct termios raw;

    if (!isatty(STDIN_FILENO))
        return -1;

    if (tcgetattr(STDIN_FILENO, &original_terminal) != 0)
        return -1;

    raw = original_terminal;

    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO,
                  TCSAFLUSH,
                  &raw) != 0)
        return -1;

    terminal_changed = 1;

    return 0;
}


/*
 * Restore normal terminal behavior.
 */
static void disable_raw_mode(void)
{
    if (!terminal_changed)
        return;

    tcsetattr(STDIN_FILENO,
              TCSAFLUSH,
              &original_terminal);

    terminal_changed = 0;
}


/*
 * Read a single key.
 */
static int read_key(void)
{
    unsigned char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return EOF;

    return c;
}


/*
 * Case-insensitive substring search.
 *
 * Returns 1 if needle occurs in haystack.
 */
static int contains_ignore_case(const char *haystack,
                                const char *needle)
{
    size_t needle_length;
    size_t i;
    size_t j;

    if (needle[0] == '\0')
        return 1;

    needle_length = strlen(needle);

    for (i = 0; haystack[i] != '\0'; i++)
    {
        for (j = 0;
             j < needle_length &&
             haystack[i + j] != '\0';
             j++)
        {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j]))
            {
                break;
            }
        }

        if (j == needle_length)
            return 1;
    }

    return 0;
}


/*
 * Read a search string.
 *
 * Raw terminal mode is temporarily disabled so that the user
 * can type normally and edit the search term with the terminal's
 * standard line discipline.
 */
static int read_search_term(char *search,
                            size_t search_size)
{
    disable_raw_mode();

    printf("\nSearch for: ");
    fflush(stdout);

    if (fgets(search, search_size, stdin) == NULL)
    {
        if (enable_raw_mode() != 0)
            return -1;

        return -1;
    }

    search[strcspn(search, "\n")] = '\0';

    if (enable_raw_mode() != 0)
        return -1;

    return 0;
}


/*
 * Display a small status line.
 */
static void show_status(void)
{
    printf("\n");
    printf("SPACE = next page   / = search   Q = quit");
    printf("\n");
}


/*
 * Display a page of text.
 *
 * Returns:
 *   1  if text was displayed
 *   0  if end of file was reached
 *  -1  on error
 */
static int display_page(FILE *file,
                        char *search_term,
                        viewer_position_t *position)
{
    int screen_lines = 0;

    while (screen_lines < VIEWER_LINES)
    {
        size_t remaining;
        size_t display_length;
        size_t break_position;
        size_t i;

        /*
         * Get the next source line when we don't already have
         * an unfinished line from the previous page.
         */
        if (!position->valid)
        {
            if (fgets(position->line,
                       sizeof(position->line),
                       file) == NULL)
            {
                return screen_lines > 0 ? 1 : 0;
            }

            position->offset = 0;
            position->valid = 1;
        }

        remaining = strlen(position->line) - position->offset;

        /*
         * If the remaining text fits on one screen row, display
         * the whole thing.
         */
        if (remaining <= VIEWER_WIDTH)
        {
            fputs(position->line + position->offset, stdout);

            position->valid = 0;
            screen_lines++;

            continue;
        }

        /*
         * The line is longer than the screen.
         *
         * Look for the last whitespace character that fits within
         * the screen width. This lets us wrap at a word boundary.
         */
        break_position = VIEWER_WIDTH;

        for (i = VIEWER_WIDTH; i > 0; i--)
        {
            if (isspace((unsigned char)
                        position->line[position->offset + i - 1]))
            {
                break_position = i - 1;
                break;
            }
        }

        /*
         * If there was no useful whitespace, the word itself is
         * longer than the screen. In that case, hard-wrap it.
         */
        if (break_position == 0)
        {
            break_position = VIEWER_WIDTH;
        }

        display_length = break_position;

        fwrite(position->line + position->offset,
                1,
                display_length,
                stdout);

        fputc('\n', stdout);

        position->offset += display_length;

        /*
         * Skip whitespace at the beginning of the next screen row.
         * This prevents the wrapped line from beginning with a
         * bunch of spaces.
         */
        while (position->line[position->offset] != '\0' &&
               position->line[position->offset] != '\n' &&
               isspace((unsigned char)
                       position->line[position->offset]))
        {
            position->offset++;
        }

        /*
         * If we've reached the end of the source line, we're done
         * with it.
         */
        if (position->line[position->offset] == '\0' ||
            position->line[position->offset] == '\n')
        {
            position->valid = 0;
        }

        screen_lines++;
    }

    (void)search_term;

    return 1;
}


/*
 * Search forward from the current file position.
 *
 * If a match is found, rewind to the beginning of the matching line
 * so that the next displayed page begins there.
 */
static int search_forward(FILE *file,
                          const char *search_term)
{
    char line[VIEWER_LINE_SIZE];

    while (1)
    {
        long position = ftell(file);

        if (fgets(line, sizeof(line), file) == NULL)
            return 0;

        if (contains_ignore_case(line, search_term))
        {
            if (fseek(file, position, SEEK_SET) != 0)
                return -1;

            return 1;
        }
    }
}


/*
 * View a plain-text file.
 */
int view_text_file(const char *filename)
{
    viewer_position_t position = {
    .offset = 0,
    .valid = 0
};
    FILE *file;
    char search_term[VIEWER_SEARCH_SIZE] = "";
    int key;
    int page_result;

    file = fopen(filename, "r");

    if (file == NULL)
    {
        perror("Unable to open text file");
        return -1;
    }

    if (enable_raw_mode() != 0)
    {
        fclose(file);
        printf("Unable to configure terminal.\n");
        return -1;
    }

    while (1)
    {
    page_result = display_page(file,
                            search_term,
                            &position);

        if (page_result < 0)
        {
            break;
        }

        if (page_result == 0)
        {
            printf("\n");
            printf("--- End of document ---\n");
            printf("Press any key to return.\n");

            key = read_key();
            (void)key;

            break;
        }

        show_status();

        key = read_key();

        if (key == EOF)
            break;

        switch (tolower((unsigned char)key))
        {
            case 'q':
                disable_raw_mode();
                fclose(file);
                return 0;

            case ' ':
                /*
                 * Continue to the next page.
                 */
                break;

            case '/':
            {
                char new_search[VIEWER_SEARCH_SIZE];

                if (read_search_term(new_search,
                                     sizeof(new_search)) != 0)
                {
                    break;
                }

                if (new_search[0] == '\0')
                {
                    break;
                }

                snprintf(search_term,
                        sizeof(search_term),
                        "%s",
                        new_search);

                /*
                * A new search starts from the beginning of the document.
                */
                if (fseek(file, 0, SEEK_SET) != 0)
                {
                    printf("\nUnable to restart search.\n");
                    break;
                }

                position.valid = 0;
                position.offset = 0;

                if (search_forward(file, search_term) == 0)
                {
                    printf("\n");
                    printf("Not found: %s\n", search_term);
                    printf("Press any key to continue.\n");

                    key = read_key();
                }

                break;
            }

            default:
                /*
                 * Ignore other keys.
                 */
                break;
        }
    }

    disable_raw_mode();
    fclose(file);

    return 0;
}