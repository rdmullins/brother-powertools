#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "wikipedia.h"
#include "transfer.h"
#include "viewer.h"

#define WIKIPEDIA_API_URL \
    "https://en.wikipedia.org/w/api.php?action=query" \
    "&prop=extracts|links" \
    "&explaintext=1&exsectionformat=wiki" \
    "&plnamespace=0" \
    "&pllimit=100" \
    "&titles=%s&format=json"

#define WIKIPEDIA_SEARCH_JSON_FILE \
    "/tmp/powertools-wikipedia-search.json"

#define WIKIPEDIA_JSON_FILE "/tmp/powertools-wikipedia.json"
#define WIKIPEDIA_TEXT_FILE "/tmp/powertools-wikipedia.txt"
#define WIKIPEDIA_LINKS_JSON_FILE \
    "/tmp/powertools-wikipedia-links.json"

typedef struct {
    char prefix[512];
    int current_part;
    int total_parts;
} wikipedia_transfer_t;

static long get_file_size(const char *filename)
{
    FILE *file;
    long size;

    file = fopen(filename, "rb");

    if (file == NULL) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    size = ftell(file);

    fclose(file);

    return size;
}

static void wikipedia_browse_links(const char *article);

static int json_extract_text(
    const char *input,
    const char *output)
{
    FILE *destination;
    json_object *root = NULL;
    json_object *query = NULL;
    json_object *pages = NULL;
    json_object *page = NULL;
    json_object *extract = NULL;

    root = json_object_from_file(input);

    if (root == NULL)
        return -1;

    if (!json_object_object_get_ex(
            root,
            "query",
            &query))
    {
        json_object_put(root);
        return -1;
    }

    if (!json_object_object_get_ex(
            query,
            "pages",
            &pages))
    {
        json_object_put(root);
        return -1;
    }

    /*
     * Wikipedia uses the page ID as the key under
     * "pages". We only need the first page returned.
     */
    json_object_object_foreach(
        pages,
        page_id,
        page_object)
    {
        (void)page_id;

        page = page_object;
        break;
    }

    if (page == NULL)
    {
        json_object_put(root);
        return -1;
    }

    if (!json_object_object_get_ex(
            page,
            "extract",
            &extract))
    {
        json_object_put(root);
        return -1;
    }

    const char *text =
        json_object_get_string(extract);

    if (text == NULL)
    {
        json_object_put(root);
        return -1;
    }

    destination = fopen(output, "w");

    if (destination == NULL)
    {
        json_object_put(root);
        return -1;
    }

    fputs(text, destination);

    fclose(destination);

    json_object_put(root);

    return 0;
}

static int json_extract_links(
    const char *input,
    WikipediaLink links[],
    int max_links)
{
    json_object *root = NULL;
    json_object *query = NULL;
    json_object *pages = NULL;
    json_object *page = NULL;
    json_object *page_links = NULL;
    json_object *link = NULL;
    json_object *title = NULL;

    if (input == NULL ||
        links == NULL ||
        max_links <= 0)
    {
        return -1;
    }

    root = json_object_from_file(input);

    if (root == NULL)
        return -1;

    if (!json_object_object_get_ex(
            root,
            "query",
            &query))
    {
        json_object_put(root);
        return -1;
    }

    if (!json_object_object_get_ex(
            query,
            "pages",
            &pages))
    {
        json_object_put(root);
        return -1;
    }

    json_object_object_foreach(
        pages,
        page_id,
        page_object)
    {
        (void)page_id;

        page = page_object;
        break;
    }

    if (page == NULL)
    {
        json_object_put(root);
        return -1;
    }

    if (!json_object_object_get_ex(
            page,
            "links",
            &page_links))
    {
        /*
         * An article with no links is a valid result.
         */
        json_object_put(root);
        return 0;
    }

    int link_count = 0;

    int array_length =
        json_object_array_length(page_links);

    for (int i = 0;
         i < array_length && link_count < max_links;
         i++)
    {
        link = json_object_array_get_idx(
            page_links,
            i
        );

        if (link == NULL)
            continue;

        if (!json_object_object_get_ex(
                link,
                "title",
                &title))
        {
            continue;
        }

        const char *link_title =
            json_object_get_string(title);

        if (link_title == NULL)
            continue;

        snprintf(
            links[link_count].title,
            sizeof(links[link_count].title),
            "%s",
            link_title
        );

        link_count++;
    }

    json_object_put(root);

    return link_count;
}

static int json_extract_link_continuation(
    const char *input,
    char *continuation,
    size_t continuation_size)
{
    json_object *root = NULL;
    json_object *continue_object = NULL;
    json_object *plcontinue = NULL;
    const char *value;

    if (input == NULL ||
        continuation == NULL ||
        continuation_size == 0)
    {
        return -1;
    }

    continuation[0] = '\0';

    root = json_object_from_file(input);

    if (root == NULL)
        return -1;

    if (!json_object_object_get_ex(
            root,
            "continue",
            &continue_object))
    {
        json_object_put(root);
        return 0;
    }

    if (!json_object_object_get_ex(
            continue_object,
            "plcontinue",
            &plcontinue))
    {
        json_object_put(root);
        return 0;
    }

    value = json_object_get_string(plcontinue);

    if (value == NULL)
    {
        json_object_put(root);
        return 0;
    }

    snprintf(
        continuation,
        continuation_size,
        "%s",
        value
    );

    json_object_put(root);

    return 1;
}

static void wikipedia_url_encode(
    const char *input,
    char *output,
    size_t output_size);


int wikipedia_fetch_article(const char *article)
{
    char encoded_article[768];
    char url[1536];
    char command[2048];

    if (article == NULL || article[0] == '\0')
        return -1;

    wikipedia_url_encode(
        article,
        encoded_article,
        sizeof(encoded_article)
    );

    snprintf(
        url,
        sizeof(url),
        WIKIPEDIA_API_URL,
        encoded_article
    );

    snprintf(
        command,
        sizeof(command),
        "curl -L -s '%s' -o '%s'",
        url,
        WIKIPEDIA_JSON_FILE
    );

    if (system(command) != 0)
        return -1;

    if (json_extract_text(
            WIKIPEDIA_JSON_FILE,
            WIKIPEDIA_TEXT_FILE) != 0)
    {
        return -1;
    }

    if (get_file_size(WIKIPEDIA_TEXT_FILE) <= 0)
        return -1;

    return 0;
}


static void wikipedia_url_encode(
    const char *input,
    char *output,
    size_t output_size)
{
    static const char hex[] =
        "0123456789ABCDEF";

    size_t pos = 0;

    while (*input != '\0' &&
           pos + 4 < output_size)
    {
        unsigned char ch =
            (unsigned char)*input++;

        if (isalnum(ch) ||
            ch == '-' ||
            ch == '_' ||
            ch == '.' ||
            ch == '~')
        {
            output[pos++] = (char)ch;
        }
        else
        {
            output[pos++] = '%';
            output[pos++] = hex[ch >> 4];
            output[pos++] = hex[ch & 0x0F];
        }
    }

    output[pos] = '\0';
}


/*
 * Remove HTML tags from Wikipedia search snippets.
 * The API wraps matching words in HTML span elements.
 */
static void wikipedia_strip_html(
    const char *input,
    char *output,
    size_t output_size)
{
    int in_tag = 0;
    size_t pos = 0;

    if (input == NULL ||
        output == NULL ||
        output_size == 0)
    {
        return;
    }

    for (size_t i = 0;
         input[i] != '\0' && pos < output_size - 1;
         i++)
    {
        if (input[i] == '<')
        {
            in_tag = 1;
            continue;
        }

        if (input[i] == '>')
        {
            in_tag = 0;

            if (pos > 0 && output[pos - 1] != ' ')
                output[pos++] = ' ';

            continue;
        }

        if (!in_tag)
        {
            unsigned char ch = (unsigned char)input[i];

            if (isspace(ch))
            {
                if (pos > 0 && output[pos - 1] != ' ')
                    output[pos++] = ' ';
            }
            else
            {
                output[pos++] = (char)ch;
            }
        }
    }

    if (pos > 0 && output[pos - 1] == ' ')
        pos--;

    output[pos] = '\0';
}


/*
 * Extract Wikipedia search results from JSON.
 */
static int json_extract_search_results(
    const char *input,
    WikipediaSearchResult results[],
    int max_results)
{
    json_object *root = NULL;
    json_object *query = NULL;
    json_object *search = NULL;
    json_object *item = NULL;
    json_object *title = NULL;
    json_object *snippet = NULL;

    if (input == NULL ||
        results == NULL ||
        max_results <= 0)
    {
        return -1;
    }

    root = json_object_from_file(input);

    if (root == NULL)
        return -1;

    if (!json_object_object_get_ex(
            root, "query", &query))
    {
        json_object_put(root);
        return -1;
    }

    if (!json_object_object_get_ex(
            query, "search", &search))
    {
        json_object_put(root);
        return -1;
    }

    int result_count = 0;

    int array_length =
        json_object_array_length(search);

    for (int i = 0;
         i < array_length && result_count < max_results;
         i++)
    {
        item = json_object_array_get_idx(search, i);

        if (item == NULL)
            continue;

        if (!json_object_object_get_ex(
                item, "title", &title))
        {
            continue;
        }

        const char *result_title =
            json_object_get_string(title);

        if (result_title == NULL)
            continue;

        snprintf(
            results[result_count].title,
            sizeof(results[result_count].title),
            "%s",
            result_title
        );

        results[result_count].snippet[0] = '\0';

        if (json_object_object_get_ex(
                item, "snippet", &snippet))
        {
            const char *result_snippet =
                json_object_get_string(snippet);

            if (result_snippet != NULL)
            {
                wikipedia_strip_html(
                    result_snippet,
                    results[result_count].snippet,
                    sizeof(results[result_count].snippet)
                );
            }
        }

        result_count++;
    }

    json_object_put(root);

    return result_count;
}


/*
 * Search Wikipedia and populate the results array.
 */
int wikipedia_search(
    const char *query,
    WikipediaSearchResult results[],
    int max_results)
{
    char encoded_query[1024];
    char url[1536];
    char command[2048];

    if (query == NULL ||
        query[0] == '\0' ||
        results == NULL ||
        max_results <= 0)
    {
        return -1;
    }

    wikipedia_url_encode(
        query,
        encoded_query,
        sizeof(encoded_query)
    );

    snprintf(
        url,
        sizeof(url),
        "https://en.wikipedia.org/w/api.php"
        "?action=query"
        "&list=search"
        "&srsearch=%s"
        "&srlimit=%d"
        "&srprop=snippet"
        "&format=json",
        encoded_query,
        max_results
    );

    snprintf(
        command,
        sizeof(command),
        "curl -L -s '%s' -o '%s'",
        url,
        WIKIPEDIA_SEARCH_JSON_FILE
    );

    if (system(command) != 0)
        return -1;

    return json_extract_search_results(
        WIKIPEDIA_SEARCH_JSON_FILE,
        results,
        max_results
    );
}


static int wikipedia_fetch_more_links(
    const char *article,
    WikipediaLink links[],
    int max_links,
    int *link_count)
{
    char encoded_article[256];
    char continuation[512];
    char url[1024];
    char command[1280];

    if (article == NULL ||
        links == NULL ||
        link_count == NULL ||
        max_links <= 0)
    {
        return -1;
    }

    snprintf(
        encoded_article,
        sizeof(encoded_article),
        "%s",
        article
    );

    for (char *p = encoded_article; *p != '\0'; p++)
    {
        if (*p == ' ')
            *p = '_';
    }

    continuation[0] = '\0';

    while (*link_count < max_links)
    {
        snprintf(
            url,
            sizeof(url),
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&prop=links"
            "&plnamespace=0"
            "&pllimit=100"
            "&titles=%s"
            "&format=json",
            encoded_article
        );
if (continuation[0] != '\0')
{
    char encoded_continuation[1024];

    wikipedia_url_encode(
        continuation,
        encoded_continuation,
        sizeof(encoded_continuation)
    );

    snprintf(
        url + strlen(url),
        sizeof(url) - strlen(url),
        "&plcontinue=%s",
        encoded_continuation
    );
}

        snprintf(
            command,
            sizeof(command),
            "curl -L -s '%s' -o '%s'",
            url,
            WIKIPEDIA_LINKS_JSON_FILE
        );
if (system(command) != 0)
{
    printf("\nLink curl failed.\n");
    return -1;
}

        int old_count = *link_count;

        int added =
            json_extract_links(
                WIKIPEDIA_LINKS_JSON_FILE,
                links + *link_count,
                max_links - *link_count
            );
if (added < 0)
{
    printf("\nLink JSON parsing failed.\n");
    return -1;
}

        *link_count += added;

        /*
         * Get the continuation token for the next
         * page of links.
         */
        int continuation_result =
            json_extract_link_continuation(
                WIKIPEDIA_LINKS_JSON_FILE,
                continuation,
                sizeof(continuation)
            );

if (continuation_result < 0)
{
    printf("\nContinuation parsing failed.\n");
    return -1;
}

        /*
         * No continuation means we have reached the
         * end of the link list.
         */
        if (continuation_result == 0)
            break;

        /*
         * Protect against a response that somehow
         * gives us no new links.
         */
        if (*link_count == old_count)
            break;
    }

    return 0;
}

int wikipedia_load_links(
    const char *article,
    WikipediaLink links[],
    int max_links)
{
    int link_count = 0;

    if (article == NULL ||
        links == NULL ||
        max_links <= 0)
    {
        return -1;
    }

    if (wikipedia_fetch_more_links(
            article,
            links,
            max_links,
            &link_count) != 0)
    {
        return -1;
    }

    return link_count;
}

static void wikipedia_browse_links(const char *article)
{
    WikipediaLink links[WIKIPEDIA_MAX_LINKS];
    char input[32];
    int link_count;
    int page = 0;

    link_count = wikipedia_load_links(
        article,
        links,
        WIKIPEDIA_MAX_LINKS
    );

    if (link_count <= 0) {
        printf("\nNo Wikipedia links found.\n");
        printf("Press Enter to return...");
        getchar();
        return;
    }

    while (1) {
        int start = page * 20;
        int end = start + 20;

        if (start >= link_count) {
            page = 0;
            continue;
        }

        if (end > link_count) {
            end = link_count;
        }

        printf("\n");
        printf("+---------------------------------------------+\n");
        printf("|              Wikipedia Links                |\n");
        printf("+---------------------------------------------+\n");
        printf("\n");

        for (int i = start; i < end; i++) {
            printf("%2d. %s\n",
                   i + 1,
                   links[i].title);
        }

        printf("\n");
        printf("N. Next page    P. Previous page\n");
        printf("B. Back to article\n");
        printf("\n");
        printf("Enter link number or command: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            return;
        }

        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0') {
            continue;
        }

        if (input[0] == 'b' || input[0] == 'B') {
            return;
        }

        if (input[0] == 'n' || input[0] == 'N') {
            if (end < link_count) {
                page++;
            } else {
                printf("\nAlready on the last page.\n");
            }
            continue;
        }

        if (input[0] == 'p' || input[0] == 'P') {
            if (page > 0) {
                page--;
            } else {
                printf("\nAlready on the first page.\n");
            }
            continue;
        }

        int choice = atoi(input);

        if (choice >= 1 && choice <= link_count) {
            printf("\nFetching %s...\n",
                   links[choice - 1].title);

            if (wikipedia_fetch_article(
                    links[choice - 1].title) != 0) {
                printf("Unable to retrieve article.\n");
                printf("Press Enter to continue...");
                getchar();
                continue;
            }

            if (json_extract_text(
                    WIKIPEDIA_JSON_FILE,
                    WIKIPEDIA_TEXT_FILE) != 0) {
                printf("Unable to extract article text.\n");
                printf("Press Enter to continue...");
                getchar();
                continue;
            }

            view_text_file(WIKIPEDIA_TEXT_FILE);

            /*
             * After reading the linked article, return to the
             * link list for the original article.
             */
            continue;
        }

        printf("\nInvalid selection.\n");
    }
}

void wikipedia_lookup(void)

{
    char article[256];
    char query[256];
    char input[32];

    WikipediaSearchResult results[
        WIKIPEDIA_MAX_SEARCH_RESULTS
    ];

    int choice;
    int result_count;
    int page;
    int start;
    int end;
    int selection;
    int article_selected = 0;

    while (!article_selected)
    {
        printf("\n");
        printf("+---------------------------------------------+\n");
        printf("|                  Wikipedia                  |\n");
        printf("+---------------------------------------------+\n");
        printf("\n");
        printf("1. Search Wikipedia\n");
        printf("2. Enter article title\n");
        printf("3. Back\n");
        printf("\n");
        printf("Enter your choice: ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            return;

        choice = atoi(input);

        switch (choice)
        {
            case 1:
            {
                printf("\nEnter search terms:\n");
                printf("> ");

                if (fgets(query, sizeof(query), stdin) == NULL)
                    return;

                query[strcspn(query, "\n")] = '\0';

                if (query[0] == '\0')
                    break;

                printf("\nSearching Wikipedia...\n");

                result_count = wikipedia_search(
                    query,
                    results,
                    WIKIPEDIA_MAX_SEARCH_RESULTS
                );

                if (result_count < 0)
                {
                    printf("\nWikipedia search failed.\n");
                    break;
                }

                if (result_count == 0)
                {
                    printf("\nNo matching articles found.\n");
                    break;
                }

                page = 0;

                while (1)
                {
                    start = page * 10;
                    end = start + 10;

                    if (end > result_count)
                        end = result_count;

                    printf("\n");
                    printf("+---------------------------------------------+\n");
                    printf("|              Wikipedia Results              |\n");
                    printf("+---------------------------------------------+\n");
                    printf("\n");

                    for (int i = start; i < end; i++)
                    {
                        printf("%2d. %s\n",
                               i - start + 1,
                               results[i].title);
                    }

                    printf("\n");
                    printf("Results %d-%d of %d\n",
                           start + 1,
                           end,
                           result_count);

                    printf("\n");
                    printf("N. Next page    P. Previous page\n");
                    printf("B. Back to search menu\n");
                    printf("\n");
                    printf("Enter article number or command: ");

                    if (fgets(input, sizeof(input), stdin) == NULL)
                        return;

                    input[strcspn(input, "\n")] = '\0';

                    if (input[0] == '\0')
                        continue;

                    if (input[0] == 'b' || input[0] == 'B')
                        break;

                    if (input[0] == 'n' || input[0] == 'N')
                    {
                        if (end < result_count)
                            page++;
                        else
                            printf("\nAlready on the last page.\n");

                        continue;
                    }

                    if (input[0] == 'p' || input[0] == 'P')
                    {
                        if (page > 0)
                            page--;
                        else
                            printf("\nAlready on the first page.\n");

                        continue;
                    }

                    selection = atoi(input);

                    if (selection >= 1 &&
                        selection <= end - start)
                    {
                        snprintf(
                            article,
                            sizeof(article),
                            "%s",
                            results[start + selection - 1].title
                        );

                        article_selected = 1;
                        break;
                    }

                    printf("\nInvalid selection.\n");
                }

                break;
            }

            case 2:
            {
                printf("\nEnter article title:\n");
                printf("> ");

                if (fgets(article, sizeof(article), stdin) == NULL)
                    return;

                article[strcspn(article, "\n")] = '\0';

                if (article[0] != '\0')
                    article_selected = 1;

                break;
            }

            case 3:
                return;

            default:
                printf("\nInvalid choice.\n");
                break;
        }
    }

    printf("\nFetching %s...\n", article);

    if (wikipedia_fetch_article(article) != 0)
    {
        printf("\nUnable to retrieve article.\n");
        return;
    }



    long article_size;
size_t max_size;
int parts;
wikipedia_transfer_t transfer;

article_size = get_file_size(WIKIPEDIA_TEXT_FILE);

if (article_size < 0) {
    printf("Unable to determine article size.\n");
    return;
}

while (1) {

printf("\n");
printf("Article retrieved successfully.\n");
printf("Article size: %ld bytes\n", article_size);
printf("\n");
printf("1. Read on screen\n");
printf("2. Browse links\n");
printf("3. Send article to PowerNote\n");
printf("4. Back\n");
printf("\n");
printf("Enter your choice: ");

if (scanf("%d", &choice) != 1) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard invalid input */
    }

    printf("Invalid input.\n");
    break;
}


getchar();

    switch (choice) {

        case 1:
            view_text_file(WIKIPEDIA_TEXT_FILE);
            break;

        case 2:
            wikipedia_browse_links(article);
            break;

        case 3: {
        char part_filename[1024];

        printf("\n");
        printf("How much free PowerNote memory is available?\n");
        printf("Enter maximum part size in bytes: ");

        if (scanf("%zu", &max_size) != 1) {
            int c;

            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard invalid input */
            }

            printf("Invalid size.\n");
            return;
        }

        getchar();

        if (max_size == 0) {
            printf("Size must be greater than zero.\n");
            return;
        }

        parts = split_file(
            WIKIPEDIA_TEXT_FILE,
            "/tmp/powertools-wikipedia",
            max_size
        );

        if (parts < 0) {
            printf("Unable to split article.\n");
            return;
        }
        strncpy(transfer.prefix,
        "/tmp/powertools-wikipedia",
        sizeof(transfer.prefix) - 1);

transfer.prefix[sizeof(transfer.prefix) - 1] = '\0';

transfer.current_part = 1;
transfer.total_parts = parts;

        printf("\n");
        printf("The article will be sent in %d parts.\n", parts);

        transfer.current_part = 1;

        while (transfer.current_part <= transfer.total_parts) {
            snprintf(part_filename,
                     sizeof(part_filename),
                     "%s_%02d.txt",
                     transfer.prefix,
                     transfer.current_part);

            if (send_transfer_part(part_filename,
                                   transfer.current_part,
                                   transfer.total_parts) != 0) {
                return;
            }

            if (transfer.current_part == transfer.total_parts) {
                printf("\n");
                printf("All parts have been transferred.\n");
                break;
            }

            printf("\n");
            printf("+---------------------------------------------+\n");
            printf("|              brother PowerTools             |\n");
            printf("|              Wikipedia Transfer             |\n");
            printf("+---------------------------------------------+\n");
            printf("| Part %d of %d complete.                      |\n",
                   transfer.current_part,
                   transfer.total_parts);
            printf("|                                             |\n");
            printf("| 1. Send next part                           |\n");
            printf("| 2. Cancel transfer                          |\n");
            printf("+---------------------------------------------+\n");
            printf("\n");
            printf("Enter your choice: ");

            if (scanf("%d", &choice) != 1) {
                int c;

                while ((c = getchar()) != '\n' && c != EOF) {
                    /* discard invalid input */
                }

                printf("Invalid input. Transfer cancelled.\n");
                return;
            }

            getchar();

            if (choice == 1) {
                transfer.current_part++;
            } else {
                printf("Transfer cancelled.\n");
                return;
            }
        }

        break;
    }

    case 4:
        return;

    default:
        printf("Invalid option.\n");
        return;
}
}}