#ifndef BUILD_CLEAR_FOREST_H
#define BUILD_CLEAR_FOREST_H

#include "types.h"
#include "map/map.h"
#include "selection/selection.h"

void update_clear_forest_drag(ClearForestDragState *drag, TileState tile_state, ModeState *mode_state,
                              Map *map, FloodFieldArray *flood_field_state, ObjectArray *objects,
                              GameState *game_state, Selection *selection, bool icon_hovered);

#endif
