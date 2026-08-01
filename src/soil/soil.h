#ifndef SOIL_H
#define SOIL_H

#include "types.h"
#include "map/map.h"

#define SOIL_WATER_CAPACITY_L 200.0f
#define SOIL_MINERAL_CAPACITY_G 200.0f

void init_soil(Map *map, const WeatherState *weather_state);

void update_soil(Map *map, const WeatherState *weather_state, const GameState *game_state);

float soil_deplete_minerals(Map *map, int tx, int ty, float amount);

float soil_deplete_water(Map *map, int tx, int ty, float amount);

void soil_enrich_minerals(Map *map, int tx, int ty, float amount);

#endif
