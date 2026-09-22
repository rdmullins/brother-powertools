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

int weather_get_current(WeatherData *weather);

#endif