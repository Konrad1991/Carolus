#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "map/map.h"
#include "flood_fill/flood_fill.h"

#define TILE_IMPASSABLE INT_MAX

static int tile_base_cost(const Tile *tile) {
  if (!map_tile_walkable(tile)) return TILE_IMPASSABLE;
  switch (tile->type) {
    case TILE_GRASS: return 2;
    case TILE_ROAD:  return 1;
    case TILE_MANSUSYARD: return 3;
    case TILE_SOIL: return 2;
    case TILE_WATER: default: return TILE_IMPASSABLE;
  }
}

int tile_cost(const Tile *tile) {
  int base = tile_base_cost(tile);
  if (base == TILE_IMPASSABLE) return TILE_IMPASSABLE;
  const float density_cost_weight = 0.005f;
  const int max_density_penalty = 4;
  int density_penalty = (int)(tile->crowd_density * density_cost_weight);
  if (density_penalty > max_density_penalty) density_penalty = max_density_penalty;
  return base + density_penalty;
}

// Flood Field
// --------------------------------------------------------------
typedef int (*TileCostFn)(const Tile *tile);

static int *run_dijkstra(const Map *map, const int *figures, int n_figures, bool allow_diagonal, TileCostFn tile_cost_fn) {
  int n = map->w * map->h;
  int *cost = malloc(sizeof(int) * n);
  bool *processed = calloc(n, sizeof(bool));
  for (int i = 0; i < n; i++) {
    cost[i] = TILE_IMPASSABLE;
  }
  BinaryHeap *heap = heap_create(n);
  for (int f = 0; f < n_figures; f++) {
    cost[figures[f]] = 0;
    heap_insert(heap, figures[f], 0);
  }

  while (heap->size > 0) {
    HeapEntry top = heap_extract_min(heap);
    int index = top.node;
    if (processed[index]) continue;
    processed[index] = true;

    int x;
    int y;
    node_to_xy(index, map->w, &x, &y);
    int neighbors[8];

    const bool is_road = map->tiles[index].type == TILE_ROAD;
    int count = map_get_neighbors(map, index, allow_diagonal && !is_road, neighbors);
    for (int k = 0; k < count; k++) {
      int j = neighbors[k];
      if (processed[j]) continue;

      int jx;
      int jy;
      node_to_xy(j, map->w, &jx, &jy);
      if (map_diagonal_move_blocked(map, x, y, jx, jy)) continue;

      int tcost = tile_cost_fn(&map->tiles[j]);
      if (tcost == TILE_IMPASSABLE) continue;

      int new_cost = cost[index] + tcost;
      if (new_cost < cost[j]) {
        cost[j] = new_cost;
        if (heap_contains(heap, j)) {
          heap_decrease_key(heap, j, new_cost);
        } else {
          heap_insert(heap, j, new_cost);
        }
      }
    }
  }
  heap_free(heap);
  free(processed);
  return cost;
}

FloodField flood_fill(const Map *map, const int* figures, int n_figures, bool allow_diagonal) {
  FloodField result = {0};
  result.cost = run_dijkstra(map, figures, n_figures, allow_diagonal, tile_cost);
  result.cost_no_density = run_dijkstra(map, figures, n_figures, allow_diagonal, tile_base_cost);
  result.len = map->w * map->h;
  return result;
}

void flood_fill_free(FloodField *result) {
  if (result->cost) free(result->cost);
  if (result->cost_no_density) free(result->cost_no_density);
  result->cost = NULL;
  result->cost_no_density = NULL;
  result->len = 0;
}

static int best_neighbor_from_cost_field(const Map *map, const int *cost_field, int node, int target_node, bool allow_diagonal, int exclude_node) {
  int x;
  int y;
  node_to_xy(node, map->w, &x, &y);
  int target_x;
  int target_y;
  node_to_xy(target_node, map->w, &target_x, &target_y);
  int neighbors[8];
  const bool is_road = map->tiles[node].type == TILE_ROAD;
  int count = map_get_neighbors(map, node, allow_diagonal && !is_road, neighbors);
  int best = -1;
  int best_cost = cost_field[node];
  int best_dist_to_target = (x - target_x) * (x - target_x) + (y - target_y) * (y - target_y);
  for (int k = 0; k < count; k++) {
    int j = neighbors[k];
    if (j == exclude_node) continue;
    if (cost_field[j] == TILE_IMPASSABLE) continue;
    int jx;
    int jy;
    node_to_xy(j, map->w, &jx, &jy);
    if (map_diagonal_move_blocked(map, x, y, jx, jy)) continue;
    if (cost_field[j] > best_cost) continue;

    int dist_to_target = (jx - target_x) * (jx - target_x) + (jy - target_y) * (jy - target_y);
    bool strictly_cheaper = cost_field[j] < best_cost;
    bool tied_but_closer = cost_field[j] == best_cost && dist_to_target < best_dist_to_target;
    if (strictly_cheaper || tied_but_closer) {
      best_cost = cost_field[j];
      best = j;
      best_dist_to_target = dist_to_target;
    }
  }
  return best;
}

int flood_field_best_neighbor(const Map *map, FloodField *field, int node, int target_node, bool allow_diagonal, int exclude_node) {
  return best_neighbor_from_cost_field(map, field->cost, node, target_node, allow_diagonal, exclude_node);
}

int flood_field_best_neighbor_ignoring_density(const Map *map, FloodField *field, int node, int target_node, bool allow_diagonal, int exclude_node) {
  return best_neighbor_from_cost_field(map, field->cost_no_density, node, target_node, allow_diagonal, exclude_node);
}

#define CROWD_DENSITY_RADIUS 2

void update_crowd_density(Map *map, ObjectArray *objects) {
  int n = map->w * map->h;
  for (int i = 0; i < n; i++) {
    map->tiles[i].crowd_density = 0.0f;
  }
  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_FIGURE) continue;
    for (int dy = -CROWD_DENSITY_RADIUS; dy <= CROWD_DENSITY_RADIUS; dy++) {
      for (int dx = -CROWD_DENSITY_RADIUS; dx <= CROWD_DENSITY_RADIUS; dx++) {
        Tile *t = map_tile(map, o->tx + dx, o->ty + dy);
        if (!t) continue;
        float dist = sqrtf((float)(dx * dx + dy * dy));
        t->crowd_density += 1.0f / (1.0f + dist);
      }
    }
  }
}

void refresh_flood_fields(Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state) {
  for (int idx = 0; idx < flood_field_state->count; idx++) {
    FloodField *f = &flood_field_state->data[idx];
    if (!f->active) continue;

    int *targets = malloc(objects->count * sizeof(int));
    int n_targets = 0;
    for (int i = 0; i < objects->count; i++) {
      Object *o = &objects->data[i];
      if (o->kind != OBJECT_FIGURE || o->figure.flood_field_idx != idx) continue;
      targets[n_targets++] = node_index(o->figure.target_tx, o->figure.target_ty, map->w);
    }
    if (n_targets == 0) {
      free(targets);
      continue;
    }

    FloodField fresh = flood_fill(map, targets, n_targets, true);
    free(targets);

    flood_fill_free(f);
    f->cost = fresh.cost;
    f->cost_no_density = fresh.cost_no_density;
    f->len = fresh.len;
  }
}
