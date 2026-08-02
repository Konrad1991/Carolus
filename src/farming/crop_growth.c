#include "farming/crop_growth.h"
#include "containers/arrays.h"
#include "raylib.h"
#include "soil/soil.h"
#include "utils/game_time.h"

typedef struct {
  float min_temp;
  float min_water;
  float min_minerals;
} GerminationRequirement;

static const GerminationRequirement REQ_GERMINATION = {
  .min_temp = 6.0f,
  .min_water = 15.0f,
  .min_minerals = 0.0f
};

static bool germination_conditions_met(const GerminationRequirement *req, float temp, float water, float minerals) {
  return temp >= req->min_temp && water >= req->min_water && minerals >= req->min_minerals;
}

typedef struct {
  float min_temp;
  float max_temp;
  float water_uptake_rate;
  float target_water_uptake;
  float minerals_uptake_rate;
  float target_minerals_uptake;
} GrowthRequirement;

static const GrowthRequirement REQ_TILLERING = {
  .min_temp = 10.0f,
  .max_temp = 30.0f,
  .water_uptake_rate = 0.05f,
  .target_water_uptake = 0.3f / WHEAT_TUFTS_PER_TILE,
  .minerals_uptake_rate = 0.004f,
  .target_minerals_uptake = 0.05f / WHEAT_TUFTS_PER_TILE
};
static const GrowthRequirement REQ_HEADING = {
  .min_temp = 12.0f,
  .max_temp = 30.0f,
  .water_uptake_rate = 0.3f,
  .target_water_uptake = 0.6f / WHEAT_TUFTS_PER_TILE,
  .minerals_uptake_rate = 0.008f,
  .target_minerals_uptake = 0.1f / WHEAT_TUFTS_PER_TILE
};
static const GrowthRequirement REQ_RIPENING = {
  .min_temp = 14.0f,
  .max_temp = -1.0f,
  .water_uptake_rate = 0.2f,
  .target_water_uptake = 0.5f / WHEAT_TUFTS_PER_TILE,
  .minerals_uptake_rate = 0.01f,
  .target_minerals_uptake = 0.003f / WHEAT_TUFTS_PER_TILE
};

static bool growth_temp_met(const GrowthRequirement *req, float temp) {
  if (temp < req->min_temp) return false;
  if (req->max_temp >= 0.0f && temp > req->max_temp) return false;
  return true;
}

static void compute_field_averages(Map *map, const Field *field, float *out_water, float *out_minerals, float *out_temp) {
  double sum_water = 0.0, sum_minerals = 0.0, sum_temp = 0.0;
  int count = 0;
  for (int y = field->corners_field[0][1]; y <= field->corners_field[1][1]; y++) {
    for (int x = field->corners_field[0][0]; x <= field->corners_field[1][0]; x++) {
      const Tile *t = map_tile(map, x, y);
      if (!t) continue;
      sum_water += t->soil_water;
      sum_minerals += t->soil_minerals;
      sum_temp += t->soil_temperature;
      count++;
    }
  }
  if (count == 0) {
    *out_water = 0.0f;
    *out_minerals = 0.0f;
    *out_temp = 0.0f;
    return;
  }
  *out_water = (float)(sum_water / count);
  *out_minerals = (float)(sum_minerals / count);
  *out_temp = (float)(sum_temp / count);
}

static const Vector2 WHEAT_TUFT_SLOTS[WHEAT_TUFTS_PER_TILE] = {
  {-0.50f, -0.50f},
  {-0.75f, -0.75f},
  {-0.25f, -0.75f},
  {-0.75f, -0.25f},
  {-0.25f, -0.25f},
  {-0.85f, -0.50f},
  {-0.15f, -0.50f},
};

static void scatter_wheat_tufts(ObjectArray *objects, const Texture_State *texture_state, Map *map, const Field *field) {
  const float wheat_vigor_min = 0.8f;
  const float wheat_vigor_max = 1.4f;
  float min_x = (float)field->corners_field[0][0] - 1.0f;
  float max_x = (float)field->corners_field[1][0];
  float min_y = (float)field->corners_field[0][1] - 1.0f;
  float max_y = (float)field->corners_field[1][1];

  for (int ty = field->corners_field[0][1]; ty <= field->corners_field[1][1]; ty++) {
    for (int tx = field->corners_field[0][0]; tx <= field->corners_field[1][0]; tx++) {
      Tile *t = map_tile(map, tx, ty);
      int tz = t ? t->z : 0;
      if (t) t->wheat_tuft_count = 0;
      for (int n = 0; n < WHEAT_TUFTS_PER_TILE; n++) {
        FigureDirection dir = (FigureDirection)GetRandomValue(0, FIGURE_DIR_COUNT - 1);
        Object tuft = {
          .id = allocate_object_id(),
          .sprite = texture_state->wheat_tuft_idle[WHEAT_STAGE_YOUNG][dir],
          .tx = tx, .ty = ty, .z = tz,
          .footprint_w = 1, .footprint_h = 1,
          .kind = OBJECT_WHEAT_TUFT,
        };
        tuft.facing = dir;
        tuft.wheat.wheat_stage = WHEAT_STAGE_YOUNG;
        tuft.wheat.wheat_progress_timer = 0.0f;
        tuft.wheat.wheat_vigor = GetRandomValue(0, 1000) / 1000.0f * (wheat_vigor_max - wheat_vigor_min) + wheat_vigor_min;

        float dx = tx + WHEAT_TUFT_SLOTS[n].x + GetRandomValue(-8, 8) / 100.0f;
        float dy = ty + WHEAT_TUFT_SLOTS[n].y + GetRandomValue(-8, 8) / 100.0f;
        if (dx < min_x) dx = min_x;
        if (dx > max_x) dx = max_x;
        if (dy < min_y) dy = min_y;
        if (dy > max_y) dy = max_y;
        tuft.draw = (PositionFloat){dx, dy};
        object_array_push(objects, tuft);
        if (t) t->wheat_tuft_ids[t->wheat_tuft_count++] = tuft.id;
      }
    }
  }
}

static const GrowthRequirement *requirement_for_wheat_stage(WheatStage stage) {
  switch (stage) {
    case WHEAT_STAGE_YOUNG: return &REQ_TILLERING;
    case WHEAT_STAGE_MIDDLE: return &REQ_HEADING;
    case WHEAT_STAGE_LARGE_GREEN: return &REQ_RIPENING;
    default: return NULL;
  }
}

static void set_wheat_stage(Object *o, WheatStage stage) {
  o->wheat.wheat_stage = stage;
  o->wheat.wheat_stage_age = 0.0f;
  o->wheat.wheat_progress_timer = 0.0f;
  o->wheat.water_absorbed = 0.0f;
  o->wheat.minerals_absorbed = 0.0f;
}

static void update_wheat_tuft_growth(Map *map, ObjectArray *objects) {
  const float dt = game_delta_time();
  const float wheat_overripe_seconds = SECONDS_PER_MONTH * 2.0f;
  const float wheat_destroyed_seconds = SECONDS_PER_MONTH * 1.0f;
  const float wheat_max_immature_seconds = SECONDS_PER_MONTH * 2.5f;
  const float growth_sustain_seconds = SECONDS_PER_MONTH * 0.70f;

  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;

    WheatStage stage = o->wheat.wheat_stage;
    if (stage == WHEAT_STAGE_HARVESTED || stage == WHEAT_STAGE_DESTROYED) continue;

    o->wheat.wheat_stage_age += dt;

    if (stage == WHEAT_STAGE_RIPE) {
      if (o->wheat.wheat_stage_age >= wheat_overripe_seconds) set_wheat_stage(o, WHEAT_STAGE_OVERRIPE);
      continue;
    }
    if (stage == WHEAT_STAGE_OVERRIPE) {
      if (o->wheat.wheat_stage_age >= wheat_destroyed_seconds) set_wheat_stage(o, WHEAT_STAGE_DESTROYED);
      continue;
    }

    if (o->wheat.wheat_stage_age >= wheat_max_immature_seconds) {
      set_wheat_stage(o, WHEAT_STAGE_DESTROYED);
      continue;
    }

    const GrowthRequirement *req = requirement_for_wheat_stage(stage);
    if (!req) continue;

    const Tile *t = map_tile(map, o->tx, o->ty);
    if (!t) continue;

    if (growth_temp_met(req, t->soil_temperature)) {
      float growth_dt = dt * o->wheat.wheat_vigor;
      o->wheat.wheat_progress_timer += growth_dt;
      o->wheat.water_absorbed += soil_deplete_water(map, o->tx, o->ty, req->water_uptake_rate * growth_dt);
      o->wheat.minerals_absorbed += soil_deplete_minerals(map, o->tx, o->ty, req->minerals_uptake_rate * growth_dt);
    }

    if (o->wheat.wheat_progress_timer < growth_sustain_seconds ||
        o->wheat.water_absorbed < req->target_water_uptake ||
        o->wheat.minerals_absorbed < req->target_minerals_uptake) continue;

    set_wheat_stage(o, (WheatStage)(stage + 1));
  }
}

static bool field_has_active_tuft(const Field *field, const ObjectArray *objects) {
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;
    if (o->tx < field->corners_field[0][0] || o->tx > field->corners_field[1][0]) continue;
    if (o->ty < field->corners_field[0][1] || o->ty > field->corners_field[1][1]) continue;
    if (o->wheat.wheat_stage != WHEAT_STAGE_HARVESTED && o->wheat.wheat_stage != WHEAT_STAGE_DESTROYED) return true;
  }
  return false;
}

static bool field_condition_is_growing(FieldCondition condition) {
  return condition == SMALL_GREEN_PLANTS || condition == MEDIUM_GREEN_PLANTS ||
    condition == LARGE_GREEN_PLANTS || condition == LARGE_YELLOW_PLANTS;
}

static FieldCondition condition_for_wheat_stage(WheatStage stage) {
  switch (stage) {
    case WHEAT_STAGE_YOUNG: return SMALL_GREEN_PLANTS;
    case WHEAT_STAGE_MIDDLE: return MEDIUM_GREEN_PLANTS;
    case WHEAT_STAGE_LARGE_GREEN: return LARGE_GREEN_PLANTS;
    case WHEAT_STAGE_RIPE:
    case WHEAT_STAGE_OVERRIPE:
    default: return LARGE_YELLOW_PLANTS;
  }
}

// A field's condition tracks its least-advanced still-growing tuft, so it only
// reports e.g. LARGE_YELLOW_PLANTS once the whole field has caught up, not
// just a few early tufts.
static void update_field_growth_condition(Field *field, const ObjectArray *objects) {
  bool found_active = false;
  WheatStage least_advanced = WHEAT_STAGE_OVERRIPE;
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;
    if (o->tx < field->corners_field[0][0] || o->tx > field->corners_field[1][0]) continue;
    if (o->ty < field->corners_field[0][1] || o->ty > field->corners_field[1][1]) continue;
    WheatStage stage = o->wheat.wheat_stage;
    if (stage == WHEAT_STAGE_HARVESTED || stage == WHEAT_STAGE_DESTROYED) continue;
    found_active = true;
    if (stage < least_advanced) least_advanced = stage;
  }
  if (!found_active) return;
  field->field_condition = condition_for_wheat_stage(least_advanced);
}

static const float FALLOW_OVERGROW_SECONDS = SECONDS_PER_MONTH;

// Bare soil not currently growing anything (freshly dug or fallow after harvest)
// slowly grasses over, cross-fading toward a grass look the same way seasons
// cross-fade (see draw_dirt/draw_grass in drawing.c).
static void update_fallow_overgrowth(Map *map, const Field *field, float dt) {
  for (int y = field->corners_field[0][1]; y <= field->corners_field[1][1]; y++) {
    for (int x = field->corners_field[0][0]; x <= field->corners_field[1][0]; x++) {
      Tile *t = map_tile(map, x, y);
      if (!t || t->type != TILE_SOIL) continue;
      t->fallow_grass_blend += dt / FALLOW_OVERGROW_SECONDS;
      if (t->fallow_grass_blend > 1.0f) t->fallow_grass_blend = 1.0f;
    }
  }
}

void update_crop_growth(Map *map, ObjectArray *objects, const Texture_State *texture_state, GameState *game_state) {
  const float dt = game_delta_time();
  const float germination_sustain_seconds = SECONDS_PER_MONTH * 0.5f;

  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      Field *f = &m->fields.data[fi];

      if (field_condition_is_growing(f->field_condition)) {
        if (!field_has_active_tuft(f, objects)) {
          f->field_condition = FALLOW;
        } else {
          update_field_growth_condition(f, objects);
        }
        continue;
      }
      if (f->field_condition == FALLOW || f->field_condition == PLOWED) {
        update_fallow_overgrowth(map, f, dt);
        continue;
      }
      if (f->field_condition != SOWED) continue;

      float water, minerals, temp;
      compute_field_averages(map, f, &water, &minerals, &temp);

      if (germination_conditions_met(&REQ_GERMINATION, temp, water, minerals)) {
        f->stage_progress_timer += dt;
      }

      if (f->stage_progress_timer < germination_sustain_seconds) continue;
      f->stage_progress_timer = 0.0f;
      f->field_condition = SMALL_GREEN_PLANTS;
      scatter_wheat_tufts(objects, texture_state, map, f);
    }
  }

  update_wheat_tuft_growth(map, objects);
}
