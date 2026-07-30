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
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (s->month_up_hovered)   { s->month = (s->month + 1) % 12;  s->month_progress = 0.0f; }
    if (s->month_down_hovered) { s->month = (s->month + 11) % 12; s->month_progress = 0.0f; }
  }
  s->month_progress += game_delta_time() / SECONDS_PER_MONTH;
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

void draw_season_bar(SeasonState *s) {
  const char* months[12] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "Oktober", "November", "December"
  };
  Rectangle r = {
    .x = 0,
    .y = 0,
    .width = 1800,
    .height = 65
  };
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);
  DrawText(
    TextFormat("Month %s", months[s->month]),
    500, 25, 20, WHITE
  );

  Rectangle up_rect   = {680, 5,  30, 25};
  Rectangle down_rect = {680, 35, 30, 25};
  s->month_up_hovered   = draw_month_arrow(up_rect, true);
  s->month_down_hovered = draw_month_arrow(down_rect, false);

  DrawLineEx((Vector2){720, 0}, (Vector2){720, 65}, 2.0f, BLACK);

}
