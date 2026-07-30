#include "utils/game_time.h"
#include "raylib.h"
#include <stdio.h>

static const float GAME_SPEED_PRESETS[GAME_SPEED_COUNT] = {1.0f, 2.0f, 5.0f, 10.0f, 20.0f};
static float g_time_scale = 1.0f;

float game_delta_time(void) {
  return GetFrameTime() * g_time_scale;
}

void set_game_time_scale(float scale) {
  g_time_scale = scale;
}

float get_game_time_scale(void) {
  return g_time_scale;
}

static bool draw_speed_button(int x, int y, int w, int h, const char *label, bool active) {
  Vector2 mouse = GetMousePosition();
  Rectangle rect = {(float)x, (float)y, (float)w, (float)h};
  bool hovered = CheckCollisionPointRec(mouse, rect);

  Color bg = active ? (Color){230, 200, 60, 230} : (hovered ? (Color){205, 205, 205, 255} : (Color){220, 220, 220, 255});
  DrawRectangleRounded(rect, 0.2f, 6, bg);
  Color border = active ? GOLD : DARKGRAY;
  DrawRectangleRoundedLinesEx(rect, 0.2f, 6, active ? 3.0f : 2.0f, border);

  int text_w = MeasureText(label, 16);
  DrawText(label, x + (w - text_w) / 2, y + h / 2 - 8, 16, BLACK);
  return hovered;
}

void draw_game_speed_bar(GameSpeedState *state) {
  int x = 1480;
  const int y = 27;
  const int btn_height = 30;
  const int btn_width = 55;
  const int btn_padding = 6;
  DrawText(
    TextFormat("Game Speed"),
    x + 100, 5, 20, WHITE
  );
  for (int i = 0; i < GAME_SPEED_COUNT; i++) {
    char label[16];
    snprintf(label, sizeof(label), "%gx", GAME_SPEED_PRESETS[i]);
    state->hovered[i] = draw_speed_button(x, y, btn_width, btn_height, label, state->current_index == i);
    x += btn_width + btn_padding;
  }
}

void update_game_speed_state(GameSpeedState *state) {
  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
  for (int i = 0; i < GAME_SPEED_COUNT; i++) {
    if (state->hovered[i]) {
      state->current_index = i;
      set_game_time_scale(GAME_SPEED_PRESETS[i]);
      return;
    }
  }
}
