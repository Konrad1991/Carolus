#ifndef UNITS_H
#define UNITS_H

#include "types.h"
#include "map/map.h"
#include "selection/selection.h"

void command_selected_units(const Selection *sel, Map *map, TileState tile_state, ObjectArray *objects, FloodFieldArray* flood_field_state, GameState *game_state, bool any_hovered);

#endif
