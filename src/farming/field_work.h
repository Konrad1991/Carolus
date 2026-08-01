#ifndef FIELD_WORK_H
#define FIELD_WORK_H

#include "types.h"
#include "map/map.h"

void culitvate_fields(ObjectArray* objects, Map* map, FloodFieldArray* flood_field_state,
          GameState* game_state, SeasonState* season_state);

#endif
