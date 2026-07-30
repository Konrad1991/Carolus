#include "drawing/update_grass.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"

void update_grass_tufts(ObjectArray *objects, Texture_State *texture_state, WeatherState *weather_state, SeasonBlend season_blend) {
  bool wind_active = weather_state->current == WEATHER_WINDY || weather_state->current == WEATHER_RAIN;
  const float grass_sway_fps = 6.0f;
  const FigureDirection grass_calm_direction = FIGURE_DIR_FRONT_RIGHT;

  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_GRASS_TUFT) continue;

    if (wind_active) {
      int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
      int frame = ((int)(GetTime() * grass_sway_fps) + phase) % FIGURE_MAX_FRAMES;
      o->sprite = texture_state->grass_tuft_sway[season_blend.base][weather_state->wind_direction][frame];
    } else {
      o->sprite = texture_state->grass_tuft_idle[season_blend.base][grass_calm_direction];
    }
  }
}
