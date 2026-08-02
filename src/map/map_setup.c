#include "map/map_setup.h"
#include "drawing/drawing_helper.h"
#include "containers/arrays.h"
#include "raylib.h"
#include <stdlib.h>
#include <math.h>

static void carve_pond(Map *map, int cx, int cy, float base_radius) {
  int margin = (int)base_radius + 6;
  for (int y = cy - margin; y <= cy + margin; y++) {
    for (int x = cx - margin; x <= cx + margin; x++) {
      Tile *t = map_tile(map, x, y);
      if (!t) continue;
      float dx = (float)(x - cx);
      float dy = (float)(y - cy);
      float dist = sqrtf(dx * dx + dy * dy);
      float angle = atan2f(dy, dx);
      float wobble = sinf(angle * 3.0f + 0.6f) * 2.5f + sinf(angle * 5.0f + 1.7f) * 1.2f;
      if (dist <= base_radius + wobble) {
        t->type = TILE_WATER;
        t->z = -0.5;
      }
    }
  }
}

static void carve_swamp(Map *map, int cx, int cy, float base_radius) {
  int margin = (int)base_radius + 6;
  for (int y = cy - margin; y <= cy + margin; y++) {
    for (int x = cx - margin; x <= cx + margin; x++) {
      Tile *t = map_tile(map, x, y);
      if (!t || t->type != TILE_GRASS) continue;
      float dx = (float)(x - cx);
      float dy = (float)(y - cy);
      float dist = sqrtf(dx * dx + dy * dy);
      float angle = atan2f(dy, dx);
      float wobble = sinf(angle * 3.0f + 0.6f) * 2.5f + sinf(angle * 5.0f + 1.7f) * 1.2f;
      if (dist <= base_radius + wobble) {
        t->type = TILE_SWAMP;
      }
    }
  }
}

void scatter_swamps(Map *map, int n_patches) {
  const float min_radius = 5.0f;
  const float max_radius = 10.0f;
  for (int i = 0; i < n_patches; i++) {
    int cx = GetRandomValue(0, map->w - 1);
    int cy = GetRandomValue(0, map->h - 1);
    float radius = min_radius + GetRandomValue(0, 100) / 100.0f * (max_radius - min_radius);
    carve_swamp(map, cx, cy, radius);
  }
}

static void carve_stream(Map *map, int base_x, float amplitude, float frequency, float phase) {
  for (int y = 0; y < map->h; y++) {
    int cx = base_x + (int)(amplitude * sinf((float)y * frequency + phase));
    for (int x = cx - 1; x <= cx + 1; x++) {
      Tile *t = map_tile(map, x, y);
      if (!t) continue;
      t->type = TILE_WATER;
      t->z = -0.5;
    }
  }
}

void scatter_trees(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_trees = (map->w * map->h) / 100;
  for (int i = 0; i < n_trees; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, BUILDING_DIR_COUNT);
    object_array_push(objects, (Object){
      .id = allocate_object_id(),
      .sprite = texture_state->oak[0][variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_TREE,
    });
    map_place_object(map, tx, ty, 1, 1, false);
  }
}

void build_tree_wall(ObjectArray *objects, Map *map, const Texture_State *texture_state, int wall_offset, int gap_center_x, int gap_half_width, int wall_thickness) {
  for (int layer = 0; layer < wall_thickness; layer++) {
    int layer_offset = wall_offset + layer;
    for (int tx = 0; tx < map->w; tx++) {
      if (abs(tx - gap_center_x) <= gap_half_width) continue;
      int ty = layer_offset - tx;
      if (!map_is_placeable(map, tx, ty, 1, 1)) continue;

      int variant = tile_variant(tx, ty, BUILDING_DIR_COUNT);
      object_array_push(objects, (Object){
        .id = allocate_object_id(),
        .sprite = texture_state->oak[0][variant],
        .tx = tx, .ty = ty, .z = 0,
        .footprint_w = 1, .footprint_h = 1,
        .kind = OBJECT_TREE,
      });
      map_place_object(map, tx, ty, 1, 1, false);
    }
  }
}

void scatter_rocks(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_rocks = (map->w * map->h) / 200;
  for (int i = 0; i < n_rocks; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, ROCK_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->rock[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_ROCK,
    });
    map_place_object(map, tx, ty, 1, 1, false);
  }
}

void scatter_grass_tufts(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_tufts = (map->w * map->h) / 12;
  for (int i = 0; i < n_tufts; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    FigureDirection dir = (FigureDirection)tile_variant(tx, ty, FIGURE_DIR_COUNT);
    Object tuft = {
      .sprite = texture_state->grass_tuft_idle[WINTER][dir],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_GRASS_TUFT,
    };
    tuft.facing = dir;
    object_array_push(objects, tuft);
  }
}

void scatter_moss_ferns(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_moss_ferns = (map->w * map->h) / 20;
  for (int i = 0; i < n_moss_ferns; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, MOSS_FERNS_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->moss_ferns[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_MOSS_FERNS,
    });
  }
}

void scatter_mushrooms(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_mushrooms = (map->w * map->h) / 400;
  for (int i = 0; i < n_mushrooms; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, MUSHROOMS_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->mushrooms[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_MUSHROOMS,
    });
  }
}

void scatter_strawberry(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_strawberry = (map->w * map->h) / 300;
  for (int i = 0; i < n_strawberry; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, STRAWBERRY_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->strawberry[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_STRAWBERRY,
    });
  }
}

void scatter_cairns(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_cairns = (map->w * map->h) / 600;
  for (int i = 0; i < n_cairns; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, CAIRN_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->cairn[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_CAIRN,
    });
    map_place_object(map, tx, ty, 1, 1, false);
  }
}

void scatter_woody_debris(ObjectArray *objects, Map *map, const Texture_State *texture_state) {
  int n_woody_debris = (map->w * map->h) / 150;
  for (int i = 0; i < n_woody_debris; i++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->type != TILE_GRASS || t->occupied) continue;

    int variant = tile_variant(tx, ty, WOODY_DEBRIS_VARIANT_COUNT);
    object_array_push(objects, (Object){
      .sprite = texture_state->woody_debris[variant],
      .tx = tx, .ty = ty, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_WOODY_DEBRIS,
    });
    map_place_object(map, tx, ty, 1, 1, false);
  }
}

void spawn_debug_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, int bx, int by, int n) {
  int *free_tx = malloc(n * sizeof(int));
  int *free_ty = malloc(n * sizeof(int));
  int n_free = map_free_tiles_near(map, bx, by, n, free_tx, free_ty);

  static unsigned int next_debug_figure_id = 900000;
  for (int i = 0; i < n_free; i++) {
    int fx = free_tx[i];
    int fy = free_ty[i];
    FigureDirection dir = (FigureDirection)tile_variant(fx, fy, FIGURE_DIR_COUNT);
    Object figure = {
      .id = next_debug_figure_id++,
      .sprite = texture_state->farmers[i % 2][FIGURE_ACTION_STAND][dir][0],
      .tx = fx, .ty = fy, .z = 0,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_FIGURE,
      .facing = dir,
      .figure = {
        .gather = {fx, fy},
        .species = i % 2,
        .flood_field_idx = -1,
        .prev_tile = -1,
        .best_distance_to_target = -1,
        .action = FIGURE_ACTION_STAND,
      },
    };
    object_array_push(objects, figure);
    map_place_figure(map, fx, fy);
  }
  free(free_tx);
  free(free_ty);
}

void define_nature(ObjectArray* objects, Map *map, TileState tile_state, const Texture_State* texture_state) {
  map_init(map, tile_state.N_WIDTH_TILES, tile_state.N_HEIGHT_TILES);

  int pond_cx = map->w * 2 / 3;
  int pond_cy = map->h / 3;
  float pond_radius = 16.0f;
  carve_pond(map, pond_cx, pond_cy, pond_radius);

  float freq = 0.05f;
  float amplitude = 10.0f;
  float phase = -(float)pond_cy * freq;
  carve_stream(map, pond_cx, amplitude, freq, phase);

  scatter_swamps(map, 4);

  scatter_trees(objects, map, texture_state);
  scatter_grass_tufts(objects, map, texture_state);
  scatter_rocks(objects, map, texture_state);
  scatter_moss_ferns(objects, map, texture_state);
  scatter_mushrooms(objects, map, texture_state);
  scatter_strawberry(objects, map, texture_state);
  scatter_cairns(objects, map, texture_state);
  scatter_woody_debris(objects, map, texture_state);

  const int gap_center_x = map->w / 2;
  const int wall_offset = map->w;
  build_tree_wall(objects, map, texture_state, wall_offset, gap_center_x, 3, 3);

  const int spawn_tx = gap_center_x - 30;
  const int spawn_ty = gap_center_x - 5;
  spawn_debug_figures(objects, map, texture_state, spawn_tx, spawn_ty, 100);
}
