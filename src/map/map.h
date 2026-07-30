#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include "types.h"

void map_init(Map *map, int w, int h);
void free_map(Map *map);
Tile *map_tile(Map *map, int x, int y);

bool map_is_placeable(const Map *map, int tx, int ty, int fw, int fh);
bool map_building_is_placeable(const Map *map, int tx, int ty, int fw, int fh);
void map_place_object(Map *map, int tx, int ty, int fw, int fh, bool build_road);
void map_place_figure(Map *map, int tx, int ty);
void map_clear_object(Map *map, const Object *o);
bool map_bridge_is_placeable(const Map *map, int tx, int ty, int fw, int fh);
bool map_figure_is_placeable(const Map *map, int tx, int ty, int fw, int fh);
bool map_mansus_is_placeable(const Map *map, int tx, int ty, int fw, int fh);

bool map_tile_walkable(const Tile *tile);
bool map_tile_free(const Tile *tile);
int map_free_tiles_near(Map *map, int bx, int by, int n, int *out_tx, int *out_ty);

int map_get_neighbors(const Map *map, int node, bool allow_diagonal, int out[8]);
bool map_diagonal_move_blocked(const Map *map, int x, int y, int jx, int jy);

#endif
