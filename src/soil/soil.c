#include "soil/soil.h"
#include "raylib.h"
#include "utils/game_time.h"
#include <stdlib.h>

#define SOIL_MINERAL_MIN 0.0f
#define SOIL_MINERAL_MAX SOIL_MINERAL_CAPACITY_G

static bool tile_touches_water(Map *map, int x, int y) {
  const Tile *n;
  n = map_tile(map, x - 1, y); if (n && n->type == TILE_WATER) return true;
  n = map_tile(map, x + 1, y); if (n && n->type == TILE_WATER) return true;
  n = map_tile(map, x, y - 1); if (n && n->type == TILE_WATER) return true;
  n = map_tile(map, x, y + 1); if (n && n->type == TILE_WATER) return true;
  return false;
}

static void compute_distance_to_water(Map *map, int *dist_out) {
  int n = map->w * map->h;
  int *queue = malloc(n * sizeof(int));
  int head = 0;
  int tail = 0;
  for (int i = 0; i < n; i++) dist_out[i] = -1;
  for (int i = 0; i < n; i++) {
    if (map->tiles[i].type == TILE_WATER) {
      dist_out[i] = 0;
      queue[tail++] = i;
    }
  }

  const int dx[4] = {-1, 1, 0, 0};
  const int dy[4] = {0, 0, -1, 1};
  while (head < tail) {
    int idx = queue[head++];
    int x = idx % map->w;
    int y = idx / map->w;
    for (int k = 0; k < 4; k++) {
      int nx = x + dx[k];
      int ny = y + dy[k];
      if (nx < 0 || nx >= map->w || ny < 0 || ny >= map->h) continue;
      int nidx = ny * map->w + nx;
      if (dist_out[nidx] != -1) continue;
      dist_out[nidx] = dist_out[idx] + 1;
      queue[tail++] = nidx;
    }
  }
  free(queue);
}

static void build_fertility_noise(Map *map, float *out) {
  const int fertility_noise_cell = 40;
  int grid_w = map->w / fertility_noise_cell + 2;
  int grid_h = map->h / fertility_noise_cell + 2;
  float *control = malloc(grid_w * grid_h * sizeof(float));
  for (int i = 0; i < grid_w * grid_h; i++) {
    control[i] = GetRandomValue(0, 1000) / 1000.0f;
  }

  for (int y = 0; y < map->h; y++) {
    for (int x = 0; x < map->w; x++) {
      float gx = (float)x / fertility_noise_cell;
      float gy = (float)y / fertility_noise_cell;
      int gx0 = (int)gx;
      int gy0 = (int)gy;
      int gx1 = gx0 + 1;
      int gy1 = gy0 + 1;
      float fx = gx - gx0;
      float fy = gy - gy0;

      float v00 = control[gy0 * grid_w + gx0];
      float v10 = control[gy0 * grid_w + gx1];
      float v01 = control[gy1 * grid_w + gx0];
      float v11 = control[gy1 * grid_w + gx1];
      float top = v00 + (v10 - v00) * fx;
      float bottom = v01 + (v11 - v01) * fx;
      out[y * map->w + x] = top + (bottom - top) * fy;
    }
  }
  free(control);
}

void init_soil(Map *map, const WeatherState *weather_state) {
  const float soil_init_water_near = 180.0f;
  const float soil_init_water_far = 150.0f;
  const int soil_init_water_radius = 20;
  const float soil_init_mineral_min = 15.0f;
  const float soil_init_mineral_max = SOIL_MINERAL_CAPACITY_G;

  int n = map->w * map->h;
  int *water_dist = malloc(n * sizeof(int));
  compute_distance_to_water(map, water_dist);
  float *fertility = malloc(n * sizeof(float));
  build_fertility_noise(map, fertility);

  for (int i = 0; i < n; i++) {
    Tile *t = &map->tiles[i];

    int d = water_dist[i] < 0 ? soil_init_water_radius : water_dist[i];
    float falloff = 1.0f - (float)d / soil_init_water_radius;
    if (falloff < 0.0f) falloff = 0.0f;
    t->soil_water = soil_init_water_far + (soil_init_water_near - soil_init_water_far) * falloff;

    t->soil_minerals = soil_init_mineral_min + (soil_init_mineral_max - soil_init_mineral_min) * fertility[i];
    t->soil_temperature = weather_state->temperature_celsius;
  }

  free(water_dist);
  free(fertility);
}

static void update_soil_water(Tile *t, Map *map, int x, int y, const WeatherState *weather_state) {
  const float soil_water_min = 0.0f;
  const float soil_water_max = SOIL_WATER_CAPACITY_L;
  const float soil_water_rain_rate = 0.03f;
  const float soil_water_dry_rate = 0.033f;
  const float soil_water_dry_ref_temperature = 20.0f;
  const float soil_water_proximity_floor = 120.0f;
  const float soil_water_proximity_rate = 0.02f;

  const bool raining = weather_state->current == WEATHER_RAIN;
  const float target = raining ? soil_water_max : soil_water_min;

  float dry_factor = t->soil_temperature / soil_water_dry_ref_temperature;
  if (dry_factor < 0.0f) dry_factor = 0.0f;
  float rate = raining ? soil_water_rain_rate : (soil_water_dry_rate * dry_factor);
  t->soil_water += (target - t->soil_water) * rate * game_delta_time();

  if (t->soil_water < soil_water_proximity_floor && tile_touches_water(map, x, y)) {
    t->soil_water += (soil_water_proximity_floor - t->soil_water) * soil_water_proximity_rate * game_delta_time();
  }
  if (t->soil_water < soil_water_min) t->soil_water = soil_water_min;
  if (t->soil_water > soil_water_max) t->soil_water = soil_water_max;
}

static void update_soil_temp(Tile *t, const WeatherState *weather_state) {
  const float soil_temp_lag_rate = 0.1f;
  const float soil_moisture_intertia = 2.0f;
  float wetness = t->soil_water / SOIL_WATER_CAPACITY_L;
  float inertia = 1.0f + wetness * soil_moisture_intertia;
  float rate = soil_temp_lag_rate / inertia;
  t->soil_temperature += (weather_state->temperature_celsius - t->soil_temperature) * rate * game_delta_time();
}

static void update_soil_minerals(Tile *t, const WeatherState *weather_state) {
  const float soil_mineral_passive_regeneration_rate = 0.03f;
  const float soil_mineral_regeneration_ref_temperature = 20.0f;
  const float soil_mineral_leach_rate_vegetated = 0.0015f;
  const float soil_mineral_leach_floor = 15.0f;

  float regen_factor = t->soil_temperature / soil_mineral_regeneration_ref_temperature;
  if (regen_factor < 0.0f) regen_factor = 0.0f;
  t->soil_minerals += soil_mineral_passive_regeneration_rate * regen_factor * game_delta_time();

  if (weather_state->current == WEATHER_RAIN) {
    t->soil_minerals += (soil_mineral_leach_floor - t->soil_minerals) * soil_mineral_leach_rate_vegetated * game_delta_time();
  }

  if (t->soil_minerals < SOIL_MINERAL_MIN) t->soil_minerals = SOIL_MINERAL_MIN;
  if (t->soil_minerals > SOIL_MINERAL_MAX) t->soil_minerals = SOIL_MINERAL_MAX;
}

static bool field_is_actively_growing(FieldCondition condition) {
  return condition == SMALL_GREEN_PLANTS || condition == MEDIUM_GREEN_PLANTS || condition == LARGE_GREEN_PLANTS;
}

static void drain_cultivated_fields(Map *map, const GameState *game_state) {
  const float soil_mineral_cultiviation_drain_rate = 0.08f;
  float drain = soil_mineral_cultiviation_drain_rate * game_delta_time();
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      if (!field_is_actively_growing(f->field_condition)) continue;
      for (int y = f->corners_field[0][1]; y <= f->corners_field[1][1]; y++) {
        for (int x = f->corners_field[0][0]; x <= f->corners_field[1][0]; x++) {
          Tile *t = map_tile(map, x, y);
          if (!t) continue;
          t->soil_minerals -= drain;
          if (t->soil_minerals < SOIL_MINERAL_MIN) t->soil_minerals = SOIL_MINERAL_MIN;
        }
      }
    }
  }
}

static void regenerate_resting_fields(Map *map, const GameState *game_state) {
  const float soil_mineral_fallow_bonus_rate = 0.01f;
  float bonus = soil_mineral_fallow_bonus_rate * game_delta_time();
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      if (field_is_actively_growing(f->field_condition)) continue;
      for (int y = f->corners_field[0][1]; y <= f->corners_field[1][1]; y++) {
        for (int x = f->corners_field[0][0]; x <= f->corners_field[1][0]; x++) {
          Tile *t = map_tile(map, x, y);
          if (!t) continue;
          t->soil_minerals += bonus;
          if (t->soil_minerals > SOIL_MINERAL_MAX) t->soil_minerals = SOIL_MINERAL_MAX;
        }
      }
    }
  }
}

static bool field_is_bare(FieldCondition condition) {
  return condition == PLOWED || condition == SOWED;
}

static void leach_bare_fields(Map *map, const WeatherState *weather_state, const GameState *game_state) {
  if (weather_state->current != WEATHER_RAIN) return;
  const float soil_mineral_leach_rate_bare_extra = 0.0015f;
  const float soil_mineral_leach_floor = 15.0f;
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      if (!field_is_bare(f->field_condition)) continue;
      for (int y = f->corners_field[0][1]; y <= f->corners_field[1][1]; y++) {
        for (int x = f->corners_field[0][0]; x <= f->corners_field[1][0]; x++) {
          Tile *t = map_tile(map, x, y);
          if (!t) continue;
          t->soil_minerals += (soil_mineral_leach_floor - t->soil_minerals) * soil_mineral_leach_rate_bare_extra * game_delta_time();
        }
      }
    }
  }
}

void update_soil(Map *map, const WeatherState *weather_state, const GameState *game_state) {
  for (int y = 0; y < map->h; y++) {
    for (int x = 0; x < map->w; x++) {
      Tile *t = map_tile(map, x, y);
      update_soil_water(t, map, x, y, weather_state);
      update_soil_temp(t, weather_state);
      update_soil_minerals(t, weather_state);
    }
  }
  drain_cultivated_fields(map, game_state);
  regenerate_resting_fields(map, game_state);
  leach_bare_fields(map, weather_state, game_state);
}

float soil_deplete_minerals(Map *map, int tx, int ty, float amount) {
  Tile *t = map_tile(map, tx, ty);
  if (!t) return 0.0f;
  float taken = amount < t->soil_minerals ? amount : t->soil_minerals;
  t->soil_minerals -= taken;
  return taken;
}

float soil_deplete_water(Map *map, int tx, int ty, float amount) {
  Tile *t = map_tile(map, tx, ty);
  if (!t) return 0.0f;
  float taken = amount < t->soil_water ? amount : t->soil_water;
  t->soil_water -= taken;
  return taken;
}

void soil_enrich_minerals(Map *map, int tx, int ty, float amount) {
  Tile *t = map_tile(map, tx, ty);
  if (!t) return;
  t->soil_minerals += amount;
  if (t->soil_minerals > SOIL_MINERAL_MAX) t->soil_minerals = SOIL_MINERAL_MAX;
}
