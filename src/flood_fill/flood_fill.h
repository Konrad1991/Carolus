#ifndef FLOOD_FILL_H
#define FLOOD_FILL_H

#include "types.h"
#include "utils/utils.h"
#include "map/map.h"

int tile_cost(const Tile *tile);
FloodField flood_fill(const Map *map, const int* figures, int n_figures, bool allow_diagonal);
void flood_fill_free(FloodField *result);
int flood_field_best_neighbor(const Map *map, FloodField *field, int node, int target_node, bool allow_diagonal, int exclude_node);
int flood_field_best_neighbor_ignoring_density(const Map *map, FloodField *field, int node, int target_node, bool allow_diagonal, int exclude_node);
void update_crowd_density(Map *map, ObjectArray *objects);
void refresh_flood_fields(Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state);

#endif
