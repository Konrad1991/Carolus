#include "weather/clouds.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"
#include "utils/game_time.h"

static const float CLOUD_FPS = 4.0f;
static const float CLOUD_SPEED_MIN = 25.0f;
static const float CLOUD_SPEED_MAX = 55.0f;

void init_clouds(CloudState *cloud_state) {
  int screen_w = GetScreenWidth();
  int screen_h = GetScreenHeight();
  for (int i = 0; i < CLOUD_COUNT; i++) {
    Cloud *c = &cloud_state->clouds[i];
    c->x = (float)GetRandomValue(0, screen_w);
    c->y = (float)GetRandomValue(0, screen_h);
    c->speed = (float)GetRandomValue((int)CLOUD_SPEED_MIN, (int)CLOUD_SPEED_MAX);
    c->phase = (float)GetRandomValue(0, FIGURE_MAX_FRAMES - 1);
  }
}

void update_clouds(CloudState *cloud_state, const WeatherState *weather_state) {
  if (weather_state->current != WEATHER_RAIN) return;

  Vector2 dir = screen_direction_for_facing(weather_state->wind_direction);
  float dt = game_delta_time();
  int screen_w = GetScreenWidth();
  int screen_h = GetScreenHeight();
  const int cloud_wrap_margin = 150;
  int wrap_w = screen_w + 2 * cloud_wrap_margin;
  int wrap_h = screen_h + 2 * cloud_wrap_margin;

  for (int i = 0; i < CLOUD_COUNT; i++) {
    Cloud *c = &cloud_state->clouds[i];
    c->x += dir.x * c->speed * dt;
    c->y += dir.y * c->speed * dt;
    if (c->x < -cloud_wrap_margin) c->x += wrap_w;
    if (c->x > screen_w + cloud_wrap_margin) c->x -= wrap_w;
    if (c->y < -cloud_wrap_margin) c->y += wrap_h;
    if (c->y > screen_h + cloud_wrap_margin) c->y -= wrap_h;
  }
}

void draw_clouds(const CloudState *cloud_state, const Texture_State *texture_state, const WeatherState *weather_state) {
  if (weather_state->current != WEATHER_RAIN) return;

  for (int i = 0; i < CLOUD_COUNT; i++) {
    const Cloud *c = &cloud_state->clouds[i];
    int frame = ((int)(GetTime() * CLOUD_FPS + c->phase)) % FIGURE_MAX_FRAMES;
    const SpriteAsset *sprite = &texture_state->cloud_drift[weather_state->wind_direction][frame];
    float w = sprite->tex.width * sprite->scale;
    float h = sprite->tex.height * sprite->scale;
    Rectangle source = {0, 0, (float)sprite->tex.width, (float)sprite->tex.height};
    Rectangle dest = {c->x - w * sprite->anchor.x, c->y - h * sprite->anchor.y, w, h};
    DrawTexturePro(sprite->tex, source, dest, (Vector2){0, 0}, 0, (Color){255, 255, 255, 215});
  }
}
