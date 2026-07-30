#ifndef GAME_TIME_H
#define GAME_TIME_H

#include "types.h"

float game_delta_time(void);

void set_game_time_scale(float scale);
float get_game_time_scale(void);

void draw_game_speed_bar(GameSpeedState *state);
void update_game_speed_state(GameSpeedState *state);

#endif
