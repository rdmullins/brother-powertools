#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "npr.h"
#include "viewer.h"

#define NPR_HOME_URL "https://text.npr.org/"
#define NPR_BASE_URL "https://text.npr.org"

#define NPR_HOME_FILE "/tmp/powertools-npr-home.html"
#define NPR_TEXT_FILE "/tmp/powertools-npr.txt"

#define NPR_HEADLINE_WIDTH 74

#define MAX_STORIES 20
#define MAX_TITLE 512
#define MAX_URL 256
#define MAX_HTML_SIZE (1024 * 1024 * 4)
#define NPR_STORIES_PER_PAGE 15

typedef struct
{
    char title[MAX_TITLE];
    char url[256];
} NPRStory;



/*
 * Fetch a URL into a local file.
 */
static int fetch_url(const char *url, const char *filename)
{
    char command[1024];
    

    snprintf(command,
             sizeof(command),
             "curl -s -L --fail '%s' > '%s'",
             url,
             filename);

    return system(command);
}


/*
 * Read an entire file into memory.
 */
static char *read_file(const char *filename, long *size_out)
{
    FILE *fp;
    char *data;
    long size;
    size_t bytes_read;

    fp = fopen(filename, "rb");

    if (fp == NULL)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return NULL;
    }

    size = ftell(fp);

    if (size <= 0 || size > MAX_HTML_SIZE)
    {
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    data = malloc((size_t)size + 1);

    if (data == NULL)
    {
        fclose(fp);
        return NULL;
    }

    bytes_read = fread(data, 1, (size_t)size, fp);
    fclose(fp);

    data[bytes_read] = '\0';

    if (size_out != NULL)
        *size_out = (long)bytes_read;

    return data;
}

/*
 * Print a headline truncated to the Brother's 80-column width.
 *
 * The title gets NPR_HEADLINE_WIDTH characters, including the
 * ellipsis when truncation is necessary.
 *
 * This works on UTF-8 text without splitting a multi-byte
 * character in the middle.
 */
static void print_headline(const char *title)
{
    size_t len = strlen(title);

    if (len <= NPR_HEADLINE_WIDTH)
    {
        printf("%s", title);
        return;
    }

    /*
     * Reserve three characters for "...".
     */
    size_t limit = NPR_HEADLINE_WIDTH - 3;

    /*
     * Walk backward if we landed in the middle of a UTF-8
     * continuation byte (10xxxxxx).
     */
    while (limit > 0 &&
           ((unsigned char)title[limit] & 0xC0) == 0x80)
    {
        limit--;
    }

    printf("%.*s...", (int)limit, title);
}

/*
 * Extract NPR's current headline list.
 *
 * NPR's text site uses:
 *
 * <li><a class="topic-title" href="/...">Headline</a></li>
 */
static int parse_headlines(const char *html,
                           NPRStory stories[],
                           int max_stories)
{
    const char *p;
    int count = 0;

    p = html;

    while (count < max_stories)
    {
        const char *class_pos;
        const char *href_pos;
        const char *url_start;
        const char *url_end;
        const char *title_start;
        const char *title_end;

        class_pos = strstr(p, "<a class=\"topic-title\"");

        if (class_pos == NULL)
            break;

        href_pos = strstr(class_pos, "href=\"");

        if (href_pos == NULL)
            break;

        url_start = href_pos + 6;
        url_end = strchr(url_start, '"');

        if (url_end == NULL)
            break;

        title_start = strchr(url_end, '>');

        if (title_start == NULL)
            break;

        title_start++;

        title_end = strstr(title_start, "</a>");

        if (title_end == NULL)
            break;

        {
            size_t url_len = (size_t)(url_end - url_start);
            size_t title_len = (size_t)(title_end - title_start);

            if (url_len >= sizeof(stories[count].url))
                url_len = sizeof(stories[count].url) - 1;

            if (title_len >= sizeof(stories[count].title))
                title_len = sizeof(stories[count].title) - 1;

            memcpy(stories[count].url,
                   url_start,
                   url_len);

            stories[count].url[url_len] = '\0';

            /*
             * Copy the title through the entity decoder.
             * We first copy it into a temporary buffer.
             */
            {
                char raw_title[MAX_TITLE];

                memcpy(raw_title,
                       title_start,
                       title_len);

                raw_title[title_len] = '\0';

                /*
                 * The titles are normally short enough that
                 * this direct pass is sufficient.
                 */
                {
                    size_t i = 0;
                    const char *q = raw_title;

                    while (*q && i < sizeof(stories[count].title) - 1)
                    {
                        if (strncmp(q, "&amp;", 5) == 0)
                        {
                            stories[count].title[i++] = '&';
                            q += 5;
                        }
                        else if (strncmp(q, "&quot;", 6) == 0)
                        {
                            stories[count].title[i++] = '"';
                            q += 6;
                        }
                        else if (strncmp(q, "&#39;", 5) == 0)
                        {
                            stories[count].title[i++] = '\'';
                            q += 5;
                        }
                        else if (strncmp(q, "&apos;", 6) == 0)
                        {
                            stories[count].title[i++] = '\'';
                            q += 6;
                        }
                        else if (strncmp(q, "&nbsp;", 6) == 0)
                        {
                            stories[count].title[i++] = ' ';
                            q += 6;
                        }
                        else
                        {
                            stories[count].title[i++] = *q++;
                        }
                    }

                    stories[count].title[i] = '\0';
                }
            }

            count++;
        }

        p = title_end + 4;
    }

    return count;
}


/*
 * Write a plain-text version of an NPR article.
 *
 * We deliberately stop before the transcript.
 */
static int convert_article(const char *html, FILE *out)
{
    const char *start;
    const char *p;
    int in_tag = 0;
    int in_script = 0;
    int last_blank = 0;

    start = strstr(html, "<div class=\"paragraphs-container\">");

    if (start == NULL)
        return -1;

    p = start + strlen("<div class=\"paragraphs-container\">");

    while (*p)
    {
        /*
         * The transcript is separate from the main story.
         */
        if (strncmp(p, "<h3>Transcript</h3>", 20) == 0)
            break;

        /*
         * Ignore scripts if NPR ever puts one inside the container.
         */
        if (!in_script && strncmp(p, "<script", 7) == 0)
        {
            in_script = 1;
            p += 7;
            continue;
        }

        if (in_script)
        {
            if (strncmp(p, "</script>", 9) == 0)
            {
                in_script = 0;
                p += 9;
            }
            else
            {
                p++;
            }

            continue;
        }

        /*
         * Paragraph / heading boundaries.
         */
        if (strncmp(p, "<p>", 3) == 0 ||
            strncmp(p, "<h3>", 4) == 0)
        {
            fputc('\n', out);
            p = strchr(p, '>');

            if (p == NULL)
                break;

            p++;
            continue;
        }

        if (strncmp(p, "</p>", 4) == 0 ||
            strncmp(p, "</h3>", 5) == 0)
        {
            fputc('\n', out);
            p = strchr(p, '>');

            if (p == NULL)
                break;

            p++;
            continue;
        }

        /*
         * Horizontal rules become blank lines.
         */
        if (strncmp(p, "<hr", 3) == 0)
        {
            fputs("\n\n", out);

            p = strchr(p, '>');

            if (p == NULL)
                break;

            p++;
            continue;
        }

        /*
         * Line breaks.
         */
        if (strncmp(p, "<br", 3) == 0)
        {
            fputc('\n', out);

            p = strchr(p, '>');

            if (p == NULL)
                break;

            p++;
            continue;
        }

        /*
         * Any other HTML tag.
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
         * Text.
         */
        if (!in_tag)
        {
            if (strncmp(p, "&amp;", 5) == 0)
            {
                fputc('&', out);
                p += 5;
            }
            else if (strncmp(p, "&quot;", 6) == 0)
            {
                fputc('"', out);
                p += 6;
            }
            else if (strncmp(p, "&#39;", 5) == 0)
            {
                fputc('\'', out);
                p += 5;
            }
            else if (strncmp(p, "&apos;", 6) == 0)
            {
                fputc('\'', out);
                p += 6;
            }
            else if (strncmp(p, "&nbsp;", 6) == 0)
            {
                fputc(' ', out);
                p += 6;
            }
            else
            {
                fputc(*p, out);
                p++;
            }

            last_blank = 0;
            continue;
        }

        p++;
    }

    /*
     * Keep the output tidy.
     */
    if (!last_blank)
        fputc('\n', out);

    return 0;
}


/*
 * Fetch and display NPR news.
 */
void npr_lookup(void)
{
    NPRStory stories[MAX_STORIES];

    char *html;
    long html_size;
int page = 0;
    int story_count;
    int choice;

    while (1)
    {
        printf("\n");
        printf("+---------------------------------------------+\n");
        printf("|                  NPR NEWS                   |\n");
        printf("+---------------------------------------------+\n");
        printf("\n");

        /*
         * Fetch the current NPR front page.
         */
        printf("Fetching NPR News...\n");

        if (fetch_url(NPR_HOME_URL, NPR_HOME_FILE) != 0)
        {
            printf("Unable to retrieve NPR News.\n");
            return;
        }

        html = read_file(NPR_HOME_FILE, &html_size);

        if (html == NULL)
        {
            printf("Unable to read NPR response.\n");
            return;
        }

        story_count = parse_headlines(
            html,
            stories,
            MAX_STORIES
        );

        free(html);

        if (story_count == 0)
        {
            printf("No NPR stories found.\n");
            return;
        }

        printf("\n");

//for (int i = 0; i < story_count; i++)
//{
//    printf("%2d. ", i + 1);
//    print_headline(stories[i].title);
//    printf("\n");
//}

while (1)
{
    int start = page * NPR_STORIES_PER_PAGE;
    int end = start + NPR_STORIES_PER_PAGE;

    if (end > story_count)
        end = story_count;

    printf("\n");
    printf("NPR NEWS\n");
    printf("\n");

    for (int i = start; i < end; i++)
    {
        printf("%2d. ", i + 1);
        print_headline(stories[i].title);
        printf("\n");
    }

    printf("\n");

    if (story_count > NPR_STORIES_PER_PAGE)
    {
        printf("N=Next  P=Previous  Q=Quit  %d-%d=Read\n",
               start + 1,
               end);
    }
    else
    {
        printf("1-%d=Read  Q=Quit\n",
               story_count);
    }

    printf("> ");

    if (scanf("%d", &choice) == 1)
    {
        getchar();

        if (choice >= start + 1 && choice <= end)
        {
            break;
        }

        printf("Invalid story number.\n");
        continue;
    }

    {
        char command;
        int c;

        /*
         * Clear the remainder of the input line.
         */
        command = (char)getchar();

        while ((c = getchar()) != '\n' && c != EOF)
        {
            /* discard input */
        }

        command = (char)tolower((unsigned char)command);

        if (command == 'q')
            return;

        if (command == 'n')
        {
            if (end < story_count)
                page++;

            continue;
        }

        if (command == 'p')
        {
            if (page > 0)
                page--;

            continue;
        }

        printf("Invalid command.\n");
    }
}

        /*
         * Build the selected article URL.
         */
        {
            char article_url[MAX_URL];

if (stories[choice - 1].url[0] != '/')
{
    printf("Invalid NPR article URL.\n");
    continue;
}

if (snprintf(article_url,
             sizeof(article_url),
             "%s%s",
             NPR_BASE_URL,
             stories[choice - 1].url)
    >= (int)sizeof(article_url))
{
    printf("NPR article URL is too long.\n");
    continue;
}

printf("\nFetching article...\n");

if (fetch_url(article_url,
              NPR_HOME_FILE) != 0)
{
    printf("Unable to retrieve article.\n");
    continue;
}

            html = read_file(NPR_HOME_FILE, &html_size);

            if (html == NULL)
            {
                printf("Unable to read article.\n");
                continue;
            }

            {
                FILE *out = fopen(NPR_TEXT_FILE, "w");

                if (out == NULL)
                {
                    free(html);
                    printf("Unable to create article text.\n");
                    continue;
                }

                /*
                 * Put the title at the top.
                 */
                fprintf(out,
                        "%s\n\n",
                        stories[choice - 1].title);

                if (convert_article(html, out) != 0)
                {
                    fclose(out);
                    free(html);

                    printf("Unable to extract article text.\n");
                    continue;
                }

                fprintf(out,
                        "\nSource: NPR\n"
                        "%s\n",
                        article_url);

                fclose(out);
            }

            free(html);

            /*
             * Let the existing Brother viewer do the work.
             */
            view_text_file(NPR_TEXT_FILE);
        }
    }
}