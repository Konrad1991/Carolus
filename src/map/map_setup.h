#ifndef MAP_SETUP_H
#define MAP_SETUP_H

#include "types.h"
#include "map/map.h"

void scatter_trees(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_grass_tufts(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_rocks(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_moss_ferns(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_mushrooms(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_strawberry(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_cairns(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_woody_debris(ObjectArray *objects, Map *map, const Texture_State *texture_state);

void scatter_swamps(Map *map, int n_patches);

void build_tree_wall(ObjectArray *objects, Map *map, const Texture_State *texture_state, int wall_offset, int gap_center_x, int gap_half_width, int wall_thickness);

void spawn_debug_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, int bx, int by, int n);

void define_nature(ObjectArray* objects, Map *map, TileState tile_state, const Texture_State* texture_state);

#endif
