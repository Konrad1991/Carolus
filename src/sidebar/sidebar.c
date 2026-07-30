#include "sidebar/sidebar.h"

float min_float(float a, float b) {
  return (a < b) ? a : b;
}

void init_sidebar_state(SidebarState* sidebar_state, Texture_State *texture_state,
                        const int SIDEBAR_WIDTH, const int SIDEBAR_PADDING,
                        const int FONT_SIZE, const float ICON_WIDTH, const float ICON_HEIGHT) {
  sidebar_state->SIDEBAR_WIDTH = SIDEBAR_WIDTH;
  sidebar_state->SIDEBAR_PADDING = SIDEBAR_PADDING;
  sidebar_state->FONT_SIZE = FONT_SIZE;
  sidebar_state->ICON_WIDTH = ICON_WIDTH;
  sidebar_state->ICON_HEIGHT = ICON_HEIGHT;
  sidebar_state->SIDEBAR_HEIGHT = GetScreenHeight();
  sidebar_state->SIDEBAR_X = GetScreenWidth() - sidebar_state->SIDEBAR_WIDTH;
  sidebar_state->si_h1 = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->house1[BUILDING_DIR_SE].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->house1[BUILDING_DIR_SE].tex.height)
  );
  sidebar_state->si_h2 = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->house2[BUILDING_DIR_SE].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->house2[BUILDING_DIR_SE].tex.height)
  );
  sidebar_state->si_road = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->road_flat[0].width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->road_flat[0].height)
  );
  sidebar_state->si_bridge = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->bridge[0].width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->bridge[0].height)
  );
  sidebar_state->si_farmer1 = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex.height)
  );
  sidebar_state->si_farmer2 = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex.height)
  );
  sidebar_state->si_ox = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->ox_stand[FIGURE_DIR_FRONT].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->ox_stand[FIGURE_DIR_FRONT].tex.height)
  );
  sidebar_state->si_oak = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->oak[0][BUILDING_DIR_SE].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->oak[0][BUILDING_DIR_SE].tex.height)
  );
  sidebar_state->si_wheat = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->wheat_tuft_idle[WHEAT_STAGE_RIPE][FIGURE_DIR_FRONT].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->wheat_tuft_idle[WHEAT_STAGE_RIPE][FIGURE_DIR_FRONT].tex.height)
  );
  sidebar_state->si_grass = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->grass_tuft_idle[0][FIGURE_DIR_FRONT].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->grass_tuft_idle[0][FIGURE_DIR_FRONT].tex.height)
  );
  sidebar_state->si_mansus = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->mansus_area_icon.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->mansus_area_icon.height)
  );
  sidebar_state->si_delete = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->remove_icon.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->remove_icon.height)
  );
}
void update_sidebar_state(SidebarState* sidebar_state) {
  sidebar_state->SIDEBAR_HEIGHT = GetScreenHeight();
  sidebar_state->SIDEBAR_X = GetScreenWidth() - sidebar_state->SIDEBAR_WIDTH;
}

// Draw sidebar
// -----------------------------------------------------------------
bool draw_icon(SidebarState sidebar_state, Texture2D tex, float si_h, int x, int y) {
  Vector2 mouse = GetMousePosition();
  Rectangle icon_rect = {x, y, sidebar_state.ICON_WIDTH, sidebar_state.ICON_HEIGHT};
  bool hovered = CheckCollisionPointRec(mouse, icon_rect);
  bool pressed = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  int press_offset = pressed ? 2 : 0;

  static Color col_default = (Color){220, 220, 220, 255}; 
  static Color col_pressed = (Color){190, 190, 190, 255};
  static Color col_hovered = (Color){205, 205, 205, 255};

  Color icon_bg = col_default;
  if (pressed) {
    icon_bg = col_pressed;
  } else if (!pressed && hovered){
    icon_bg = col_hovered;
  }

  Rectangle icon_rect_draw = {
    x + press_offset,
    y + press_offset,
    sidebar_state.ICON_WIDTH,
    sidebar_state.ICON_HEIGHT
  };
  DrawRectangleRounded(icon_rect_draw, 0.2f, 8, icon_bg);

  float tex_w = tex.width * si_h;
  float tex_h = tex.height * si_h;
  Vector2 pos = {
    x + press_offset + (sidebar_state.ICON_WIDTH - tex_w) / 2.0f,
    y + press_offset + (sidebar_state.ICON_HEIGHT - tex_h) / 2.0f,
  };
  DrawTextureEx(tex, pos, 0, si_h, WHITE);
  DrawRectangleRoundedLinesEx(icon_rect_draw, 0.2f, 8, 2.0f, DARKGRAY);
  return hovered;
}

void draw_sidebar(SidebarState* sidebar_state, Texture_State *texture_state, bool hovered[MODE_COUNT]) {
  update_sidebar_state(sidebar_state);
  Rectangle r = {
    sidebar_state->SIDEBAR_X, 0,
    sidebar_state->SIDEBAR_WIDTH, sidebar_state->SIDEBAR_HEIGHT
  };
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);
  const int sb_text_y = 10;
  const int sb_x = sidebar_state->SIDEBAR_X + sidebar_state->SIDEBAR_PADDING;
  const int sb_y1 = sb_text_y + sidebar_state->FONT_SIZE;
  const int sb_y2 = sb_y1 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y3 = sb_y2 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y4 = sb_y3 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y5 = sb_y4 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y6 = sb_y5 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y7 = sb_y6 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y8 = sb_y7 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y9 = sb_y8 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y10 = sb_y9 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y11 = sb_y10 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y12 = sb_y11 + (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  hovered[BUILD_LIVING_HOUSE] = draw_icon(*sidebar_state, texture_state->house1[BUILDING_DIR_SE].tex, sidebar_state->si_h1, sb_x, sb_y1);
  hovered[BUILD_BARN] = draw_icon(*sidebar_state, texture_state->house2[BUILDING_DIR_SE].tex, sidebar_state->si_h2, sb_x, sb_y2);
  hovered[BUILD_FIELD] = draw_icon(*sidebar_state, texture_state->wheat_tuft_idle[WHEAT_STAGE_RIPE][FIGURE_DIR_FRONT].tex, sidebar_state->si_wheat, sb_x, sb_y3);
  hovered[BUILD_ROAD] = draw_icon(*sidebar_state, texture_state->road_flat[0], sidebar_state->si_road, sb_x, sb_y4);
  hovered[BUILD_BRIDGE] = draw_icon(*sidebar_state, texture_state->bridge[0], sidebar_state->si_bridge, sb_x, sb_y5);
  hovered[BUILD_FARMER1] = draw_icon(*sidebar_state, texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex, sidebar_state->si_farmer1, sb_x, sb_y6);
  hovered[BUILD_FARMER2] = draw_icon(*sidebar_state, texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex, sidebar_state->si_farmer2, sb_x, sb_y7);
  hovered[BUILD_OX] = draw_icon(*sidebar_state, texture_state->ox_stand[FIGURE_DIR_FRONT].tex, sidebar_state->si_ox, sb_x, sb_y8);
  hovered[BUILD_OAK] = draw_icon(*sidebar_state, texture_state->oak[0][BUILDING_DIR_SE].tex, sidebar_state->si_oak, sb_x, sb_y9);
  hovered[BUILD_GRASS] = draw_icon(*sidebar_state, texture_state->grass_tuft_idle[0][FIGURE_DIR_FRONT].tex, sidebar_state->si_grass, sb_x, sb_y10);
  hovered[BUILD_MANSUS] = draw_icon(*sidebar_state, texture_state->mansus_area_icon, sidebar_state->si_mansus, sb_x, sb_y11);
  hovered[BUILD_DELETE] = draw_icon(*sidebar_state, texture_state->remove_icon, sidebar_state->si_delete, sb_x, sb_y12);
}
