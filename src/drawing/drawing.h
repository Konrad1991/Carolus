#ifndef DRAWING_H
#define DRAWING_H

#include "raylib.h"
#include "raymath.h"
#include "map/map.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

void draw_scene(const ObjectArray *objects,
                TileState tile_state, Texture_State *texture_state,
                SeasonBlend season_blend, Map *map, int highlight_index,
                const Selection *selection,
                const Object *preview, bool preview_active, Color preview_tint,
                float water_level_z, SoilOverlayState soil_overlay_state);

void draw_cursor(const Texture_State *texture_state, const TileState* tile_state,
                 const ObjectArray* objects, const Selection* sel, Map *map);

void visible_tile_bounds(TileState tile_state, const Map *map,
                         int *min_tx, int *max_tx, int *min_ty, int *max_ty);

#endif
