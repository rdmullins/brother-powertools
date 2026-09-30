#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "openlibrary.h"

#define OPENLIBRARY_URL_MAX 256

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

static size_t write_callback(void *contents,
                             size_t size,
                             size_t nmemb,
                             void *userp)
{
    size_t total = size * nmemb;
    ResponseBuffer *response = userp;
    char *new_data;

    new_data = realloc(response->data,
                       response->size + total + 1);

    if (new_data == NULL) {
        return 0;
    }

    response->data = new_data;

    memcpy(response->data + response->size,
           contents,
           total);

    response->size += total;
    response->data[response->size] = '\0';

    return total;
}

static void copy_json_string(char *destination,
                             size_t destination_size,
                             struct json_object *object,
                             const char *key)
{
    struct json_object *value;

    if (destination == NULL ||
        destination_size == 0 ||
        object == NULL) {
        return;
    }

    if (!json_object_object_get_ex(object, key, &value)) {
        return;
    }

    if (!json_object_is_type(value, json_type_string)) {
        return;
    }

    strncpy(destination,
            json_object_get_string(value),
            destination_size - 1);

    destination[destination_size - 1] = '\0';
}

static int openlibrary_get_author_name(
    const char *author_key,
    char *name,
    size_t name_size)
{
    CURL *curl;
    CURLcode result;
    ResponseBuffer response = { NULL, 0 };
    struct json_object *root;
    char url[OPENLIBRARY_URL_MAX];

    if (author_key == NULL ||
        name == NULL ||
        name_size == 0)
    {
        return -1;
    }

    name[0] = '\0';

    snprintf(
        url,
        sizeof(url),
        "https://openlibrary.org%s.json",
        author_key
    );

    curl = curl_easy_init();

    if (curl == NULL)
        return -1;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "BrotherPowerTools/1.0"
    );
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    );
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );
    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        free(response.data);
        return -1;
    }

    if (response.data == NULL)
        return -1;

    root = json_tokener_parse(response.data);

    free(response.data);

    if (root == NULL)
        return -1;

    copy_json_string(
        name,
        name_size,
        root,
        "name"
    );

    json_object_put(root);

    if (name[0] == '\0')
        return -1;

    return 0;
}

int openlibrary_lookup_isbn(
    const char *isbn,
    OpenLibraryBook *book)
{
    CURL *curl;
    CURLcode result;
    ResponseBuffer response = { NULL, 0 };
    struct json_object *root;
    struct json_object *isbn_13;
    struct json_object *isbn_10;
    struct json_object *authors;
    struct json_object *publishers;
    struct json_object *subjects;
    char url[OPENLIBRARY_URL_MAX];

    if (isbn == NULL || book == NULL)
        return -1;

    memset(book, 0, sizeof(*book));

    snprintf(
        book->isbn,
        sizeof(book->isbn),
        "%s",
        isbn
    );

    curl = curl_easy_init();

    if (curl == NULL)
        return -1;

    snprintf(
        url,
        sizeof(url),
        "https://openlibrary.org/isbn/%s.json",
        isbn
    );

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "BrotherPowerTools/1.0"
    );
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    );
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );
    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        fprintf(
            stderr,
            "Open Library request failed: %s\n",
            curl_easy_strerror(result)
        );

        free(response.data);
        return -1;
    }

    if (response.data == NULL)
        return -1;

    root = json_tokener_parse(response.data);

    free(response.data);

    if (root == NULL)
    {
        fprintf(
            stderr,
            "Unable to parse Open Library response.\n"
        );

        return -1;
    }

    /*
     * Prefer the ISBN returned by OpenLibrary.
     */

    if (json_object_object_get_ex(
            root,
            "isbn_13",
            &isbn_13) &&
        json_object_is_type(
            isbn_13,
            json_type_array) &&
        json_object_array_length(isbn_13) > 0)
    {
        struct json_object *value =
            json_object_array_get_idx(isbn_13, 0);

        if (json_object_is_type(
                value,
                json_type_string))
        {
            snprintf(
                book->isbn,
                sizeof(book->isbn),
                "%s",
                json_object_get_string(value)
            );
        }
    }
    else if (json_object_object_get_ex(
                 root,
                 "isbn_10",
                 &isbn_10) &&
             json_object_is_type(
                 isbn_10,
                 json_type_array) &&
             json_object_array_length(isbn_10) > 0)
    {
        struct json_object *value =
            json_object_array_get_idx(isbn_10, 0);

        if (json_object_is_type(
                value,
                json_type_string))
        {
            snprintf(
                book->isbn,
                sizeof(book->isbn),
                "%s",
                json_object_get_string(value)
            );
        }
    }

    copy_json_string(
        book->title,
        sizeof(book->title),
        root,
        "title"
    );

    copy_json_string(
        book->publish_date,
        sizeof(book->publish_date),
        root,
        "publish_date"
    );

    /*
     * Publishers are strings in the current
     * OpenLibrary edition API.
     */

    if (json_object_object_get_ex(
            root,
            "publishers",
            &publishers) &&
        json_object_is_type(
            publishers,
            json_type_array) &&
        json_object_array_length(publishers) > 0)
    {
        struct json_object *publisher =
            json_object_array_get_idx(
                publishers,
                0
            );

        if (json_object_is_type(
                publisher,
                json_type_string))
        {
            snprintf(
                book->publisher,
                sizeof(book->publisher),
                "%s",
                json_object_get_string(publisher)
            );
        }
    }

    /*
     * Subjects are also strings in the current
     * OpenLibrary edition API.
     */

    if (json_object_object_get_ex(
            root,
            "subjects",
            &subjects) &&
        json_object_is_type(
            subjects,
            json_type_array))
    {
        size_t count =
            json_object_array_length(subjects);

        if (count > OPENLIBRARY_MAX_SUBJECTS)
            count = OPENLIBRARY_MAX_SUBJECTS;

        for (size_t i = 0; i < count; i++)
        {
            struct json_object *subject =
                json_object_array_get_idx(
                    subjects,
                    i
                );

            if (!json_object_is_type(
                    subject,
                    json_type_string))
            {
                continue;
            }

            snprintf(
                book->subjects[book->subject_count],
                OPENLIBRARY_MAX_SUBJECT_LENGTH,
                "%s",
                json_object_get_string(subject)
            );

            book->subject_count++;
        }
    }

    /*
     * Authors are represented by author keys.
     * Fetch the first author's record to obtain
     * the actual name.
     */

    if (json_object_object_get_ex(
            root,
            "authors",
            &authors) &&
        json_object_is_type(
            authors,
            json_type_array) &&
        json_object_array_length(authors) > 0)
    {
        struct json_object *author =
            json_object_array_get_idx(
                authors,
                0
            );

        struct json_object *author_key;

        if (json_object_object_get_ex(
                author,
                "key",
                &author_key) &&
            json_object_is_type(
                author_key,
                json_type_string))
        {
            openlibrary_get_author_name(
                json_object_get_string(author_key),
                book->author,
                sizeof(book->author)
            );
        }
    }

    json_object_put(root);

    /*
     * A successful API response without a title
     * isn't useful as a book record.
     */

    if (book->title[0] == '\0')
        return 1;

    return 0;
}