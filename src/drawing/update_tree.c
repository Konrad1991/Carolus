#include "drawing/update_tree.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"

void update_trees(ObjectArray *objects, Texture_State *texture_state, WeatherState *weather_state, SeasonBlend season_blend) {
  bool wind_active = weather_state->current == WEATHER_WINDY || weather_state->current == WEATHER_RAIN;
  const float oak_sway_fps = 6.0f;

  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_TREE) continue;

    if (wind_active) {
      // All trees lean the same way - the global wind direction.
      int phase = tile_variant(o->tx, o->ty, FIGURE_MAX_FRAMES);
      int frame = ((int)(GetTime() * oak_sway_fps) + phase) % FIGURE_MAX_FRAMES;
      o->sprite = texture_state->oak_sway[season_blend.base][weather_state->wind_direction][frame];
    } else {
      int variant = tile_variant(o->tx, o->ty, BUILDING_DIR_COUNT);
      o->sprite = texture_state->oak[season_blend.base][variant];
    }
  }
}
