#include "weather/weather.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"
#include "utils/game_time.h"
#include <math.h>

#define WEATHER_ICON_SIZE 50

typedef struct {
  float min_c;
  float max_c;
} MonthTempRange;

static const MonthTempRange MONTH_TEMP_RANGE[12] = {
  {-10.0f,  2.0f}, // January
  { -8.0f,  4.0f}, // February
  { -4.0f, 18.0f}, // March
  {  2.0f, 18.0f}, // April
  {  6.0f, 20.0f}, // May
  { 10.0f, 24.0f}, // June
  { 13.0f, 27.0f}, // July
  { 13.0f, 26.0f}, // August
  {  8.0f, 21.0f}, // September
  {  3.0f, 14.0f}, // October
  { -2.0f,  8.0f}, // November
  { -7.0f,  3.0f}, // December
};

#define WATER_LEVEL_MIN_Z -1.0f
#define WATER_LEVEL_MAX_Z 0.0f

static float water_level_for_temperature(float temperature_celsius) {
  const float water_level_ref_temp_min_c = -10.0f;
  const float water_level_ref_temp_max_c = 27.0f;
  float t = (temperature_celsius - water_level_ref_temp_min_c) /
            (water_level_ref_temp_max_c - water_level_ref_temp_min_c);
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  return WATER_LEVEL_MAX_Z - t * (WATER_LEVEL_MAX_Z - WATER_LEVEL_MIN_Z);
}

void init_weather_state(WeatherState *weather_state, SeasonState season_state) {
  weather_state->current = WEATHER_SUNNY;
  for (int w = 0; w < WEATHER_COUNT; w++) weather_state->hovered[w] = false;
  weather_state->wind_direction = FIGURE_DIR_BACK_RIGHT;
  weather_state->wind_lever_hovered = false;
  MonthTempRange range = MONTH_TEMP_RANGE[season_state.month];
  weather_state->temperature_celsius = (range.min_c + range.max_c) * 0.5f;
  weather_state->water_level_z = water_level_for_temperature(weather_state->temperature_celsius);
}

static bool draw_weather_icon_frame(int x, int y, bool active) {
  Vector2 mouse = GetMousePosition();
  Rectangle icon_rect = {x, y, WEATHER_ICON_SIZE, WEATHER_ICON_SIZE};
  bool hovered = CheckCollisionPointRec(mouse, icon_rect);
  bool pressed = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  int press_offset = pressed ? 2 : 0;

  static Color col_default = (Color){220, 220, 220, 255};
  static Color col_pressed = (Color){190, 190, 190, 255};
  static Color col_hovered = (Color){205, 205, 205, 255};
  Color icon_bg = col_default;
  if (pressed) {
    icon_bg = col_pressed;
  } else if (hovered) {
    icon_bg = col_hovered;
  }

  Rectangle icon_rect_draw = {
    x + press_offset,
    y + press_offset,
    WEATHER_ICON_SIZE,
    WEATHER_ICON_SIZE
  };
  DrawRectangleRounded(icon_rect_draw, 0.2f, 8, icon_bg);
  Color border = active ? GOLD : DARKGRAY;
  DrawRectangleRoundedLinesEx(icon_rect_draw, 0.2f, 8, active ? 3.0f : 2.0f, border);
  return hovered;
}

static void draw_sun_glyph(int x, int y) {
  float cx = x + WEATHER_ICON_SIZE / 2.0f;
  float cy = y + WEATHER_ICON_SIZE / 2.0f;
  float radius = WEATHER_ICON_SIZE * 0.22f;
  Color col = (Color){235, 170, 30, 255};
  DrawCircle((int)cx, (int)cy, radius, col);
  int n_rays = 8;
  float ray_len = WEATHER_ICON_SIZE * 0.16f;
  for (int i = 0; i < n_rays; i++) {
    float angle = i * (2.0f * PI / n_rays);
    Vector2 inner = {cx + cosf(angle) * (radius + 4.0f), cy + sinf(angle) * (radius + 4.0f)};
    Vector2 outer = {cx + cosf(angle) * (radius + 4.0f + ray_len), cy + sinf(angle) * (radius + 4.0f + ray_len)};
    DrawLineEx(inner, outer, 3.0f, col);
  }
}

static void draw_wind_direction_glyph(int x, int y, FigureDirection dir) {
  float cx = x + WEATHER_ICON_SIZE / 2.0f;
  float cy = y + WEATHER_ICON_SIZE / 2.0f;
  Vector2 v = screen_direction_for_facing(dir);
  float arm = WEATHER_ICON_SIZE * 0.3f;
  Vector2 tip = {cx + v.x * arm, cy + v.y * arm};
  Vector2 tail = {cx - v.x * arm, cy - v.y * arm};
  Color col = (Color){90, 130, 200, 255};
  DrawLineEx(tail, tip, 3.0f, col);

  Vector2 perp = {-v.y, v.x};
  float head = WEATHER_ICON_SIZE * 0.12f;
  Vector2 left = {tip.x - v.x * head + perp.x * head, tip.y - v.y * head + perp.y * head};
  Vector2 right = {tip.x - v.x * head - perp.x * head, tip.y - v.y * head - perp.y * head};
  DrawTriangle(tip, left, right, col);
}

static void draw_wind_glyph(int x, int y) {
  Color col = (Color){90, 130, 200, 255};
  float cx = x + WEATHER_ICON_SIZE / 2.0f;
  float cy = y + WEATHER_ICON_SIZE / 2.0f;
  float half_w = WEATHER_ICON_SIZE * 0.3f;
  const int segments = 10;
  for (int line = -1; line <= 1; line++) {
    float row_y = cy + line * (WEATHER_ICON_SIZE * 0.18f);
    float row_w = half_w * 2.0f - fabsf((float)line) * (WEATHER_ICON_SIZE * 0.08f);
    Vector2 prev = {cx - row_w / 2.0f, row_y};
    for (int s = 1; s <= segments; s++) {
      float t = (float)s / segments;
      float wx = cx - row_w / 2.0f + row_w * t;
      float wy = row_y + sinf(t * PI * 2.0f) * 4.0f;
      Vector2 cur = {wx, wy};
      DrawLineEx(prev, cur, 3.0f, col);
      prev = cur;
    }
  }
}

static void draw_rain_glyph(int x, int y) {
  float cx = x + WEATHER_ICON_SIZE / 2.0f;
  float cy = y + WEATHER_ICON_SIZE / 2.0f;
  Color cloud_col = (Color){170, 175, 185, 255};
  Color drop_col = (Color){90, 130, 200, 255};

  float cloud_y = cy - WEATHER_ICON_SIZE * 0.14f;
  DrawCircle((int)(cx - WEATHER_ICON_SIZE * 0.14f), (int)cloud_y, WEATHER_ICON_SIZE * 0.12f, cloud_col);
  DrawCircle((int)cx, (int)(cloud_y - WEATHER_ICON_SIZE * 0.06f), WEATHER_ICON_SIZE * 0.15f, cloud_col);
  DrawCircle((int)(cx + WEATHER_ICON_SIZE * 0.15f), (int)cloud_y, WEATHER_ICON_SIZE * 0.11f, cloud_col);
  Rectangle cloud_belly = {
    cx - WEATHER_ICON_SIZE * 0.26f, cloud_y - WEATHER_ICON_SIZE * 0.02f,
    WEATHER_ICON_SIZE * 0.52f, WEATHER_ICON_SIZE * 0.16f,
  };
  DrawRectangleRec(cloud_belly, cloud_col);

  for (int i = -1; i <= 1; i++) {
    float dx = cx + i * (WEATHER_ICON_SIZE * 0.16f);
    float dy = cy + WEATHER_ICON_SIZE * 0.14f;
    DrawLineEx((Vector2){dx, dy}, (Vector2){dx - WEATHER_ICON_SIZE * 0.05f, dy + WEATHER_ICON_SIZE * 0.16f}, 2.5f, drop_col);
  }
}

void draw_weather_bar(WeatherState *weather_state) {
  int x = 10;
  const int y = 10;
  const int padding = 10;

  weather_state->hovered[WEATHER_SUNNY] = draw_weather_icon_frame(x, y, weather_state->current == WEATHER_SUNNY);
  draw_sun_glyph(x, y);

  x += WEATHER_ICON_SIZE + padding;
  weather_state->hovered[WEATHER_WINDY] = draw_weather_icon_frame(x, y, weather_state->current == WEATHER_WINDY);
  draw_wind_glyph(x, y);

  x += WEATHER_ICON_SIZE + padding;
  weather_state->hovered[WEATHER_RAIN] = draw_weather_icon_frame(x, y, weather_state->current == WEATHER_RAIN);
  draw_rain_glyph(x, y);

  x += WEATHER_ICON_SIZE + padding;
  weather_state->wind_lever_hovered = draw_weather_icon_frame(x, y, false);
  draw_wind_direction_glyph(x, y, weather_state->wind_direction);

  x += WEATHER_ICON_SIZE + padding;
  DrawText(TextFormat("%.0f°C (U/D)", weather_state->temperature_celsius), x, y + WEATHER_ICON_SIZE / 2 - 10, 20, WHITE);

  x += 150;
  DrawLineEx((Vector2){x, 0}, (Vector2){x, 65}, 2.0f, BLACK);
}

static float rain_cooling_offset(int month) {
  bool summer = month >= 5 && month <= 7; // June, July, August (0-indexed)
  return summer ? 2.0f : 0.5f;
}

void update_weather_state(WeatherState *weather_state, SeasonState season_state) {
  const float temperature_step_c = 1.0f;
  MonthTempRange range = MONTH_TEMP_RANGE[season_state.month];

  float target;
  float rate;
  switch (weather_state->current) {
    case WEATHER_SUNNY:
      target = range.max_c;
      rate = 0.05f;
      break;
    case WEATHER_RAIN:
      target = (range.min_c + range.max_c) * 0.5f - rain_cooling_offset(season_state.month);
      rate = 0.05f;
      break;
    case WEATHER_WINDY:
    default:
      target = (range.min_c + range.max_c) * 0.5f;
      rate = 0.02f;
      break;
  }
  weather_state->temperature_celsius += (target - weather_state->temperature_celsius) * rate * game_delta_time();

  if (IsKeyPressed(KEY_D)) {
    weather_state->temperature_celsius -= temperature_step_c;
    if (weather_state->temperature_celsius < range.min_c) weather_state->temperature_celsius = range.min_c;
  }
  if (IsKeyPressed(KEY_U)) {
    weather_state->temperature_celsius += temperature_step_c;
    if (weather_state->temperature_celsius > range.max_c) weather_state->temperature_celsius = range.max_c;
  }

  const float evaporation_rate = 0.05f;
  const float rain_rate = 0.7f;
  if (weather_state->current == WEATHER_RAIN) {
    weather_state->water_level_z += (WATER_LEVEL_MAX_Z - weather_state->water_level_z) * rain_rate * game_delta_time();
  } else {
    float evaporation_target = water_level_for_temperature(weather_state->temperature_celsius);
    weather_state->water_level_z += (evaporation_target - weather_state->water_level_z) * evaporation_rate * game_delta_time();
  }
  if (weather_state->water_level_z < WATER_LEVEL_MIN_Z) weather_state->water_level_z = WATER_LEVEL_MIN_Z;
  if (weather_state->water_level_z > WATER_LEVEL_MAX_Z) weather_state->water_level_z = WATER_LEVEL_MAX_Z;

  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;

  if (weather_state->wind_lever_hovered) {
    weather_state->wind_direction = (FigureDirection)((weather_state->wind_direction + 1) % FIGURE_DIR_COUNT);
    return;
  }

  for (Weather w = 0; w < WEATHER_COUNT; w++) {
    if (weather_state->hovered[w]) {
      weather_state->current = w;
      break;
    }
  }
}
