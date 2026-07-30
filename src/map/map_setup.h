#ifndef MAP_SETUP_H
#define MAP_SETUP_H

#include "types.h"
#include "map/map.h"

void scatter_trees(ObjectArray *objects, Map *map, Texture_State *texture_state);

void scatter_grass_tufts(ObjectArray *objects, Map *map, Texture_State *texture_state);

void build_tree_wall(ObjectArray *objects, Map *map, Texture_State *texture_state, int wall_offset, int gap_center_y, int gap_half_width);

void spawn_debug_figures(ObjectArray *objects, Map *map, Texture_State *texture_state, int bx, int by, int n);

void define_nature(ObjectArray* objects, Map *map, TileState tile_state, Texture_State* texture_state);

#endif
