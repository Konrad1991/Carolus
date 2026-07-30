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

void draw_cursor(Texture_State *texture_state, const TileState* tile_state,
                 const ObjectArray* objects, const Selection* sel, Map *map);

#endif
