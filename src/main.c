#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "mode/mode.h"
#include "map/map.h"
#include "map/map_setup.h"
#include "textures/textures.h"
#include "drawing/drawing.h"
#include "drawing/drawing_helper.h"
#include "drawing/update_tile_state.h"
#include "drawing/update_figure.h"
#include "drawing/update_scenery_sway.h"
#include "drawing/build_object.h"
#include "drawing/build_road.h"
#include "drawing/build_clear_forest.h"
#include "flood_fill/flood_fill.h"
#include "sidebar/sidebar.h"
#include "selection/selection.h"
#include "units/units.h"
#include "containers/arrays.h"
#include "weather/weather.h"
#include "weather/weather_scenario.h"
#include "weather/clouds.h"
#include "weather/puddles.h"
#include "seasons/seasons.h"
#include "soil/soil.h"
#include "soil/soil_overlay.h"
#include "farming/crop_growth.h"
#include "farming/field_work.h"
#include "utils/game_time.h"
#include "minimap/minimap.h"

// Graphic states
// -----------------------------------
static Texture_State texture_state;
static ModeState mode_state = { .current = MOVEMENT };
static TileState tile_state;
static SidebarState sidebar_state;
static FieldAssignState field_assign_state = {0};
static RoadDragState road_drag_state = {0};
static ClearForestDragState clear_forest_drag_state = {0};
static Selection selection = {.mansus_idx = -1, .field_mansus_idx = -1, .field_idx = -1};
static WeatherScenarioState weather_scenario_state = {.current = WEATHER_SCENARIO_PERFECT_YEAR, .last_seen_month = -1};
static CloudState cloud_state = {0};
static SeasonState season_state = {0};
static SoilOverlayState soil_overlay_state = {0};
static MinimapState minimap_state;

// Game states
// -----------------------------------
static Map map;
static GameState game_state = {0};
static WeatherState weather_state = {0};
static FloodFieldArray flood_field_state = {0};
static GameSpeedState game_speed_state = {0};

static void draw_all(const ObjectArray *objects, int delete_target, const BuildPreview *preview, SeasonBlend season_blend) {
  draw_scene(objects, tile_state, &texture_state,
             season_blend, &map,
             delete_target, &selection, &game_state,
             &preview->obj, preview->show_ghost, preview->tint,
             weather_state.water_level_z, soil_overlay_state);
  draw_mansus_assign_flash(tile_state, &map, &game_state);
  draw_mansus_selection_highlight(tile_state, &map, &game_state, &selection);
  draw_field_status_icon(tile_state, &map, &game_state, &selection);
  draw_clouds(&cloud_state, &texture_state, &weather_state);
  draw_selection_rect(&selection);

  for (int m = 0; m < MODE_COUNT; m++) mode_state.hovered[m] = false;
  draw_sidebar(&sidebar_state, &texture_state, mode_state.hovered);

  draw_season_bar(&season_state);
  draw_weather_bar(&weather_state);
  draw_soil_overlay_bar(&soil_overlay_state);
  draw_soil_overlay_legend(soil_overlay_state);
  draw_weather_scenario_bar(&weather_scenario_state);
  draw_game_speed_bar(&game_speed_state);
  draw_minimap(&minimap_state, &map, &tile_state, objects, &game_state);
  draw_mansus_goods_panel(&game_state, &selection);
  draw_cursor(&texture_state, &tile_state, objects, &selection, &map);
}

static bool compute_any_hovered(void) {
  bool any_hovered = false;
  for (int m = BUILD_LIVING_HOUSE; m < MODE_COUNT; m++) any_hovered = any_hovered || mode_state.hovered[m];
  for (int c = 0; c < BUILD_CATEGORY_COUNT; c++) any_hovered = any_hovered || sidebar_state.category_hovered[c];
  for (int w = 0; w < WEATHER_COUNT; w++) any_hovered = any_hovered || weather_state.hovered[w];
  any_hovered = any_hovered || weather_state.wind_lever_hovered;
  for (int o = 0; o < SOIL_OVERLAY_COUNT; o++) any_hovered = any_hovered || soil_overlay_state.hovered[o];
  for (int ws = 0; ws < WEATHER_SCENARIO_COUNT; ws++) any_hovered = any_hovered || weather_scenario_state.hovered[ws];
  for (int gs = 0; gs < GAME_SPEED_COUNT; gs++) any_hovered = any_hovered || game_speed_state.hovered[gs];
  any_hovered = any_hovered || minimap_state.hovered;
  Vector2 mouse = GetMousePosition();
  Rectangle sidebar_rect = {
    (float)sidebar_state.SIDEBAR_X, 0,
    (float)sidebar_state.SIDEBAR_WIDTH, (float)sidebar_state.SIDEBAR_HEIGHT
  };
  any_hovered = any_hovered || CheckCollisionPointRec(mouse, sidebar_rect);
  any_hovered = any_hovered || CheckCollisionPointRec(mouse, sidebar_flyout_rect(&sidebar_state));
  any_hovered = any_hovered || CheckCollisionPointRec(mouse, topbar_panel_rect());
  any_hovered = any_hovered || CheckCollisionPointRec(mouse, soil_overlay_panel_rect());
  return any_hovered;
}

int main(void) {

  SetConfigFlags(FLAG_FULLSCREEN_MODE);
  InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Carolus");
  init_tile_state(&tile_state, 64, 32, 400, 400);
  SetTargetFPS(60);

  const float flood_field_refresh_interval = 0.75f;
  float flood_field_refresh_timer = 0.0f;
  init_texture_state(&texture_state, "../Images");
  init_sidebar_state(&sidebar_state, &texture_state, 120, 30, 16, 75, 75);
  init_weather_state(&weather_state, season_state);
  init_clouds(&cloud_state);

  HideCursor();

  ObjectArray objects = {0};
  define_nature(&objects, &map, tile_state, &texture_state);
  init_minimap_state(&minimap_state, &map, &objects, &game_state);
  init_soil(&map, &weather_state);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    int delete_target = hovered_delete_target(&objects, tile_state, mode_state);
    int delete_field_idx = -1;
    int delete_field_target = delete_target < 0 ? hovered_delete_field(&game_state, tile_state, mode_state, &delete_field_idx) : -1;

    const SeasonBlend season_blend = resolve_season_blend(season_state);

    BuildPreview preview = build_object_preview(tile_state, mode_state, &texture_state, &map, &game_state, season_blend);

    draw_all(&objects, delete_target, &preview, season_blend);
    const bool any_hovered = compute_any_hovered();

    build_object(&objects, &preview, tile_state, mode_state, &map, &game_state, &texture_state, any_hovered);
    delete_object(&objects, &map, &game_state, delete_target, any_hovered, &flood_field_state);
    delete_field(&objects, &map, &game_state, delete_field_target, delete_field_idx, any_hovered);

    update_field_action(&field_assign_state, tile_state, &mode_state, &map, &game_state, any_hovered, &objects, &texture_state);
    update_road_drag(&road_drag_state, tile_state, &mode_state, &map, &objects, &texture_state, any_hovered);
    update_clear_forest_drag(&clear_forest_drag_state, tile_state, &mode_state, &map, &flood_field_state, &objects, &game_state, &selection, any_hovered);
    update_selection(&selection, tile_state, mode_state, &objects, &game_state, any_hovered);
    update_crowd_density(&map, &objects);

    flood_field_refresh_timer += game_delta_time();
    if (flood_field_refresh_timer >= flood_field_refresh_interval) {
      flood_field_refresh_timer = 0.0f;
      refresh_flood_fields(&map, &objects, &flood_field_state);
    }

    command_selected_units(&selection, &map, tile_state, &objects, &flood_field_state, &game_state, any_hovered);

    update_figures(&objects, &map, &texture_state, &flood_field_state, &game_state);
    update_mansus_effects(&game_state);
    update_scenery_sway(&objects, &texture_state, &weather_state, season_blend);
    update_clouds(&cloud_state, &weather_state);
    update_puddles(&objects, &map, &texture_state, &weather_state);
    update_md(&mode_state);
    update_sidebar_category(&sidebar_state, &mode_state);
    update_weather_scenario(&weather_state, &weather_scenario_state, &map, &game_state, season_state.month);
    update_weather_state(&weather_state, season_state);
    update_soil_overlay_state(&soil_overlay_state);
    update_weather_scenario_state(&weather_scenario_state);
    update_game_speed_state(&game_speed_state);
    update_soil(&map, &weather_state, &game_state);
    update_crop_growth(&map, &objects, &texture_state, &game_state);
    update_season_state(&season_state);
    update_tile_state(&tile_state);
    update_minimap_state(&minimap_state, &tile_state, &map);

    culitvate_fields(&objects, &map, &flood_field_state, &game_state, &season_state);

    EndDrawing();
  }

  object_array_free(&objects);
  selection_free(&selection);
  free_map(&map);
  free_texture_state(&texture_state);
  for (int i = 0; i < game_state.mansen.count; i++) {
    field_array_free(&game_state.mansen.data[i].fields);
  }
  mansus_array_free(&game_state.mansen);
  flood_field_array_free(&flood_field_state);
  free_minimap_state(&minimap_state);

  CloseWindow();
  return 0;
}
