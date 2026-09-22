#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "weather.h"

#define WEATHER_CITY    "Lexington"
#define WEATHER_STATE   "KY"
#define WEATHER_COUNTRY "US"

#define GEOCODE_URL "https://api.openweathermap.org/geo/1.0/direct"
#define WEATHER_URL "https://api.openweathermap.org/data/2.5/weather"

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp)
{
    size_t total = size * nmemb;
    ResponseBuffer *response = (ResponseBuffer *)userp;

    char *new_data = realloc(response->data, response->size + total + 1);

    if (new_data == NULL)
        return 0;

    response->data = new_data;

    memcpy(response->data + response->size, contents, total);

    response->size += total;
    response->data[response->size] = '\0';

    return total;
}

static WeatherCondition weather_condition_from_id(int id)
{
    if (id >= 200 && id <= 232)
        return WEATHER_RAINY;

    if (id >= 300 && id <= 321)
        return WEATHER_RAINY;

    if (id >= 500 && id <= 531)
        return WEATHER_RAINY;

    if (id >= 600 && id <= 622)
        return WEATHER_SNOWY;

    if (id == 800)
        return WEATHER_SUNNY;

    if (id >= 801 && id <= 804)
        return WEATHER_CLOUDY;

    return WEATHER_CLOUDY;
}

static int get_json(CURL *curl, const char *url, ResponseBuffer *response)
{
    CURLcode result;

    response->data = NULL;
    response->size = 0;

    curl_easy_reset(curl);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr, "Weather: CURL error: %s\n",
                curl_easy_strerror(result));
        free(response->data);
        response->data = NULL;
        response->size = 0;
        return 0;
    }

    return 1;
}

int weather_get_current(WeatherData *weather)
{
    const char *api_key;

    CURL *curl;

    char geocode_url[512];
    char weather_url[512];

    ResponseBuffer response = {0};

    struct json_object *root;
    struct json_object *item;

    double latitude;
    double longitude;

    if (weather == NULL)
        return 0;

    memset(weather, 0, sizeof(WeatherData));

    api_key = getenv("OPENWEATHER_API_KEY");

    if (api_key == NULL || api_key[0] == '\0') {
        fprintf(stderr, "Weather: OPENWEATHER_API_KEY is not set.\n");
        return 0;
    }

    curl = curl_easy_init();

    if (curl == NULL) {
        fprintf(stderr, "Weather: unable to initialize CURL.\n");
        return 0;
    }

    /*
     * ------------------------------------------------------------
     * Step 1: Geocode the selected city.
     * ------------------------------------------------------------
     */

    snprintf(
        geocode_url,
        sizeof(geocode_url),
        "%s?q=%s,%s,%s&limit=1&appid=%s",
        GEOCODE_URL,
        WEATHER_CITY,
        WEATHER_STATE,
        WEATHER_COUNTRY,
        api_key
    );

    if (!get_json(curl, geocode_url, &response)) {
        curl_easy_cleanup(curl);
        return 0;
    }

    root = json_tokener_parse(response.data);

    free(response.data);
    response.data = NULL;

    if (root == NULL) {
        fprintf(stderr, "Weather: invalid geocoding JSON.\n");
        curl_easy_cleanup(curl);
        return 0;
    }

    if (!json_object_is_type(root, json_type_array) ||
        json_object_array_length(root) == 0) {

        fprintf(stderr, "Weather: location not found.\n");
        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    item = json_object_array_get_idx(root, 0);

    if (!json_object_object_get_ex(item, "lat", &item)) {
        fprintf(stderr, "Weather: geocoding response missing latitude.\n");
        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    latitude = json_object_get_double(item);

    item = json_object_array_get_idx(root, 0);

    if (!json_object_object_get_ex(item, "lon", &item)) {
        fprintf(stderr, "Weather: geocoding response missing longitude.\n");
        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    longitude = json_object_get_double(item);

    json_object_put(root);

    /*
     * ------------------------------------------------------------
     * Step 2: Get current weather using the coordinates.
     * ------------------------------------------------------------
     */

    snprintf(
        weather_url,
        sizeof(weather_url),
        "%s?lat=%.6f&lon=%.6f&units=imperial&appid=%s",
        WEATHER_URL,
        latitude,
        longitude,
        api_key
    );

    if (!get_json(curl, weather_url, &response)) {
        curl_easy_cleanup(curl);
        return 0;
    }

 //fprintf(stderr, "\nWEATHER API RESPONSE:\n%s\n\n", response.data);

root = json_tokener_parse(response.data);

free(response.data);

if (root == NULL) {
        fprintf(stderr, "Weather: invalid weather JSON.\n");
        curl_easy_cleanup(curl);
        return 0;
    }

    /*
     * ------------------------------------------------------------
     * Location
     * ------------------------------------------------------------
     */

    snprintf(weather->city, sizeof(weather->city), "%s", WEATHER_CITY);
    snprintf(weather->state, sizeof(weather->state), "%s", WEATHER_STATE);
    snprintf(weather->country, sizeof(weather->country), "%s",
             WEATHER_COUNTRY);

    weather->latitude = latitude;
    weather->longitude = longitude;

    /*
     * ------------------------------------------------------------
     * Main weather information
     * ------------------------------------------------------------
     */

    if (json_object_object_get_ex(root, "weather", &item) &&
        json_object_is_type(item, json_type_array) &&
        json_object_array_length(item) > 0) {

        struct json_object *weather_item;
        struct json_object *value;

        weather_item = json_object_array_get_idx(item, 0);

        if (json_object_object_get_ex(weather_item, "id", &value))
            weather->condition_id = json_object_get_int(value);

        if (json_object_object_get_ex(weather_item, "description", &value)) {
            snprintf(weather->description,
                     sizeof(weather->description),
                     "%s",
                     json_object_get_string(value));
        }

        weather->condition =
            weather_condition_from_id(weather->condition_id);
    }

    /*
     * ------------------------------------------------------------
     * Temperature / humidity / pressure
     * ------------------------------------------------------------
     */

    if (json_object_object_get_ex(root, "main", &item)) {
        struct json_object *value;

        if (json_object_object_get_ex(item, "temp", &value))
            weather->temperature =
                (int)(json_object_get_double(value) + 0.5);

        if (json_object_object_get_ex(item, "feels_like", &value))
            weather->feels_like =
                (int)(json_object_get_double(value) + 0.5);

        if (json_object_object_get_ex(item, "humidity", &value))
            weather->humidity = json_object_get_int(value);

        if (json_object_object_get_ex(item, "pressure", &value))
            weather->pressure = json_object_get_int(value);
    }

    /*
     * ------------------------------------------------------------
     * Visibility
     * ------------------------------------------------------------
     */

    if (json_object_object_get_ex(root, "visibility", &item))
        weather->visibility = json_object_get_int(item);

    /*
     * ------------------------------------------------------------
     * Wind
     * ------------------------------------------------------------
     */

    if (json_object_object_get_ex(root, "wind", &item)) {
        struct json_object *value;

        if (json_object_object_get_ex(item, "speed", &value))
            weather->wind_speed = json_object_get_double(value);

        if (json_object_object_get_ex(item, "deg", &value))
            weather->wind_direction = json_object_get_int(value);
    }

    weather->retrieved = time(NULL);
    weather->valid = 1;

    json_object_put(root);
    curl_easy_cleanup(curl);

    return 1;
}