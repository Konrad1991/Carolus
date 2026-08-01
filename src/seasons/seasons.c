#include "seasons/seasons.h"
#include "drawing/drawing_helper.h"
#include "raylib.h"
#include "utils/game_time.h"
#include <math.h>

static const Season SEASON_FOR_MONTH[12] = {
  WINTER, WINTER, WINTER,
  SPRING, SPRING, SPRING,
  SUMMER, SUMMER, SUMMER,
  AUTUMN, AUTUMN, AUTUMN
};

SeasonBlend resolve_season_blend(SeasonState s) {
  bool transition = (s.month % 3) == 2;
  Season base = SEASON_FOR_MONTH[s.month];
  Season fade_in = transition ? SEASON_FOR_MONTH[(s.month + 1) % 12] : base;
  float alpha = transition ? s.month_progress : 0.0f;
  return (SeasonBlend){base, fade_in, alpha};
}

void update_season_state(SeasonState* s) {
  s->month_progress += game_delta_time() / SECONDS_PER_MONTH;
  if (s->month_progress >= 1.0f) {
    s->month_progress -= 1.0f;
    s->month = (s->month + 1) % 12;
    if (s->month == 0) s->year++;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (s->month_up_hovered) {
      s->month = (s->month + 1) % 12;
      s->month_progress = 0.0f;
      if (s->month == 0) s->year++;
    }
    if (s->month_down_hovered) {
      if (s->month == 0) s->year--;
      s->month = (s->month + 11) % 12;
      s->month_progress = 0.0f;
    }
  }
}

static bool draw_month_arrow(Rectangle r, bool pointing_up) {
  bool hovered = CheckCollisionPointRec(GetMousePosition(), r);
  DrawRectangleRounded(r, 0.2f, 6, hovered ? LIGHTGRAY : GRAY);
  float cx = r.x + r.width / 2.0f, cy = r.y + r.height / 2.0f;
  float half = r.width * 0.25f;
  if (pointing_up) {
    DrawTriangle((Vector2){cx, cy - half}, (Vector2){cx - half, cy + half}, (Vector2){cx + half, cy + half}, BLACK);
  } else {
    DrawTriangle((Vector2){cx + half, cy - half}, (Vector2){cx - half, cy - half}, (Vector2){cx, cy + half}, BLACK);
  }
  return hovered;
}

float topbar_scale(void) {
  const float reference_screen_width = 1920.0f;
  return (float)GetScreenWidth() / reference_screen_width;
}

float topbar_vertical_scale(void) {
  const float vertical_compression = 0.9f;
  return topbar_scale() * vertical_compression;
}

Rectangle topbar_panel_rect(void) {
  return (Rectangle){0, 0, 1800.0f * topbar_scale(), 65.0f * topbar_vertical_scale()};
}

static const int CAROLUS_START_YEAR_AD = 500;

void draw_season_bar(SeasonState *s) {
  const char* months[12] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "Oktober", "November", "December"
  };
  float scale = topbar_scale();
  float vscale = topbar_vertical_scale();
  Rectangle r = topbar_panel_rect();
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);
  DrawText(
    TextFormat("Month %s, Year %d AD", months[s->month], CAROLUS_START_YEAR_AD + s->year),
    (int)(400 * scale), (int)(25 * vscale), (int)(20 * vscale), WHITE
  );

  Rectangle up_rect   = {680 * scale, 5 * vscale,  30 * scale, 25 * vscale};
  Rectangle down_rect = {680 * scale, 35 * vscale, 30 * scale, 25 * vscale};
  s->month_up_hovered   = draw_month_arrow(up_rect, true);
  s->month_down_hovered = draw_month_arrow(down_rect, false);

  DrawLineEx((Vector2){720 * scale, 0}, (Vector2){720 * scale, r.height}, 2.0f, BLACK);
}
