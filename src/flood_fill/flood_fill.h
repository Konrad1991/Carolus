#ifndef FLOOD_FILL_H
#define FLOOD_FILL_H

#include "types.h"
#include "utils/utils.h"
#include "map/map.h"

FloodField flood_fill(Map *map, int* figures, int n_figures, bool allow_diagonal);
void flood_fill_free(FloodField *result);
int flood_field_best_neighbor(Map *map, FloodField *field, int node, bool allow_diagonal, int exclude_node);
void update_crowd_density(Map *map, ObjectArray *objects);
void refresh_flood_fields(Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state);

#endif
