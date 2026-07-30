#include <stdlib.h>
#include "map.h"

void map_init(Map *map, int w, int h) {
  map->w = w;
  map->h = h;
  map->tiles = calloc(w * h, sizeof(Tile));
}

void free_map(Map *map) {
  free(map->tiles);
}

Tile *map_tile(Map *map, int x, int y) {
  if (x < 0 || x >= map->w || y < 0 || y >= map->h) return NULL;
  return &map->tiles[y * map->w + x];
}

bool map_is_placeable(Map *map, int tx, int ty, int fw, int fh) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      if (fx < 0 || fx >= map->w || fy < 0 || fy >= map->h) return false;
      const Tile *t = &map->tiles[fy * map->w + fx];
      if (t->occupied || t->figure_occupied || t->type == TILE_ROAD || t->type == TILE_WATER) return false;
    }
  }
  return true;
}

bool map_building_is_placeable(Map *map, int tx, int ty, int fw, int fh) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      if (fx < 0 || fx >= map->w || fy < 0 || fy >= map->h) return false;
      const Tile *t = &map->tiles[fy * map->w + fx];
      if (t->occupied || t->figure_occupied || t->type != TILE_MANSUSYARD) return false;
    }
  }
  return true;
}

void map_place_object(Map *map, int tx, int ty, int fw, int fh, bool build_road) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      Tile *t = map_tile(map, fx, fy);
      if (t) {
        t->occupied = 1;
        if (build_road) {
          t->type = TILE_ROAD;
          t->occupied = 0;
        }
      }
    }
  }
}

void map_place_figure(Map *map, int tx, int ty) {
  Tile *t = map_tile(map, tx, ty);
  if (t) t->figure_occupied = 1;
}

void map_clear_object(Map *map, const Object *o) {
  for (int fy = o->ty - o->footprint_h + 1; fy <= o->ty; fy++) {
    for (int fx = o->tx - o->footprint_w + 1; fx <= o->tx; fx++) {
      Tile *t = map_tile(map, fx, fy);
      if (!t) continue;
      if (o->kind == OBJECT_BRIDGE) {
        t->type = TILE_WATER;
      } else {
        t->occupied = 0;
        if (o->kind == OBJECT_FIGURE) t->figure_occupied = 0;
        if (o->kind == OBJECT_MANSUS) {
          t->type = TILE_GRASS;
        } else if (o->kind != OBJECT_FIGURE && t->type != TILE_MANSUSYARD) {
          t->type = TILE_GRASS;
        }
      }
    }
  }
}

bool map_bridge_is_placeable(Map *map, int tx, int ty, int fw, int fh) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      if (fx < 0 || fx >= map->w || fy < 0 || fy >= map->h) return false;
      const Tile *t = &map->tiles[fy * map->w + fx];
      if (t->occupied || t->figure_occupied) return false;
      if (t->type == TILE_WATER) return true;
    }
  }
  return false;
}

bool map_figure_is_placeable(Map *map, int tx, int ty, int fw, int fh) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      if (fx < 0 || fx >= map->w || fy < 0 || fy >= map->h) return false;
      const Tile *t = &map->tiles[fy * map->w + fx];
      if (t->occupied || t->figure_occupied || t->type == TILE_WATER) return false;
    }
  }
  return true;
}

bool map_mansus_is_placeable(Map *map, int tx, int ty, int fw, int fh) {
  for (int fy = ty - fh + 1; fy <= ty; fy++) {
    for (int fx = tx - fw + 1; fx <= tx; fx++) {
      if (fx < 0 || fx >= map->w || fy < 0 || fy >= map->h) return false;
      const Tile *t = &map->tiles[fy * map->w + fx];
      if (t->occupied || t->figure_occupied || t->type == TILE_ROAD || t->type == TILE_WATER ||
          t->type == TILE_MANSUSYARD) return false;
    }
  }
  return true;
}

bool map_tile_walkable(const Tile *tile) {
  return !tile->occupied && tile->type != TILE_WATER;
}

bool map_tile_free(const Tile *tile) {
  return map_tile_walkable(tile) && !tile->figure_occupied;
}

int map_free_tiles_near(Map *map, int bx, int by, int n, int *out_tx, int *out_ty) {
  int found = 0;
  int max_d = map->w + map->h;
  for (int d = 0; d <= max_d && found < n; d++) {
    int x0 = bx - d, x1 = bx + d;
    int y0 = by - d, y1 = by + d;
    for (int y = y0; y <= y1 && found < n; y++) {
      for (int x = x0; x <= x1 && found < n; x++) {
        if (d > 0 && x != x0 && x != x1 && y != y0 && y != y1) continue;
        const Tile *t = map_tile(map, x, y);
        if (!t || !map_tile_free(t)) continue;
        out_tx[found] = x;
        out_ty[found] = y;
        found++;
      }
    }
  }
  return found;
}
