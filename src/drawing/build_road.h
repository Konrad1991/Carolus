#ifndef BUILD_ROAD_H
#define BUILD_ROAD_H

#include "types.h"
#include "map/map.h"

void update_road_drag(RoadDragState *drag, TileState tile_state, ModeState *mode_state,
                       Map *map, ObjectArray *objects, const Texture_State *texture_state,
                       bool icon_hovered);

#endif
