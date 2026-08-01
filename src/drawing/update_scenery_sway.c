#include "drawing/update_scenery_sway.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"

// Grass, trees, and wheat used to each run their own full pass over the
// ObjectArray every frame just to pick out their own kind - three redundant
// O(n) scans over what can be a very large array (figures, wheat tufts,
// grass tufts, trees all mixed together). Merged into one pass.

static const bool WHEAT_STAGE_SWAYS[WHEAT_STAGE_COUNT] = {
  [WHEAT_STAGE_YOUNG] = true,
  [WHEAT_STAGE_MIDDLE] = true,
  [WHEAT_STAGE_LARGE_GREEN] = true,
  [WHEAT_STAGE_RIPE] = true,
  [WHEAT_STAGE_OVERRIPE] = true,
  [WHEAT_STAGE_HARVESTED] = false,
  [WHEAT_STAGE_DESTROYED] = false,
};

static void update_grass_tuft_sway(Object *o, const Texture_State *texture_state, bool wind_active,
                                   const WeatherState *weather_state, SeasonBlend season_blend, float sway_fps) {
  const FigureDirection grass_calm_direction = FIGURE_DIR_FRONT_RIGHT;
  if (wind_active) {
    int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
    int frame = ((int)(GetTime() * sway_fps) + phase) % FIGURE_MAX_FRAMES;
    o->sprite = texture_state->grass_tuft_sway[season_blend.base][weather_state->wind_direction][frame];
  } else {
    o->sprite = texture_state->grass_tuft_idle[season_blend.base][grass_calm_direction];
  }
}

static void update_tree_sway(Object *o, const Texture_State *texture_state, bool wind_active,
                             const WeatherState *weather_state, SeasonBlend season_blend, float sway_fps) {
  if (o->tree.state == TREE_STATE_BEAM) {
    o->sprite = texture_state->oak_beam[o->facing];
    return;
  }
  if (o->tree.state == TREE_STATE_FELLED) {
    o->sprite = texture_state->oak_trunk[o->facing];
    return;
  }
  if (wind_active) {
    // All trees lean the same way - the global wind direction.
    int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
    int frame = ((int)(GetTime() * sway_fps) + phase) % FIGURE_MAX_FRAMES;
    o->sprite = texture_state->oak_sway[season_blend.base][weather_state->wind_direction][frame];
  } else {
    int variant = tile_variant(o->tx, o->ty, BUILDING_DIR_COUNT);
    o->sprite = texture_state->oak[season_blend.base][variant];
  }
}

static void update_wheat_tuft_sway(Object *o, const Texture_State *texture_state, bool wind_active,
                                   const WeatherState *weather_state, float sway_fps) {
  const FigureDirection wheat_calm_direction = FIGURE_DIR_FRONT_RIGHT;
  WheatStage stage = o->wheat.wheat_stage;
  if (stage == WHEAT_STAGE_HARVESTED) {
    o->sprite = texture_state->wheat_sheaf;
    return;
  }
  if (wind_active && WHEAT_STAGE_SWAYS[stage]) {
    int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
    int frame = ((int)(GetTime() * sway_fps) + phase) % FIGURE_MAX_FRAMES;
    o->sprite = texture_state->wheat_tuft_sway[stage][weather_state->wind_direction][frame];
  } else {
    o->sprite = texture_state->wheat_tuft_idle[stage][wheat_calm_direction];
  }
}

void update_scenery_sway(ObjectArray *objects, const Texture_State *texture_state, const WeatherState *weather_state, SeasonBlend season_blend) {
  bool wind_active = weather_state->current == WEATHER_WINDY || weather_state->current == WEATHER_RAIN;
  const float sway_fps = 6.0f;

  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    switch (o->kind) {
      case OBJECT_GRASS_TUFT:
        update_grass_tuft_sway(o, texture_state, wind_active, weather_state, season_blend, sway_fps);
        break;
      case OBJECT_TREE:
        update_tree_sway(o, texture_state, wind_active, weather_state, season_blend, sway_fps);
        break;
      case OBJECT_WHEAT_TUFT:
        update_wheat_tuft_sway(o, texture_state, wind_active, weather_state, sway_fps);
        break;
      default:
        break;
    }
  }
}
