#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "weather.h"

static char weather_city[64] = "Lexington";
static char weather_state[32] = "KY";
static char weather_country[8] = "US";

#define GEOCODE_URL "https://api.openweathermap.org/geo/1.0/direct"
#define WEATHER_URL "https://api.openweathermap.org/data/2.5/weather"
#define FORECAST_URL "https://api.openweathermap.org/data/2.5/forecast"

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

static int geocode_location(const char *city,
                            const char *state,
                            const char *country,
                            double *latitude,
                            double *longitude);

int weather_set_location(const char *city,
                         const char *state,
                         const char *country)
{
    double latitude;
    double longitude;

    if (city == NULL || state == NULL || country == NULL)
        return 0;

    if (city[0] == '\0' ||
        state[0] == '\0' ||
        country[0] == '\0')
        return 0;

    /*
     * Make sure OpenWeather can actually find this location
     * before changing our current location.
     */
    if (!geocode_location(city,
                          state,
                          country,
                          &latitude,
                          &longitude)) {

        return 0;
    }

    /*
     * Location is valid, so now commit it.
     */
    snprintf(weather_city,
             sizeof(weather_city),
             "%s",
             city);

    snprintf(weather_state,
             sizeof(weather_state),
             "%s",
             state);

    snprintf(weather_country,
             sizeof(weather_country),
             "%s",
             country);

    return 1;
}

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

static int geocode_location(const char *city,
                            const char *state,
                            const char *country,
                            double *latitude,
                            double *longitude)
{
    const char *api_key;
    CURL *curl;
    char url[512];
    ResponseBuffer response = {0};

    struct json_object *root;
    struct json_object *item;
    struct json_object *value;

    if (city == NULL || state == NULL || country == NULL ||
        latitude == NULL || longitude == NULL)
        return 0;

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

    snprintf(
        url,
        sizeof(url),
        "%s?q=%s,%s,%s&limit=1&appid=%s",
        GEOCODE_URL,
        city,
        state,
        country,
        api_key
    );

    if (!get_json(curl, url, &response)) {
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

        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    item = json_object_array_get_idx(root, 0);

    if (!json_object_object_get_ex(item, "lat", &value)) {
        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    *latitude = json_object_get_double(value);

    if (!json_object_object_get_ex(item, "lon", &value)) {
        json_object_put(root);
        curl_easy_cleanup(curl);
        return 0;
    }

    *longitude = json_object_get_double(value);

    json_object_put(root);
    curl_easy_cleanup(curl);

    return 1;
}

int weather_get_current(WeatherData *weather)
{
    const char *api_key;

    CURL *curl;

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

if (!geocode_location(weather_city,
                      weather_state,
                      weather_country,
                      &latitude,
                      &longitude)) {

    fprintf(stderr, "Weather: unable to find %s, %s, %s.\n",
            weather_city,
            weather_state,
            weather_country);

    curl_easy_cleanup(curl);
    return 0;
}

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

    snprintf(weather->city, sizeof(weather->city), "%s", weather_city);
    snprintf(weather->state, sizeof(weather->state), "%s", weather_state);
    snprintf(weather->country, sizeof(weather->country), "%s",
             weather_country);

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



int weather_get_forecast(WeatherForecast *forecast)
{
    const char *api_key;
    double latitude;
    double longitude;
    CURL *curl;
    ResponseBuffer response = {0};
    char weather_url[512];

    json_object *root;
    json_object *list;
    json_object *city;

    if (forecast == NULL)
        return 0;

    memset(forecast, 0, sizeof(*forecast));

    api_key = getenv("OPENWEATHER_API_KEY");

    if (api_key == NULL || api_key[0] == '\0')
        return 0;

    if (!geocode_location(weather_city,
                          weather_state,
                          weather_country,
                          &latitude,
                          &longitude)) {
        return 0;
    }

    curl = curl_easy_init();

    if (curl == NULL)
        return 0;

    snprintf(weather_url,
             sizeof(weather_url),
             "%s?lat=%.6f&lon=%.6f&units=imperial&appid=%s",
             FORECAST_URL,
             latitude,
             longitude,
             api_key);

    if (!get_json(curl, weather_url, &response)) {
        curl_easy_cleanup(curl);
        free(response.data);
        return 0;
    }

    root = json_tokener_parse(response.data);

    if (root == NULL) {
        curl_easy_cleanup(curl);
        free(response.data);
        return 0;
    }

    if (!json_object_object_get_ex(root, "list", &list)) {
        json_object_put(root);
        curl_easy_cleanup(curl);
        free(response.data);
        return 0;
    }

    if (!json_object_object_get_ex(root, "city", &city)) {
        json_object_put(root);
        curl_easy_cleanup(curl);
        free(response.data);
        return 0;
    }

    snprintf(forecast->city,
             sizeof(forecast->city),
             "%s",
             weather_city);

    snprintf(forecast->state,
             sizeof(forecast->state),
             "%s",
             weather_state);

    int count = json_object_array_length(list);

    for (int i = 0; i < count; i++) {

        json_object *entry;
        json_object *main_data;
        json_object *weather_array;
        json_object *weather_data;
        json_object *dt_txt;
        json_object *temp;
        json_object *pop;

        const char *date_time;
        char date[11];
        char time[6];

        entry = json_object_array_get_idx(list, i);

        if (entry == NULL)
            continue;

        if (!json_object_object_get_ex(entry, "dt_txt", &dt_txt))
            continue;

        if (!json_object_object_get_ex(entry, "main", &main_data))
            continue;

        if (!json_object_object_get_ex(entry, "weather", &weather_array))
            continue;

        if (json_object_array_length(weather_array) == 0)
            continue;

        weather_data = json_object_array_get_idx(weather_array, 0);

        if (!json_object_object_get_ex(main_data, "temp", &temp))
            continue;

        if (!json_object_object_get_ex(entry, "pop", &pop))
            continue;

        date_time = json_object_get_string(dt_txt);

        /*
         * dt_txt format:
         *
         * 2026-09-23 12:00:00
         *
         * Extract the date and time portions.
         */
        snprintf(date, sizeof(date), "%.10s", date_time);
        snprintf(time, sizeof(time), "%.5s", date_time + 11);

        /*
         * Find an existing day.
         */
        int day_index = -1;

        for (int d = 0; d < forecast->count; d++) {
            if (strcmp(forecast->days[d].day, date) == 0) {
                day_index = d;
                break;
            }
        }

        /*
         * Create a new day if necessary.
         */
        if (day_index == -1) {

            if (forecast->count >= WEATHER_FORECAST_DAYS)
                continue;

            day_index = forecast->count;
            forecast->count++;

            memset(&forecast->days[day_index],
                   0,
                   sizeof(ForecastDay));

            snprintf(forecast->days[day_index].day,
                     sizeof(forecast->days[day_index].day),
                     "%s",
                     date);

            forecast->days[day_index].high =
                (int)json_object_get_double(temp);

            forecast->days[day_index].low =
                (int)json_object_get_double(temp);

            forecast->days[day_index].precipitation_chance =
                (int)(json_object_get_double(pop) * 100.0);

            json_object *condition_id;

            if (json_object_object_get_ex(weather_data,
                                          "id",
                                          &condition_id)) {

                int id = json_object_get_int(condition_id);

                forecast->days[day_index].condition =
                    weather_condition_from_id(id);
            }

            json_object *description;

            if (json_object_object_get_ex(weather_data,
                                          "description",
                                          &description)) {

                snprintf(forecast->days[day_index].description,
                         sizeof(forecast->days[day_index].description),
                         "%s",
                         json_object_get_string(description));
            }

            /*
             * Use the first forecast's condition for now.
             * We'll improve this to prefer the daytime
             * condition once the basic version is working.
             */

            forecast->days[day_index].valid = 1;
        }

        /*
         * Update daily high and low.
         */
        int temperature =
            (int)json_object_get_double(temp);

        if (temperature > forecast->days[day_index].high)
            forecast->days[day_index].high = temperature;

        if (temperature < forecast->days[day_index].low)
            forecast->days[day_index].low = temperature;

        /*
         * Keep the highest precipitation probability
         * for the day.
         */
        int precipitation =
            (int)(json_object_get_double(pop) * 100.0);

        if (precipitation >
            forecast->days[day_index].precipitation_chance) {

            forecast->days[day_index].precipitation_chance =
                precipitation;
        }
    }

    forecast->valid = (forecast->count > 0);

    json_object_put(root);
    curl_easy_cleanup(curl);
    free(response.data);

    return forecast->valid;
}