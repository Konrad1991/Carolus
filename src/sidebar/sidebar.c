#include "sidebar/sidebar.h"

float min_float(float a, float b) {
  return (a < b) ? a : b;
}

// Category -> member build modes (Pharaoh-style: one main-sidebar icon opens
// a flyout of the related build modes, instead of every mode getting its own
// permanent row - the flat list stopped scaling once it passed ~12 icons).
static const Mode CATEGORY_MODES_FARMING[] = { BUILD_HIVE, BUILD_FIELD };
static const Mode CATEGORY_MODES_MANSUS[]  = { BUILD_MANSUS, BUILD_BARN, BUILD_LIVING_HOUSE, BUILD_WELL };
static const Mode CATEGORY_MODES_DEV[]     = { BUILD_FARMER1, BUILD_FARMER2, BUILD_OX, BUILD_OAK, BUILD_GRASS };

static const Mode *CATEGORY_MODES[BUILD_CATEGORY_COUNT] = {
  [BUILD_CATEGORY_FARMING] = CATEGORY_MODES_FARMING,
  [BUILD_CATEGORY_MANSUS]  = CATEGORY_MODES_MANSUS,
  [BUILD_CATEGORY_DEV]     = CATEGORY_MODES_DEV,
};
static const int CATEGORY_MODE_COUNTS[BUILD_CATEGORY_COUNT] = {
  [BUILD_CATEGORY_FARMING] = 2,
  [BUILD_CATEGORY_MANSUS]  = 4,
  [BUILD_CATEGORY_DEV]     = 5,
};

// Icons the main sidebar column always shows, outside any category flyout.
#define SIDEBAR_MAIN_ICON_COUNT 7

void init_sidebar_state(SidebarState* sidebar_state, const Texture_State *texture_state,
                        const int SIDEBAR_WIDTH, const int SIDEBAR_PADDING,
                        const int FONT_SIZE, const float ICON_WIDTH, const float ICON_HEIGHT) {
  const float reference_screen_width = 1920.0f;
  // Derived from the main column's row count (categories collapse the rest
  // into flyouts, which are never taller than the main column) instead of a
  // fixed magic number, so the sidebar keeps fitting the screen as icons get
  // added or removed instead of silently clipping the last row.
  const int n_icon_rows = SIDEBAR_MAIN_ICON_COUNT;
  const float reference_icon_stack_height = (float)(SIDEBAR_PADDING + FONT_SIZE + n_icon_rows * ((int)ICON_HEIGHT + SIDEBAR_PADDING));
  float width_scale = (float)GetScreenWidth() / reference_screen_width;
  float height_scale = (float)GetScreenHeight() / reference_icon_stack_height;
  // Icons used to scale purely off height_scale while the sidebar panel itself
  // scaled off width_scale - two independent factors that only happen to agree
  // at the 1920-wide reference resolution. Off that (e.g. a screen taller
  // relative to its width than the reference), height_scale outgrew width_scale
  // and icons ballooned to fill nearly the whole sidebar width with no margin.
  // Capping icon scale at width_scale too keeps them from ever outgrowing the
  // panel that contains them, on any resolution.
  float icon_scale = min_float(width_scale, height_scale);

  sidebar_state->SIDEBAR_WIDTH = (int)(SIDEBAR_WIDTH * width_scale);
  sidebar_state->SIDEBAR_PADDING = (int)(SIDEBAR_PADDING * icon_scale);
  sidebar_state->FONT_SIZE = (int)(FONT_SIZE * icon_scale);
  sidebar_state->ICON_WIDTH = ICON_WIDTH * icon_scale;
  sidebar_state->ICON_HEIGHT = ICON_HEIGHT * icon_scale;
  sidebar_state->SIDEBAR_HEIGHT = GetScreenHeight();
  sidebar_state->SIDEBAR_X = GetScreenWidth() - sidebar_state->SIDEBAR_WIDTH;
  sidebar_state->open_category = -1;
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
  sidebar_state->si_clear_forest = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->axe_cursor.tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->axe_cursor.tex.height)
  );
  sidebar_state->si_mansus = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->mansus_area_icon.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->mansus_area_icon.height)
  );
  sidebar_state->si_delete = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->remove_icon.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->remove_icon.height)
  );
  sidebar_state->si_hive = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->hive.tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->hive.tex.height)
  );
  sidebar_state->si_well = min_float(
    (float)(sidebar_state->ICON_WIDTH / texture_state->well[FIGURE_DIR_FRONT].tex.width),
    (float)(sidebar_state->ICON_HEIGHT / texture_state->well[FIGURE_DIR_FRONT].tex.height)
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

static Texture2D category_member_tex(const Texture_State *texture_state, Mode m) {
  switch (m) {
    case BUILD_HIVE: return texture_state->hive.tex;
    case BUILD_FIELD: return texture_state->wheat_tuft_idle[WHEAT_STAGE_RIPE][FIGURE_DIR_FRONT].tex;
    case BUILD_MANSUS: return texture_state->mansus_area_icon;
    case BUILD_BARN: return texture_state->house2[BUILDING_DIR_SE].tex;
    case BUILD_LIVING_HOUSE: return texture_state->house1[BUILDING_DIR_SE].tex;
    case BUILD_WELL: return texture_state->well[FIGURE_DIR_FRONT].tex;
    case BUILD_FARMER1: return texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex;
    case BUILD_FARMER2: return texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex;
    case BUILD_OX: return texture_state->ox_stand[FIGURE_DIR_FRONT].tex;
    case BUILD_OAK: return texture_state->oak[0][BUILDING_DIR_SE].tex;
    case BUILD_GRASS: return texture_state->grass_tuft_idle[0][FIGURE_DIR_FRONT].tex;
    default: return texture_state->remove_icon;
  }
}

static float category_member_scale(const SidebarState *sidebar_state, Mode m) {
  switch (m) {
    case BUILD_HIVE: return sidebar_state->si_hive;
    case BUILD_FIELD: return sidebar_state->si_wheat;
    case BUILD_MANSUS: return sidebar_state->si_mansus;
    case BUILD_BARN: return sidebar_state->si_h2;
    case BUILD_LIVING_HOUSE: return sidebar_state->si_h1;
    case BUILD_WELL: return sidebar_state->si_well;
    case BUILD_FARMER1: return sidebar_state->si_farmer1;
    case BUILD_FARMER2: return sidebar_state->si_farmer2;
    case BUILD_OX: return sidebar_state->si_ox;
    case BUILD_OAK: return sidebar_state->si_oak;
    case BUILD_GRASS: return sidebar_state->si_grass;
    default: return 1.0f;
  }
}

static int category_flyout_top_y(const SidebarState *sidebar_state) {
  return 10 + sidebar_state->FONT_SIZE;
}

static Rectangle category_flyout_rect(const SidebarState *sidebar_state, BuildCategory category) {
  int n = CATEGORY_MODE_COUNTS[category];
  int row_step = (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  int flyout_w = (int)sidebar_state->ICON_WIDTH + sidebar_state->SIDEBAR_PADDING * 2;
  int flyout_h = n * row_step - sidebar_state->SIDEBAR_PADDING + sidebar_state->SIDEBAR_PADDING * 2;
  int flyout_x = sidebar_state->SIDEBAR_X - flyout_w - sidebar_state->SIDEBAR_PADDING;
  int flyout_y = category_flyout_top_y(sidebar_state) - sidebar_state->SIDEBAR_PADDING;
  return (Rectangle){(float)flyout_x, (float)flyout_y, (float)flyout_w, (float)flyout_h};
}

Rectangle sidebar_flyout_rect(const SidebarState *sidebar_state) {
  if (sidebar_state->open_category < 0) return (Rectangle){0, 0, 0, 0};
  return category_flyout_rect(sidebar_state, (BuildCategory)sidebar_state->open_category);
}

static void draw_category_flyout(SidebarState *sidebar_state, const Texture_State *texture_state,
                                 bool hovered[MODE_COUNT], BuildCategory category, int top_y) {
  int n = CATEGORY_MODE_COUNTS[category];
  const Mode *members = CATEGORY_MODES[category];
  int row_step = (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;

  Rectangle r = category_flyout_rect(sidebar_state, category);
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);

  int icon_x = (int)r.x + sidebar_state->SIDEBAR_PADDING;
  int y = top_y;
  for (int i = 0; i < n; i++) {
    Mode m = members[i];
    hovered[m] = draw_icon(*sidebar_state, category_member_tex(texture_state, m), category_member_scale(sidebar_state, m), icon_x, y);
    y += row_step;
  }
}

void draw_sidebar(SidebarState* sidebar_state, const Texture_State *texture_state, bool hovered[MODE_COUNT]) {
  update_sidebar_state(sidebar_state);
  Rectangle r = {
    sidebar_state->SIDEBAR_X, 0,
    sidebar_state->SIDEBAR_WIDTH, sidebar_state->SIDEBAR_HEIGHT
  };
  DrawRectangleRounded(r, 0.2f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2.0f, BLACK);
  const int sb_text_y = 10;
  const int sb_x = sidebar_state->SIDEBAR_X + sidebar_state->SIDEBAR_PADDING;
  const int row_step = (int)sidebar_state->ICON_HEIGHT + sidebar_state->SIDEBAR_PADDING;
  const int sb_y1 = sb_text_y + sidebar_state->FONT_SIZE;

  int y = sb_y1;
  sidebar_state->category_hovered[BUILD_CATEGORY_FARMING] = draw_icon(*sidebar_state, texture_state->wheat_tuft_idle[WHEAT_STAGE_RIPE][FIGURE_DIR_FRONT].tex, sidebar_state->si_wheat, sb_x, y);
  y += row_step;
  sidebar_state->category_hovered[BUILD_CATEGORY_MANSUS] = draw_icon(*sidebar_state, texture_state->mansus_area_icon, sidebar_state->si_mansus, sb_x, y);
  y += row_step;
  sidebar_state->category_hovered[BUILD_CATEGORY_DEV] = draw_icon(*sidebar_state, texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex, sidebar_state->si_farmer1, sb_x, y);
  y += row_step;
  hovered[BUILD_ROAD] = draw_icon(*sidebar_state, texture_state->road_flat[0], sidebar_state->si_road, sb_x, y);
  y += row_step;
  hovered[BUILD_BRIDGE] = draw_icon(*sidebar_state, texture_state->bridge[0], sidebar_state->si_bridge, sb_x, y);
  y += row_step;
  hovered[BUILD_CLEAR_FOREST] = draw_icon(*sidebar_state, texture_state->axe_cursor.tex, sidebar_state->si_clear_forest, sb_x, y);
  y += row_step;
  hovered[BUILD_DELETE] = draw_icon(*sidebar_state, texture_state->remove_icon, sidebar_state->si_delete, sb_x, y);

  if (sidebar_state->open_category >= 0) {
    draw_category_flyout(sidebar_state, texture_state, hovered, (BuildCategory)sidebar_state->open_category, sb_y1);
  }
}

// Category icons aren't real Modes (mode.c's update_md only knows about
// Mode enum values), so opening/closing a flyout is handled separately here:
// click a category icon to toggle its flyout, click one of its members to
// close it again (update_md itself already picked the mode up via `hovered`
// the same way it does for any other icon), or right-click to back out.
void update_sidebar_category(SidebarState *sidebar_state, ModeState *mode_state) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    sidebar_state->open_category = -1;
    return;
  }
  if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;

  for (int c = 0; c < BUILD_CATEGORY_COUNT; c++) {
    if (sidebar_state->category_hovered[c]) {
      sidebar_state->open_category = (sidebar_state->open_category == c) ? -1 : c;
      return;
    }
  }

  if (sidebar_state->open_category >= 0) {
    const Mode *members = CATEGORY_MODES[sidebar_state->open_category];
    int n = CATEGORY_MODE_COUNTS[sidebar_state->open_category];
    for (int i = 0; i < n; i++) {
      if (mode_state->hovered[members[i]]) {
        sidebar_state->open_category = -1;
        return;
      }
    }
  }
}
