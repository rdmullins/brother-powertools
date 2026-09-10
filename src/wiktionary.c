#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wiktionary.h"
#include "transfer.h"

#define JSON_FILE "/tmp/powertools-wiktionary.json"
#define TEXT_FILE "/tmp/powertools-wiktionary.txt"
#define MAX_JSON_SIZE 1024 * 1024

/*
 * Decode a JSON string beginning at the opening quote.
 *
 * Returns a pointer to the character after the closing quote.
 */
static const char *decode_json_string(const char *p, char *out, size_t out_size)
{
    size_t i = 0;

    if (*p != '"')
        return NULL;

    p++;

    while (*p && *p != '"' && i < out_size - 1)
    {
        if (*p == '\\')
        {
            p++;

            if (!*p)
                break;

            switch (*p)
            {
                case '"':
                    out[i++] = '"';
                    break;

                case '\\':
                    out[i++] = '\\';
                    break;

                case '/':
                    out[i++] = '/';
                    break;

                case 'b':
                    out[i++] = '\b';
                    break;

                case 'f':
                    out[i++] = '\f';
                    break;

                case 'n':
                    out[i++] = '\n';
                    break;

                case 'r':
                    out[i++] = '\r';
                    break;

                case 't':
                    out[i++] = '\t';
                    break;

                case 'u':
                    /*
                     * Wiktionary entries can contain Unicode escapes.
                     * For now, preserve them rather than attempting
                     * UTF-16 decoding.
                     */
                    if (p[1] && p[2] && p[3] && p[4])
                    {
                        out[i++] = '?';
                        p += 4;
                    }
                    break;

                default:
                    out[i++] = *p;
                    break;
            }

            p++;
        }
        else
        {
            out[i++] = *p++;
        }
    }

    out[i] = '\0';

    if (*p == '"')
        p++;

    return p;
}


/*
 * Strip the simple HTML markup used inside Wiktionary definitions.
 *
 * This is intentionally lightweight. We don't need a full HTML parser;
 * we're converting a short definition into plain text for the Brother.
 */
static void write_clean_definition(FILE *out, const char *definition)
{
    const char *p = definition;
    int in_tag = 0;
    int in_style = 0;
    int in_ol = 0;
    int last_was_space = 0;

    while (*p)
    {
        /*
         * Discard the entire contents of <style>...</style>.
         */
        if (!in_style && strncmp(p, "<style", 6) == 0)
        {
            in_style = 1;
            p += 6;
            continue;
        }

        if (in_style)
        {
            if (strncmp(p, "</style>", 8) == 0)
            {
                in_style = 0;
                p += 8;
            }
            else
            {
                p++;
            }

            continue;
        }

        /*
         * Ignore nested ordered lists.
         *
         * The Wiktionary API gives us the nested sense again as
         * its own definition object, so we don't want to print it
         * twice.
         */
        if (!in_ol && strncmp(p, "<ol", 3) == 0)
        {
            in_ol = 1;
            p += 3;
            continue;
        }

        if (in_ol)
        {
            if (strncmp(p, "</ol>", 5) == 0)
            {
                in_ol = 0;
                p += 5;
            }
            else
            {
                p++;
            }

            continue;
        }

        /*
         * HTML tag.
         */
        if (*p == '<')
        {
            in_tag = 1;
            p++;
            continue;
        }

        if (*p == '>')
        {
            in_tag = 0;
            p++;
            continue;
        }

        /*
         * Normal text.
         */
        if (!in_tag)
        {
            if (isspace((unsigned char)*p))
            {
                if (!last_was_space)
                {
                    fputc(' ', out);
                    last_was_space = 1;
                }
            }
            else
            {
                fputc(*p, out);
                last_was_space = 0;
            }
        }

        p++;
    }

    fputc('\n', out);
}


/*
 * Extract definitions from the Wiktionary JSON response.
 *
 * We deliberately walk the JSON sequentially instead of using strstr()
 * on arbitrary pointers. The response is small, and this keeps the parser
 * predictable and safe.
 */
static int json_extract_definitions(const char *json, FILE *out)
{
    const char *p;
    const char *english;
    const char *english_end;

    char key[64];
    char value[8192];
    char part_of_speech[128];

    int definitions = 0;

    part_of_speech[0] = '\0';

    /*
     * Find the English language section.
     *
     * The API response begins:
     *
     *     {"en":[...],"sv":[...]}
     *
     * We deliberately restrict ourselves to the English section.
     */
    english = strstr(json, "\"en\"");

    if (!english)
        return 0;

    /*
     * The next language entry marks the end of the English section.
     */
    english_end = strstr(english + 4, "],\"sv\"");

    if (!english_end)
        english_end = json + strlen(json);

    p = english + 4;

    while (p < english_end && *p)
    {
        /*
         * We only process JSON strings.
         */
        if (*p != '"')
        {
            p++;
            continue;
        }

        {
            const char *next =
                decode_json_string(p, key, sizeof(key));

            if (!next)
                break;

            p = next;
        }

        /*
         * Skip whitespace and the colon.
         */
        while (p < english_end &&
               isspace((unsigned char)*p))
        {
            p++;
        }

        if (p >= english_end || *p != ':')
            continue;

        p++;

        while (p < english_end &&
               isspace((unsigned char)*p))
        {
            p++;
        }

        /*
         * Part of speech.
         */
        if (strcmp(key, "partOfSpeech") == 0)
        {
            if (*p == '"')
            {
                const char *next =
                    decode_json_string(
                        p,
                        part_of_speech,
                        sizeof(part_of_speech));

                if (!next)
                    break;

                p = next;
            }

            continue;
        }

        /*
         * Definition.
         */
        if (strcmp(key, "definition") == 0)
        {
            if (*p == '"')
            {
                const char *next =
                    decode_json_string(
                        p,
                        value,
                        sizeof(value));

                if (!next)
                    break;

                /*
                 * Only print definitions belonging to a
                 * recognized part of speech.
                 */
                if (part_of_speech[0] != '\0')
                {
                    fprintf(out, "%s\n", part_of_speech);
                    write_clean_definition(out, value);
                    fputc('\n', out);

                    definitions++;
                }

                p = next;
            }

            continue;
        }
    }

    return definitions;
}




/*
 * Fetch and display a Wiktionary entry.
 */
void wiktionary_lookup(void)
{
    char word[256];
    char command[1024];
    FILE *fp;
    FILE *out;
    char *json;
    long size;
    size_t bytes_read;
    int definitions;

    size_t max_size;
    int parts;
    int choice;
    int current_part;
    char part_filename[512];

    printf("\n");
    printf("+---------------------------------------------+\n");
    printf("|                 Wiktionary                  |\n");
    printf("+---------------------------------------------+\n");
    printf("\n");

    printf("Enter word:\n");
    printf("> ");

    if (fgets(word, sizeof(word), stdin) == NULL)
        return;

    word[strcspn(word, "\n")] = '\0';

    if (word[0] == '\0')
        return;

    /*
     * Fetch the definition endpoint.
     */
    printf("\nFetching Wiktionary entry...\n");

    snprintf(command, sizeof(command),
             "curl -s -L "
             "'https://en.wiktionary.org/api/rest_v1/page/definition/%s' "
             "> " JSON_FILE,
             word);

    if (system(command) != 0)
    {
        printf("Error fetching Wiktionary entry.\n");
        return;
    }

    /*
     * Open the JSON response.
     */
    fp = fopen(JSON_FILE, "rb");

    if (fp == NULL)
    {
        printf("Unable to open Wiktionary response.\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= 0 || size >= MAX_JSON_SIZE)
    {
        fclose(fp);
        printf("Invalid Wiktionary response.\n");
        return;
    }

    json = malloc((size_t)size + 1);

    if (json == NULL)
    {
        fclose(fp);
        printf("Out of memory.\n");
        return;
    }

    bytes_read = fread(json, 1, (size_t)size, fp);
    fclose(fp);

    json[bytes_read] = '\0';

    /*
     * Create the plain-text entry.
     */
    out = fopen(TEXT_FILE, "w");

    if (out == NULL)
    {
        free(json);
        printf("Unable to create text output.\n");
        return;
    }

    /*
     * Put the defined word at the top.
     */
    fprintf(out, "%s\n\n", word);

    definitions = json_extract_definitions(json, out);

    /*
     * Add the source URL.
     */
    fprintf(out, "Source:\n");
    fprintf(out, "https://en.wiktionary.org/wiki/%s\n", word);

    fclose(out);
    free(json);

    if (definitions == 0)
    {
        printf("\nNo definitions found.\n");
        return;
    }

    printf("\nFound %d definition(s).\n", definitions);

    /*
     * Ask how much room is available on the PowerNote.
     */
    printf("\n");
    printf("How much free PowerNote memory is available?\n");
    printf("Enter maximum part size in bytes: ");

    if (scanf("%zu", &max_size) != 1)
    {
        int c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
            /* discard invalid input */
        }

        printf("Invalid size.\n");
        return;
    }

    getchar();

    if (max_size == 0)
    {
        printf("Size must be greater than zero.\n");
        return;
    }

    /*
     * Split the entry into Brother-sized pieces.
     */
    parts = split_file(
        TEXT_FILE,
        "/tmp/powertools-wiktionary",
        max_size
    );

    if (parts < 0)
    {
        printf("Unable to split Wiktionary entry.\n");
        return;
    }

    printf("\n");
    printf("The Wiktionary entry will be sent in %d part%s.\n",
           parts,
           parts == 1 ? "" : "s");

    /*
     * Send each part.
     */
    current_part = 1;

    while (current_part <= parts)
    {
        snprintf(part_filename,
                 sizeof(part_filename),
                 "/tmp/powertools-wiktionary_%02d.txt",
                 current_part);

        if (send_transfer_part(part_filename,
                               current_part,
                               parts) != 0)
        {
            return;
        }

        if (current_part == parts)
        {
            printf("\n");
            printf("All parts have been transferred.\n");
            break;
        }

        printf("\n");
        printf("+---------------------------------------------+\n");
        printf("|              brother PowerTools             |\n");
        printf("|             Wiktionary Transfer             |\n");
        printf("+---------------------------------------------+\n");
        printf("| Part %d of %d complete.                      |\n",
               current_part,
               parts);
        printf("|                                             |\n");
        printf("| 1. Send next part                          |\n");
        printf("| 2. Cancel transfer                         |\n");
        printf("+---------------------------------------------+\n");
        printf("\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            int c;

            while ((c = getchar()) != '\n' && c != EOF)
            {
                /* discard invalid input */
            }

            printf("Invalid input. Transfer cancelled.\n");
            return;
        }

        getchar();

        if (choice == 1)
        {
            current_part++;
        }
        else
        {
            printf("Transfer cancelled.\n");
            return;
        }
    }
}