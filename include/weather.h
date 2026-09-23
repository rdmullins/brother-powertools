#ifndef WEATHER_H
#define WEATHER_H

#include <time.h>

typedef enum {
    WEATHER_SUNNY,
    WEATHER_CLOUDY,
    WEATHER_RAINY,
    WEATHER_SNOWY
} WeatherCondition;

typedef struct {
    int valid;

    char city[64];
    char state[32];
    char country[8];

    double latitude;
    double longitude;

    int condition_id;
    WeatherCondition condition;
    char description[64];

    int temperature;
    int feels_like;
    int humidity;
    int pressure;
    int visibility;

    double wind_speed;
    int wind_direction;

    time_t retrieved;
} WeatherData;

#define WEATHER_FORECAST_DAYS 5

typedef struct {
    int valid;

    char day[11];
    WeatherCondition condition;
    char description[64];

    int high;
    int low;
    int precipitation_chance;
} ForecastDay;

typedef struct {
    int valid;
    char city[64];
    char state[32];

    ForecastDay days[WEATHER_FORECAST_DAYS];
    int count;
} WeatherForecast;

int weather_get_current(WeatherData *weather);

int weather_get_forecast(WeatherForecast *forecast);

int weather_set_location(const char *city,
                         const char *state,
                         const char *country);

#endif