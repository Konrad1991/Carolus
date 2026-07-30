#include "drawing/update_wheat.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"

static const bool WHEAT_STAGE_SWAYS[WHEAT_STAGE_COUNT] = {
  [WHEAT_STAGE_YOUNG] = true,
  [WHEAT_STAGE_MIDDLE] = true,
  [WHEAT_STAGE_LARGE_GREEN] = true,
  [WHEAT_STAGE_RIPE] = true,
  [WHEAT_STAGE_OVERRIPE] = true,
  [WHEAT_STAGE_HARVESTED] = false,
  [WHEAT_STAGE_DESTROYED] = false,
};

void update_wheat_tufts(ObjectArray *objects, const Texture_State *texture_state, const WeatherState *weather_state) {
  bool wind_active = weather_state->current == WEATHER_WINDY || weather_state->current == WEATHER_RAIN;
  const float wheat_sway_fps = 6.0f;
  const FigureDirection wheat_calm_direction = FIGURE_DIR_FRONT_RIGHT;

  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;
    WheatStage stage = o->wheat.wheat_stage;
    if (stage == WHEAT_STAGE_HARVESTED) {
      o->sprite = texture_state->wheat_sheaf;
      continue;
    }
    if (wind_active && WHEAT_STAGE_SWAYS[stage]) {
      int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
      int frame = ((int)(GetTime() * wheat_sway_fps) + phase) % FIGURE_MAX_FRAMES;
      o->sprite = texture_state->wheat_tuft_sway[stage][weather_state->wind_direction][frame];
    } else {
      o->sprite = texture_state->wheat_tuft_idle[stage][wheat_calm_direction];
    }
  }
}
