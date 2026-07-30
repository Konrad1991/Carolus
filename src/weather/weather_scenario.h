#ifndef WEATHER_SCENARIO_H
#define WEATHER_SCENARIO_H

#include "types.h"
#include "map/map.h"

void update_weather_scenario(WeatherState *weather_state, WeatherScenarioState *scenario_state, Map *map, const GameState *game_state);

void cycle_weather_scenario(WeatherScenarioState *scenario_state);

void draw_weather_scenario_bar(WeatherScenarioState *state);

void update_weather_scenario_state(WeatherScenarioState *state);

#endif
