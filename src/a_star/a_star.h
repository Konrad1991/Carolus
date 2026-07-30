#ifndef A_STAR_H
#define A_STAR_H

#include "types.h"
#include "utils/utils.h"
#include "map/map.h"

int node_index(int x, int y, int w);
void node_to_xy(int node, int w, int *x, int *y);

PathResult a_star(Map *map, int start, int end, bool allow_diagonal);
void path_result_free(PathResult *result);

#endif
