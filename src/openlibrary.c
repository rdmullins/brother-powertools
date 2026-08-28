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

int openlibrary_lookup_isbn(const char *isbn,
                            OpenLibraryBook *book)
{
    CURL *curl;
    CURLcode result;
    ResponseBuffer response = { NULL, 0 };
    struct json_object *root;
    struct json_object *edition;
    char url[OPENLIBRARY_URL_MAX];

    if (isbn == NULL || book == NULL) {
        return -1;
    }

    memset(book, 0, sizeof(*book));

    snprintf(book->isbn,
             sizeof(book->isbn),
             "%s",
             isbn);

    curl = curl_easy_init();

    if (curl == NULL) {
        return -1;
    }

    snprintf(url,
             sizeof(url),
             "https://openlibrary.org/api/books?bibkeys=ISBN:%s&jscmd=data&format=json",
             isbn);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl,
                     CURLOPT_USERAGENT,
                     "BrotherPowerTools/1.0");
    curl_easy_setopt(curl,
                     CURLOPT_WRITEFUNCTION,
                     write_callback);
    curl_easy_setopt(curl,
                     CURLOPT_WRITEDATA,
                     &response);
    curl_easy_setopt(curl,
                     CURLOPT_FOLLOWLOCATION,
                     1L);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr,
                "Open Library request failed: %s\n",
                curl_easy_strerror(result));

        curl_easy_cleanup(curl);
        free(response.data);

        return -1;
    }

    curl_easy_cleanup(curl);

    if (response.data == NULL) {
        return -1;
    }

    root = json_tokener_parse(response.data);

    free(response.data);

    if (root == NULL) {
        fprintf(stderr,
                "Unable to parse Open Library response.\n");
        return -1;
    }

    {
        char key[OPENLIBRARY_URL_MAX];

        snprintf(key,
                 sizeof(key),
                 "ISBN:%s",
                 isbn);

        if (!json_object_object_get_ex(root,
                                       key,
                                       &edition)) {
            json_object_put(root);
            return 1;
        }
    }

    copy_json_string(book->title,
                     sizeof(book->title),
                     edition,
                     "title");

    copy_json_string(book->publish_date,
                     sizeof(book->publish_date),
                     edition,
                     "publish_date");

    {
        struct json_object *authors;

        if (json_object_object_get_ex(edition,
                                       "authors",
                                       &authors) &&
            json_object_is_type(authors,
                                json_type_array) &&
            json_object_array_length(authors) > 0) {

            struct json_object *author =
                json_object_array_get_idx(authors, 0);

            copy_json_string(book->author,
                             sizeof(book->author),
                             author,
                             "name");
        }
    }

    {
        struct json_object *publishers;

        if (json_object_object_get_ex(edition,
                                       "publishers",
                                       &publishers) &&
            json_object_is_type(publishers,
                                json_type_array) &&
            json_object_array_length(publishers) > 0) {

            struct json_object *publisher =
                json_object_array_get_idx(publishers, 0);

            copy_json_string(book->publisher,
                             sizeof(book->publisher),
                             publisher,
                             "name");
        }
    }

    {
        struct json_object *subjects;

        if (json_object_object_get_ex(edition,
                                       "subjects",
                                       &subjects) &&
            json_object_is_type(subjects,
                                json_type_array)) {

            size_t count =
                json_object_array_length(subjects);

            if (count > OPENLIBRARY_MAX_SUBJECTS) {
                count = OPENLIBRARY_MAX_SUBJECTS;
            }

            for (size_t i = 0; i < count; i++) {
                struct json_object *subject;
                struct json_object *name;

                subject =
                    json_object_array_get_idx(subjects, i);

                if (!json_object_object_get_ex(
                        subject,
                        "name",
                        &name)) {
                    continue;
                }

                if (!json_object_is_type(name,
                                         json_type_string)) {
                    continue;
                }

                strncpy(book->subjects[book->subject_count],
                        json_object_get_string(name),
                        OPENLIBRARY_MAX_SUBJECT_LENGTH - 1);

                book->subjects[book->subject_count]
                              [OPENLIBRARY_MAX_SUBJECT_LENGTH - 1]
                    = '\0';

                book->subject_count++;
            }
        }
    }

    json_object_put(root);

    return 0;
}