#ifndef WEATHER_H
#define WEATHER_H

#include "types.h"

void init_weather_state(WeatherState *weather_state, SeasonState season_state);

void draw_weather_bar(WeatherState *weather_state);

void update_weather_state(WeatherState *weather_state, SeasonState season_state);

#endif
