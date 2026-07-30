#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "map/map.h"
#include "flood_fill/flood_fill.h"

#define TILE_IMPASSABLE INT_MAX

static int get_neighbors(int node, int w, int h, bool allow_diagonal, int out[8]) {
  int x, y;
  node_to_xy(node, w, &x, &y);
  int dx[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
  int dy[8] = {0, 0, -1, 1, -1, 1, -1, 1};
  int n = allow_diagonal ? 8 : 4;
  int count = 0;
  for (int k = 0; k < n; k++) {
    int nx = x + dx[k];
    int ny = y + dy[k];
    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
      out[count++] = node_index(nx, ny, w);
    }
  }
  return count;
}

static int tile_cost(const Tile *tile) {
  if (!map_tile_walkable(tile)) return TILE_IMPASSABLE;
  int base;
  switch (tile->type) {
    case TILE_GRASS: base = 2; break;
    case TILE_ROAD:  base = 1; break;
    case TILE_MANSUSYARD: base = 3; break;
    case TILE_SOIL: base = 2; break;
    case TILE_WATER: default: return TILE_IMPASSABLE;
  }
  const float density_cost_weight = 0.75f;
  const int max_density_penalty = 4;
  int density_penalty = (int)(tile->crowd_density * density_cost_weight);
  if (density_penalty > max_density_penalty) density_penalty = max_density_penalty;
  return base + density_penalty;
}

// Flood Field
// --------------------------------------------------------------
FloodField flood_fill(Map *map, int* figures, int n_figures, bool allow_diagonal) {
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
    int count = get_neighbors(index, map->w, map->h, allow_diagonal && !is_road, neighbors);
    for (int k = 0; k < count; k++) {
      int j = neighbors[k];
      if (processed[j]) continue;

      int jx;
      int jy;
      node_to_xy(j, map->w, &jx, &jy);
      bool diagonal = jx != x && jy != y;
      if (diagonal) {
        const Tile *flank_a = &map->tiles[node_index(x, jy, map->w)];
        const Tile *flank_b = &map->tiles[node_index(jx, y, map->w)];
        if (!map_tile_walkable(flank_a) || !map_tile_walkable(flank_b)) continue;
      }

      int tcost = tile_cost(&map->tiles[j]);
      if (tcost == TILE_IMPASSABLE) continue;

      int new_cost = cost[index] + tcost;
      if (new_cost < cost[j]) {
        cost[j] = new_cost;
        int f = new_cost;
        if (heap_contains(heap, j)) {
          heap_decrease_key(heap, j, f);
        } else {
          heap_insert(heap, j, f);
        }
      }
    }
  }
  heap_free(heap);

  FloodField result = {0};
  result.cost = cost;
  result.len = n;
  free(processed);
  return result;
}

void flood_fill_free(FloodField *result) {
  if (result->cost) free(result->cost);
  result->cost = NULL;
  result->len = 0;
}


int flood_field_best_neighbor(Map *map, FloodField *field, int node, bool allow_diagonal, int exclude_node) {
  int x;
  int y;
  node_to_xy(node, map->w, &x, &y);
  int neighbors[8];
  const bool is_road = map->tiles[node].type == TILE_ROAD;
  int count = get_neighbors(node, map->w, map->h, allow_diagonal && !is_road, neighbors);
  int best = -1;
  int best_cost = field->cost[node];
  for (int k = 0; k < count; k++) {
    int j = neighbors[k];
    if (j == exclude_node) continue;
    if (field->cost[j] == TILE_IMPASSABLE) continue;
    int jx;
    int jy;
    node_to_xy(j, map->w, &jx, &jy);
    bool diagonal = jx != x && jy != y;
    if (diagonal) {
      const Tile *flank_a = &map->tiles[node_index(x, jy, map->w)];
      const Tile *flank_b = &map->tiles[node_index(jx, y, map->w)];
      if (!map_tile_walkable(flank_a) || !map_tile_walkable(flank_b)) continue;
    }

    if (field->cost[j] < best_cost) {
      best_cost = field->cost[j];
      best = j;
    }
  }
  return best;
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
    f->len = fresh.len;
  }
}
