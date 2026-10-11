#define _POSIX_C_SOURCE 200809L

#include <curl/curl.h>
#include <json-c/json.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infokat.h"

#define INFOKAT_ENDPOINT "https://saalck-uky.primo.exlibrisgroup.com/primaws/rest/pub/pnxs"
#define INFOKAT_PAGE_SIZE 10
#define INFOKAT_MAX_RESPONSE (2U * 1024U * 1024U)

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} ResponseBuffer;

static size_t receive_data(void *contents, size_t size, size_t nmemb, void *userp)
{
    ResponseBuffer *buffer = userp;
    size_t bytes = size * nmemb;
    char *grown;

    if (bytes > INFOKAT_MAX_RESPONSE - buffer->length - 1)
        return 0;

    if (buffer->length + bytes + 1 > buffer->capacity) {
        size_t capacity = buffer->capacity ? buffer->capacity : 8192;

        while (capacity < buffer->length + bytes + 1)
            capacity *= 2;

        grown = realloc(buffer->data, capacity);
        if (grown == NULL)
            return 0;

        buffer->data = grown;
        buffer->capacity = capacity;
    }

    memcpy(buffer->data + buffer->length, contents, bytes);
    buffer->length += bytes;
    buffer->data[buffer->length] = '\0';

    return bytes;
}

static struct json_object *search_catalog(const char *query, int offset)
{
    CURL *curl;
    CURLcode result;
    char *escaped = NULL;
    char *url = NULL;
    long status = 0;
    ResponseBuffer buffer = {0};
    struct json_object *json = NULL;
    struct json_tokener *tokener = NULL;

    curl = curl_easy_init();
    if (curl == NULL)
        return NULL;

    escaped = curl_easy_escape(curl, query, 0);
    if (escaped == NULL)
        goto cleanup;

    {
        size_t needed = strlen(INFOKAT_ENDPOINT) + strlen(escaped) + 512;
        url = malloc(needed);
        if (url == NULL)
            goto cleanup;

        snprintf(url, needed,
            INFOKAT_ENDPOINT
            "?acTriggered=false"
            "&blendFacetsSeparately=false"
            "&citationTrailFilterByAvailability=true"
            "&disableCache=false"
            "&getMore=0"
            "&inst=01SAA_UKY"
            "&isCDSearch=false"
            "&lang=en"
            "&limit=%d"
            "&newspapersActive=false"
            "&newspapersSearch=false"
            "&offset=%d"
            "&otbRanking=false"
            "&pcAvailability=true"
            "&q=any,contains,%s"
            "&qExclude="
            "&qInclude="
            "&rapido=false"
            "&refEntryActive=false"
            "&rtaLinks=true"
            "&scope=MyInst_and_CI"
            "&searchInFulltextUserSelection=false"
            "&skipDelivery=Y"
            "&sort=rank"
            "&tab=Everything"
            "&vid=01SAA_UKY%%3AUKY",
            INFOKAT_PAGE_SIZE, offset, escaped);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BrotherPowerTools/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        fprintf(stderr, "\nInfoKat request failed: %s\n",
                curl_easy_strerror(result));
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (status < 200 || status >= 300) {
        fprintf(stderr, "\nInfoKat returned HTTP %ld.\n", status);
        goto cleanup;
    }

    if (buffer.data == NULL) {
        fprintf(stderr, "\nInfoKat returned an empty response.\n");
        goto cleanup;
    }

    tokener = json_tokener_new();
    if (tokener == NULL)
        goto cleanup;

    json = json_tokener_parse_ex(tokener, buffer.data, (int)buffer.length);
    if (json == NULL || json_tokener_get_error(tokener) != json_tokener_success) {
        fprintf(stderr, "\nCould not parse InfoKat's JSON response.\n");
        if (json != NULL) {
            json_object_put(json);
            json = NULL;
        }
    }

cleanup:
    if (tokener != NULL)
        json_tokener_free(tokener);
    free(buffer.data);
    free(url);
    if (escaped != NULL)
        curl_free(escaped);
    curl_easy_cleanup(curl);
    return json;
}

static struct json_object *member(struct json_object *object, const char *key)
{
    struct json_object *value = NULL;

    if (object != NULL)
        json_object_object_get_ex(object, key, &value);

    return value;
}

static const char *first_value(struct json_object *object, const char *key)
{
    struct json_object *value = member(object, key);

    if (value == NULL || json_object_is_type(value, json_type_null))
        return NULL;

    if (json_object_is_type(value, json_type_array)) {
        if (json_object_array_length(value) == 0)
            return NULL;
        value = json_object_array_get_idx(value, 0);
    }

    if (value == NULL || json_object_is_type(value, json_type_null))
        return NULL;

    if (json_object_is_type(value, json_type_string))
        return json_object_get_string(value);

    return NULL;
}

static void print_field(const char *label, const char *value)
{
    if (value != NULL && value[0] != '\0')
        printf("%-13s %s\n", label, value);
}

static void print_record(struct json_object *doc)
{
    struct json_object *pnx = member(doc, "pnx");
    struct json_object *display = member(pnx, "display");
    struct json_object *addata = member(pnx, "addata");
    struct json_object *delivery = member(doc, "delivery");
    struct json_object *location = member(delivery, "bestlocation");
    struct json_object *subjects = member(display, "subject");
    size_t i;

    printf("\n");
    printf("+------------------------------------------------------------------------------+\n");
    printf("|                              INFO KAT RECORD                                 |\n");
    printf("+------------------------------------------------------------------------------+\n");

    print_field("Title:", first_value(display, "title"));
    print_field("Author:", first_value(addata, "au"));
    print_field("Year:", first_value(addata, "date"));
    print_field("Publisher:", first_value(addata, "pub"));
    print_field("Place:", first_value(addata, "cop"));
    print_field("Type:", first_value(display, "type"));
    print_field("Description:", first_value(display, "format"));

    if (subjects != NULL && json_object_is_type(subjects, json_type_array)) {
        printf("\nSubjects:\n");
        for (i = 0; i < json_object_array_length(subjects); i++) {
            struct json_object *subject = json_object_array_get_idx(subjects, i);
            const char *text = subject ? json_object_get_string(subject) : NULL;
            if (text != NULL)
                printf("  %.76s\n", text);
        }
    }

    printf("\nUK Libraries:\n");
    if (location != NULL) {
        print_field("Status:", first_value(location, "availabilityStatus"));
        print_field("Library:", first_value(location, "mainLocation"));
        print_field("Collection:", first_value(location, "subLocation"));
        print_field("Call number:", first_value(location, "callNumber"));
    } else {
        printf("  No local holding details supplied for this result.\n");
    }

    print_field("OCLC:", first_value(addata, "oclcid"));
    print_field("UK Record:", first_value(display, "mms"));

    printf("\nPress Enter to return to results...");
    while (getchar() != '\n' && !feof(stdin)) {
        /* consume input */
    }
}

static void show_results(struct json_object *json,
                         const char *query,
                         int offset,
                         int total)
{
    struct json_object *docs = member(json, "docs");
    int count = docs && json_object_is_type(docs, json_type_array)
        ? (int)json_object_array_length(docs) : 0;
    int choice;
    int i;
    int page = offset / INFOKAT_PAGE_SIZE + 1;
    int pages = total > 0 ? (total + INFOKAT_PAGE_SIZE - 1) / INFOKAT_PAGE_SIZE : 1;

    for (;;) {
        printf("\n");
        printf("+------------------------------------------------------------------------------+\n");
        printf("|                              INFOKAT SEARCH                                  |\n");
        printf("+------------------------------------------------------------------------------+\n");
        printf(" Query: %.65s\n", query);
        printf(" Results %d-%d of %d (page %d of %d)\n\n",
               count ? offset + 1 : 0, offset + count, total, page, pages);

        if (count == 0) {
            printf("No results returned.\n");
        }

        for (i = 0; i < count; i++) {
            struct json_object *doc = json_object_array_get_idx(docs, i);
            struct json_object *pnx = member(doc, "pnx");
            struct json_object *display = member(pnx, "display");
            struct json_object *addata = member(pnx, "addata");
            struct json_object *delivery = member(doc, "delivery");
            struct json_object *location = member(delivery, "bestlocation");
            const char *title = first_value(display, "title");
            const char *author = first_value(addata, "au");
            const char *year = first_value(addata, "date");
            const char *library = first_value(location, "mainLocation");
            const char *call = first_value(location, "callNumber");

            printf("%2d. %.72s\n", i + 1, title ? title : "(Untitled)");
            printf("    %.45s", author ? author : "Author unknown");
            if (year != NULL)
                printf("  (%s)", year);
            putchar('\n');
            if (library != NULL)
                printf("    %.34s", library);
            if (call != NULL)
                printf("  %.30s", call);
            if (library != NULL || call != NULL)
                putchar('\n');
        }

        printf("\n");
        printf("Enter 1-%d to view a record", count);
        printf(" | N next | P previous | S new search | B back\n> ");

        {
            char input[64];
            char *end;
            long parsed;

            if (fgets(input, sizeof(input), stdin) == NULL)
                return;
            input[strcspn(input, "\r\n")] = '\0';

            if (strcasecmp(input, "b") == 0 || strcasecmp(input, "q") == 0)
                return;
            if (strcasecmp(input, "s") == 0) {
                printf("Enter new search: ");
                if (fgets(input, sizeof(input), stdin) == NULL)
                    return;
                input[strcspn(input, "\r\n")] = '\0';
                if (input[0] != '\0') {
                    /* A special value signals the caller to start a new search. */
                    strncpy((char *)query, input, 255);
                }
                return;
            }
            if (strcasecmp(input, "n") == 0 || strcasecmp(input, "p") == 0)
                return;

            parsed = strtol(input, &end, 10);
            if (end != input && *end == '\0' && parsed >= 1 && parsed <= count) {
                print_record(json_object_array_get_idx(docs, (size_t)parsed - 1));
            } else {
                printf("Invalid selection.\n");
            }
        }
    }
}

void infokat_lookup(void)
{
    char query[256];
    int offset = 0;
    int total = 0;
    int action = 0;

    printf("\nInfoKat — University of Kentucky Libraries\n");
    printf("Search by title, author, subject, or keyword.\n");
    printf("Search: ");

    if (fgets(query, sizeof(query), stdin) == NULL)
        return;
    query[strcspn(query, "\r\n")] = '\0';
    if (query[0] == '\0')
        return;

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        printf("\nCould not initialize network library.\n");
        return;
    }

    while (1) {
        struct json_object *json = search_catalog(query, offset);
        struct json_object *info;
        struct json_object *docs;
        int count;

        if (json == NULL) {
            printf("\nUnable to retrieve InfoKat results.\n");
            printf("Press Enter to return...");
            while (getchar() != '\n' && !feof(stdin)) {}
            break;
        }

        info = member(json, "info");
        docs = member(json, "docs");
        total = info ? json_object_get_int(member(info, "total")) : 0;
        count = docs && json_object_is_type(docs, json_type_array)
            ? (int)json_object_array_length(docs) : 0;

        if (offset >= total && offset > 0) {
            offset = ((total > 0 ? total - 1 : 0) / INFOKAT_PAGE_SIZE) * INFOKAT_PAGE_SIZE;
            json_object_put(json);
            continue;
        }

        show_results(json, query, offset, total);
        json_object_put(json);

        /*
         * The results display accepts navigation/search commands. The next
         * version will keep these as explicit actions; for now we prompt for
         * the action after returning from the result list.
         */
        printf("\nN next page | P previous page | S new search | B back\n> ");
        {
            char input[64];
            if (fgets(input, sizeof(input), stdin) == NULL)
                break;
            input[strcspn(input, "\r\n")] = '\0';

            if (strcasecmp(input, "b") == 0 || strcasecmp(input, "q") == 0)
                break;
            if (strcasecmp(input, "n") == 0) {
                if (offset + INFOKAT_PAGE_SIZE < total)
                    offset += INFOKAT_PAGE_SIZE;
                else
                    printf("Already at the last page.\n");
            } else if (strcasecmp(input, "p") == 0) {
                if (offset >= INFOKAT_PAGE_SIZE)
                    offset -= INFOKAT_PAGE_SIZE;
                else
                    printf("Already at the first page.\n");
            } else if (strcasecmp(input, "s") == 0) {
                printf("New search: ");
                if (fgets(query, sizeof(query), stdin) == NULL)
                    break;
                query[strcspn(query, "\r\n")] = '\0';
                if (query[0] != '\0')
                    offset = 0;
            } else {
                printf("Returning to InfoKat search.\n");
            }
        }
        (void)count;
        (void)action;
    }

    curl_global_cleanup();
}
