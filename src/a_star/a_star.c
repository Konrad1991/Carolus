#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "map/map.h"
#include "a_star/a_star.h"

#define TILE_IMPASSABLE INT_MAX

static int tile_cost(const Tile *tile) {
  if (!map_tile_walkable(tile)) return TILE_IMPASSABLE;
  switch (tile->type) {
    case TILE_GRASS: return 2;
    case TILE_ROAD:  return 1;
    case TILE_MANSUSYARD: return 3;
    case TILE_SOIL: return 2;
    case TILE_WATER: return TILE_IMPASSABLE;
  }
  return TILE_IMPASSABLE;
}

/* Chebyshev distance - with diagonal movement allowed, this is the minimum
 * number of steps to reach the target, so it stays admissible as long as
 * every step costs at least 1 (true here since the cheapest tile, Road,
 * costs 1). Manhattan distance would overestimate once diagonals are free
 * to skip two orthogonal steps for one. */
static int heuristic(int node, int end, int w) {
  int x1, y1, x2, y2;
  node_to_xy(node, w, &x1, &y1);
  node_to_xy(end, w, &x2, &y2);
  int dx = abs(x1 - x2);
  int dy = abs(y1 - y2);
  return dx > dy ? dx : dy;
}

/* --- A* --- */

void path_result_free(PathResult *result) {
  free(result->solution);
  result->solution = NULL;
  result->solution_len = 0;
}

PathResult a_star(const Map *map, int start, int end, bool allow_diagonal) {
  int n = map->w * map->h;
  int *pi = malloc(sizeof(int) * n);
  int *prev = malloc(sizeof(int) * n);
  bool *processed = calloc(n, sizeof(bool));
  for (int i = 0; i < n; i++) {
    pi[i] = TILE_IMPASSABLE;
    prev[i] = -1;
  }

  pi[start] = 0;
  BinaryHeap *heap = heap_create(n);
  heap_insert(heap, start, heuristic(start, end, map->w));

  while (heap->size > 0) {
    HeapEntry top = heap_extract_min(heap);
    int index = top.node;
    if (processed[index]) continue;
    processed[index] = true;

    int x, y;
    node_to_xy(index, map->w, &x, &y);
    int neighbors[8];

    const bool is_road = map->tiles[index].type == TILE_ROAD;
    int count = map_get_neighbors(map, index, allow_diagonal && !is_road, neighbors);
    for (int k = 0; k < count; k++) {
      int j = neighbors[k];
      if (processed[j]) continue;

      int jx, jy;
      node_to_xy(j, map->w, &jx, &jy);
      if (map_diagonal_move_blocked(map, x, y, jx, jy)) continue;

      int cost = tile_cost(&map->tiles[j]);
      if (cost == TILE_IMPASSABLE) continue;

      int new_cost = pi[index] + cost;
      if (new_cost < pi[j]) {
        pi[j] = new_cost;
        prev[j] = index;
        int f = new_cost + heuristic(j, end, map->w);
        if (heap_contains(heap, j)) {
          heap_decrease_key(heap, j, f);
        } else {
          heap_insert(heap, j, f);
        }
      }
    }
  }
  heap_free(heap);

  PathResult result = {0};
  if (pi[end] == TILE_IMPASSABLE) {
    result.solution = NULL;
    result.solution_len = 0;
    result.cost = -1;
    free(pi);
    free(prev);
    free(processed);
    return result;
  }

  int total_cost = pi[end];
  int *tmp = malloc(sizeof(int) * n);
  int len = 0;
  int cur = end;
  while (cur != start) {
    tmp[len++] = cur;
    cur = prev[cur];
  }
  tmp[len++] = start;

  int *solution = malloc(sizeof(int) * len);
  for (int i = 0; i < len; i++) solution[i] = tmp[len - 1 - i];
  free(tmp);

  free(pi);
  free(prev);
  free(processed);

  result.solution = solution;
  result.solution_len = len;
  result.cost = total_cost;
  return result;
}
