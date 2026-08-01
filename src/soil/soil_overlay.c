#include "soil/soil_overlay.h"
#include "soil/soil.h"
#include "raylib.h"

#define SOIL_OVERLAY_TEMP_MIN_C -10.0f
#define SOIL_OVERLAY_TEMP_MAX_C 30.0f

static const char *OVERLAY_LABELS[SOIL_OVERLAY_COUNT] = {
  [SOIL_OVERLAY_NONE] = "Normal",
  [SOIL_OVERLAY_WATER] = "Water",
  [SOIL_OVERLAY_MINERALS] = "Mineral",
  [SOIL_OVERLAY_TEMPERATURE] = "Temp.",
};

static bool draw_overlay_button(int x, int y, int w, int h, const char *label, bool active) {
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

Rectangle soil_overlay_panel_rect(void) {
  return (Rectangle){0, 250, 110, 250};
}

void draw_soil_overlay_bar(SoilOverlayState *state) {
  const int x = 10;
  int y = 270;
  const int width = 90;
  const int height = 30;
  const int padding = 6;
  Rectangle r = soil_overlay_panel_rect();
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);
  DrawText(
    TextFormat("Special\nmaps"),
    x, y, 20, WHITE
  );
  y += height*2;

  for (int i = 0; i < SOIL_OVERLAY_COUNT; i++) {
    state->hovered[i] = draw_overlay_button(x, y, width, height, OVERLAY_LABELS[i], state->current == (SoilOverlay)i);
    y += height + padding;
  }
}

void update_soil_overlay_state(SoilOverlayState *state) {
  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
  for (int i = 0; i < SOIL_OVERLAY_COUNT; i++) {
    if (state->hovered[i]) {
      state->current = (SoilOverlay)i;
      return;
    }
  }
}

typedef struct {
  const char *title;
  const char *low_label;
  const char *high_label;
  Color low;
  Color high;
} OverlayLegend;

static const OverlayLegend OVERLAY_LEGENDS[SOIL_OVERLAY_COUNT] = {
  [SOIL_OVERLAY_WATER] = {
    .title = "Bodenwasser",
    .low_label = "dry (0L)",
    .high_label = "saturated (200L)",
    .low = {230, 140, 40, 255},
    .high = {20, 110, 230, 255},
  },
  [SOIL_OVERLAY_MINERALS] = {
    .title = "Mineralgehalt",
    .low_label = "depleted (0g)",
    .high_label = "fertile (200g)",
    .low = {200, 20, 20, 255},
    .high = {255, 210, 0, 255},
  },
  [SOIL_OVERLAY_TEMPERATURE] = {
    .title = "Bodentemperatur",
    .low = {30, 90, 230, 255},
    .high = {235, 60, 30, 255},
  },
};

void draw_soil_overlay_legend(SoilOverlayState state) {
  if (state.current == SOIL_OVERLAY_NONE) return;
  const OverlayLegend *legend = &OVERLAY_LEGENDS[state.current];

  const int legend_x = 10;
  const int legend_y = 110;
  const int legend_bar_w = 280;
  const int legend_bar_h = 20;
  const int legend_font_size = 16;

  DrawText(legend->title, legend_x, legend_y, legend_font_size, BLACK);

  int bar_y = legend_y + legend_font_size + 6;
  DrawRectangleGradientH(legend_x, bar_y, legend_bar_w, legend_bar_h, legend->low, legend->high);
  DrawRectangleLines(legend_x, bar_y, legend_bar_w, legend_bar_h, BLACK);

  const char *low_label = legend->low_label;
  const char *high_label = legend->high_label;
  if (state.current == SOIL_OVERLAY_TEMPERATURE) {
    low_label = TextFormat("%.0f C", SOIL_OVERLAY_TEMP_MIN_C);
    high_label = TextFormat("%.0f C", SOIL_OVERLAY_TEMP_MAX_C);
  }

  int label_y = bar_y + legend_bar_h + 4;
  DrawText(low_label, legend_x, label_y, legend_font_size, BLACK);
  int high_w = MeasureText(high_label, legend_font_size);
  DrawText(high_label, legend_x + legend_bar_w - high_w, label_y, legend_font_size, BLACK);
}

static Color lerp_color(Color a, Color b, float t) {
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  return (Color){
    (unsigned char)(a.r + (b.r - a.r) * t),
    (unsigned char)(a.g + (b.g - a.g) * t),
    (unsigned char)(a.b + (b.b - a.b) * t),
    (unsigned char)(a.a + (b.a - a.a) * t),
  };
}

Color soil_overlay_tint(SoilOverlayState state, const Tile *tile) {
  if (state.current == SOIL_OVERLAY_NONE) return (Color){0, 0, 0, 0};
  if (tile->type == TILE_WATER) return (Color){0, 190, 210, 255};
  const OverlayLegend *legend = &OVERLAY_LEGENDS[state.current];

  float t;
  switch (state.current) {
    case SOIL_OVERLAY_WATER: t = tile->soil_water / SOIL_WATER_CAPACITY_L; break;
    case SOIL_OVERLAY_MINERALS: t = tile->soil_minerals / SOIL_MINERAL_CAPACITY_G; break;
    case SOIL_OVERLAY_TEMPERATURE:
      t = (tile->soil_temperature - SOIL_OVERLAY_TEMP_MIN_C) / (SOIL_OVERLAY_TEMP_MAX_C - SOIL_OVERLAY_TEMP_MIN_C);
      break;
    default: return (Color){0, 0, 0, 0};
  }
  return lerp_color(legend->low, legend->high, t);
}
