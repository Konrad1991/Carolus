#include "weather/weather_scenario.h"
#include "seasons/seasons.h"
#include "soil/soil.h"
#include "utils/game_time.h"
#include "raylib.h"
#include <stddef.h>

typedef struct {
  Weather weather;
  float duration;
} ScenarioPhase;

static const ScenarioPhase BAD_YEAR_PHASES[] = {
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 40.0f}, {WEATHER_RAIN, 8.0f},
  {WEATHER_SUNNY, 300.0f},
};

static const ScenarioPhase GOOD_YEAR_PHASES[] = {
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 25.0f}, {WEATHER_RAIN, 16.0f},
  {WEATHER_SUNNY, 55.0f}, {WEATHER_RAIN, 10.0f},
  {WEATHER_SUNNY, 55.0f}, {WEATHER_RAIN, 10.0f},
  {WEATHER_SUNNY, 300.0f},
};

static const ScenarioPhase MEDIOCRE_YEAR_PHASES[] = {
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 20.0f}, {WEATHER_RAIN, 12.0f},
  {WEATHER_SUNNY, 60.0f}, {WEATHER_RAIN, 6.0f},
  {WEATHER_SUNNY, 60.0f}, {WEATHER_RAIN, 6.0f},
  {WEATHER_SUNNY, 300.0f},
};

static float average_field_soil_water(Map *map, const GameState *game_state) {
  double sum = 0.0;
  int count = 0;
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      for (int y = f->corners_field[0][1]; y <= f->corners_field[1][1]; y++) {
        for (int x = f->corners_field[0][0]; x <= f->corners_field[1][0]; x++) {
          const Tile *t = map_tile(map, x, y);
          if (!t) continue;
          sum += t->soil_water;
          count++;
        }
      }
    }
  }
  return count > 0 ? (float)(sum / count) : SOIL_WATER_CAPACITY_L;
}

static void update_perfect_year(WeatherState *weather_state, WeatherScenarioState *scenario_state, Map *map, const GameState *game_state) {
  const float perfect_year_vegetative_seconds = SECONDS_PER_MONTH * 4.0f;
  if (scenario_state->elapsed >= perfect_year_vegetative_seconds) {
    weather_state->current = WEATHER_SUNNY;
    return;
  }

  scenario_state->sub_timer -= game_delta_time();
  if (scenario_state->sub_timer > 0.0f) return;

  const float perfect_year_water_threshold = 20.0f;
  weather_state->current = (average_field_soil_water(map, game_state) < perfect_year_water_threshold) ? WEATHER_RAIN : WEATHER_SUNNY;
  const float perfect_year_min_dwell_seconds = 10.0f;
  scenario_state->sub_timer = perfect_year_min_dwell_seconds;
}

static void run_scenario_phases(WeatherState *weather_state, WeatherScenarioState *scenario_state, const ScenarioPhase *phases, int count) {
  float t = scenario_state->elapsed;
  for (int i = 0; i < count; i++) {
    if (t < phases[i].duration || i == count - 1) {
      weather_state->current = phases[i].weather;
      return;
    }
    t -= phases[i].duration;
  }
}

void update_weather_scenario(WeatherState *weather_state, WeatherScenarioState *scenario_state, Map *map, const GameState *game_state) {
  if (scenario_state->current == WEATHER_SCENARIO_NONE) return;

  scenario_state->elapsed += game_delta_time();

  if (scenario_state->current == WEATHER_SCENARIO_PERFECT_YEAR) {
    update_perfect_year(weather_state, scenario_state, map, game_state);
  } else if (scenario_state->current == WEATHER_SCENARIO_GOOD_YEAR) {
    run_scenario_phases(weather_state, scenario_state, GOOD_YEAR_PHASES, sizeof(GOOD_YEAR_PHASES) / sizeof(GOOD_YEAR_PHASES[0]));
  } else if (scenario_state->current == WEATHER_SCENARIO_MEDIOCRE_YEAR) {
    run_scenario_phases(weather_state, scenario_state, MEDIOCRE_YEAR_PHASES, sizeof(MEDIOCRE_YEAR_PHASES) / sizeof(MEDIOCRE_YEAR_PHASES[0]));
  } else if (scenario_state->current == WEATHER_SCENARIO_BAD_YEAR) {
    run_scenario_phases(weather_state, scenario_state, BAD_YEAR_PHASES, sizeof(BAD_YEAR_PHASES) / sizeof(BAD_YEAR_PHASES[0]));
  }
}

static void select_weather_scenario(WeatherScenarioState *scenario_state, WeatherScenario scenario) {
  scenario_state->current = scenario;
  scenario_state->elapsed = 0.0f;
  scenario_state->sub_timer = 0.0f;
}

void cycle_weather_scenario(WeatherScenarioState *scenario_state) {
  select_weather_scenario(scenario_state, (WeatherScenario)((scenario_state->current + 1) % WEATHER_SCENARIO_COUNT));
}

static const char *SCENARIO_LABELS[WEATHER_SCENARIO_COUNT] = {
  [WEATHER_SCENARIO_NONE] = "manual",
  [WEATHER_SCENARIO_PERFECT_YEAR] = "perfect year",
  [WEATHER_SCENARIO_GOOD_YEAR] = "good year",
  [WEATHER_SCENARIO_MEDIOCRE_YEAR] = "mediocre year",
  [WEATHER_SCENARIO_BAD_YEAR] = "bad year",
};

static bool draw_scenario_button(int x, int y, int w, int h, float vscale, const char *label, bool active) {
  Vector2 mouse = GetMousePosition();
  Rectangle rect = {(float)x, (float)y, (float)w, (float)h};
  bool hovered = CheckCollisionPointRec(mouse, rect);
  Color bg = active ? (Color){230, 200, 60, 230} : (hovered ? (Color){205, 205, 205, 255} : (Color){220, 220, 220, 255});
  DrawRectangleRounded(rect, 0.2f, 6, bg);
  Color border = active ? GOLD : DARKGRAY;
  DrawRectangleRoundedLinesEx(rect, 0.2f, 6, active ? 3.0f : 2.0f, border);
  int font_size = (int)(16 * vscale);
  int text_w = MeasureText(label, font_size);
  DrawText(label, x + (w - text_w) / 2, y + h / 2 - font_size / 2, font_size, BLACK);
  return hovered;
}

void draw_weather_scenario_bar(WeatherScenarioState *state) {
  float scale = topbar_scale();
  float vscale = topbar_vertical_scale();
  int x = (int)(750 * scale);
  int y = (int)(27 * vscale);
  int btn_width = (int)(130 * scale);
  int btn_height = (int)(30 * vscale);
  int btn_padding = (int)(10 * scale);

  for (int i = 0; i < WEATHER_SCENARIO_COUNT; i++) {
    state->hovered[i] = draw_scenario_button(x, y, btn_width, btn_height, vscale, SCENARIO_LABELS[i], state->current == (WeatherScenario)i);
    x += btn_width + btn_padding;
  }
  DrawLineEx((Vector2){x, 0}, (Vector2){x, 65 * vscale}, 2.0f, BLACK);
}

void update_weather_scenario_state(WeatherScenarioState *state) {
  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
  for (int i = 0; i < WEATHER_SCENARIO_COUNT; i++) {
    if (state->hovered[i]) {
      select_weather_scenario(state, (WeatherScenario)i);
      return;
    }
  }
}
