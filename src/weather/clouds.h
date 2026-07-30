#ifndef CLOUDS_H
#define CLOUDS_H

#include "types.h"

void init_clouds(CloudState *cloud_state);
void update_clouds(CloudState *cloud_state, const WeatherState *weather_state);
void draw_clouds(const CloudState *cloud_state, const Texture_State *texture_state, const WeatherState *weather_state);

#endif
