#include "weather/puddles.h"
#include "drawing/drawing_helper.h"
#include "containers/arrays.h"
#include "map/map.h"
#include "raylib.h"
#include "utils/game_time.h"
#include <stdlib.h>

static const float PUDDLE_LIFETIME_MIN = 24.0f;
static const float PUDDLE_LIFETIME_MAX = 45.0f;

static const float PUDDLE_REF_TEMPERATURE_C = 20.0f;
static const float PUDDLE_TEMP_DURATION_FACTOR = 0.05f;

static const int PUDDLE_DENSITY_DIVISOR = 250;
static const int PUDDLE_SPAWN_ATTEMPTS = 10;

static float random_lifetime(float temperature_celsius) {
  const float puddle_lifetime_scale_min = 0.2f;
  float scale = 1.0f - (temperature_celsius - PUDDLE_REF_TEMPERATURE_C) * PUDDLE_TEMP_DURATION_FACTOR;
  if (scale < puddle_lifetime_scale_min) scale = puddle_lifetime_scale_min;
  int lo = (int)(PUDDLE_LIFETIME_MIN * scale);
  int hi = (int)(PUDDLE_LIFETIME_MAX * scale);
  if (hi <= lo) hi = lo + 1;
  return (float)GetRandomValue(lo, hi);
}

// Active puddles are tracked by id here so decay/spawn only ever touch this
// small list instead of scanning the whole (potentially huge) ObjectArray.
static unsigned int *g_active_puddle_ids = NULL;
static int g_puddle_count = 0;
static int g_puddle_capacity = 0;

static void track_puddle(unsigned int id) {
  if (g_puddle_count == g_puddle_capacity) {
    g_puddle_capacity = g_puddle_capacity == 0 ? 64 : g_puddle_capacity * 2;
    g_active_puddle_ids = realloc(g_active_puddle_ids, g_puddle_capacity * sizeof(unsigned int));
  }
  g_active_puddle_ids[g_puddle_count++] = id;
}

static void untrack_puddle(int list_index) {
  g_active_puddle_ids[list_index] = g_active_puddle_ids[g_puddle_count - 1];
  g_puddle_count--;
}

static void try_spawn_puddle(ObjectArray *objects, Map *map, Texture_State *texture_state, float temperature_celsius) {
  for (int attempt = 0; attempt < PUDDLE_SPAWN_ATTEMPTS; attempt++) {
    int tx = GetRandomValue(0, map->w - 1);
    int ty = GetRandomValue(0, map->h - 1);
    Tile *t = map_tile(map, tx, ty);
    if (!t || t->occupied) continue;
    if (t->type != TILE_GRASS && t->type != TILE_SOIL) continue;
    if (t->has_puddle) continue;

    FigureDirection dir = (FigureDirection)tile_variant(tx, ty, FIGURE_DIR_COUNT);
    float base_scale = texture_state->puddle[dir].scale;
    Object puddle = {
      .id = allocate_object_id(),
      .sprite = texture_state->puddle[dir],
      .tx = tx, .ty = ty, .z = t->z,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_PUDDLE,
    };
    float lifetime = random_lifetime(temperature_celsius);
    puddle.sprite.scale = base_scale;
    puddle.puddle.lifetime_total = lifetime;
    puddle.puddle.lifetime_remaining = lifetime;
    puddle.puddle.base_scale = base_scale;
    object_array_push(objects, puddle);
    t->has_puddle = true;
    track_puddle(puddle.id);
    return;
  }
}

void update_puddles(ObjectArray *objects, Map *map, Texture_State *texture_state, const WeatherState *weather_state) {
  float dt = game_delta_time();

  for (int i = g_puddle_count - 1; i >= 0; i--) {
    int idx = object_array_find_by_id(objects, g_active_puddle_ids[i]);
    if (idx < 0) {
      untrack_puddle(i);
      continue;
    }
    Object *o = &objects->data[idx];

    o->puddle.lifetime_remaining -= dt;
    if (o->puddle.lifetime_remaining <= 0.0f) {
      Tile *t = map_tile(map, o->tx, o->ty);
      if (t) t->has_puddle = false;
      object_array_remove_swap(objects, idx);
      untrack_puddle(i);
      continue;
    }
    float t = o->puddle.lifetime_remaining / o->puddle.lifetime_total;
    o->sprite.scale = o->puddle.base_scale * t;
  }

  if (weather_state->current != WEATHER_RAIN) return;
  int target = (map->w * map->h) / PUDDLE_DENSITY_DIVISOR;
  if (g_puddle_count < target) {
    try_spawn_puddle(objects, map, texture_state, weather_state->temperature_celsius);
  }
}
