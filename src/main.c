#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdio.h>
#include "mode/mode.h"
#include "map/map.h"
#include "map/map_setup.h"
#include "textures/textures.h"
#include "drawing/drawing.h"
#include "drawing/drawing_helper.h"
#include "drawing/update_tile_state.h"
#include "drawing/update_figure.h"
#include "drawing/update_grass.h"
#include "drawing/update_tree.h"
#include "drawing/update_wheat.h"
#include "drawing/build_object.h"
#include "drawing/build_road.h"
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
#include "utils/game_time.h"

// Graphic states
// -----------------------------------
static Texture_State texture_state;
static ModeState mode_state = { .current = MOVEMENT };
static TileState tile_state;
static SidebarState sidebar_state;
static FieldAssignState field_assign_state = {0};
static RoadDragState road_drag_state = {0};
static Selection selection = {0};
static WeatherScenarioState weather_scenario_state = {.current = WEATHER_SCENARIO_PERFECT_YEAR};
static CloudState cloud_state = {0};
static SeasonState season_state = {0};
static SoilOverlayState soil_overlay_state = {0};

// Game states
// -----------------------------------
static Map map;
static GameState game_state = {0};
static WeatherState weather_state = {0};
static FloodFieldArray flood_field_state = {0};
static GameSpeedState game_speed_state = {0};

int main(void) {

  SetConfigFlags(FLAG_FULLSCREEN_MODE);
  InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Carolus");
  init_tile_state(&tile_state, 64, 32, 400, 400);
  SetTargetFPS(60);

  const float flood_field_refresh_interval = 0.75f;
  float flood_field_refresh_timer = 0.0f;
  init_texture_state(&texture_state, "../Images");
  init_sidebar_state(&sidebar_state, &texture_state, 120, 10, 30, 80, 80);
  init_weather_state(&weather_state, season_state);
  init_clouds(&cloud_state);

  HideCursor();

  ObjectArray objects = {0};
  define_nature(&objects, &map, tile_state, &texture_state);
  init_soil(&map, &weather_state);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    int delete_target = hovered_delete_target(&objects, tile_state, mode_state);
    int delete_field_target = delete_target < 0 ? hovered_delete_field(&game_state, tile_state, mode_state) : -1;

    const SeasonBlend season_blend = resolve_season_blend(season_state);

    BuildPreview preview = build_object_preview(tile_state, mode_state, &texture_state, &map, &game_state, season_blend);

    for (int m = 0; m < MODE_COUNT; m++) mode_state.hovered[m] = false;

    draw_scene(&objects, tile_state, &texture_state,
               season_blend, &map,
               delete_target, &selection,
               &preview.obj, preview.show_ghost, preview.tint,
               weather_state.water_level_z, soil_overlay_state);
    draw_clouds(&cloud_state, &texture_state, &weather_state);
    draw_selection_rect(&selection);
    draw_sidebar(&sidebar_state, &texture_state, mode_state.hovered);
    draw_season_bar(&season_state);
    draw_weather_bar(&weather_state);
    draw_soil_overlay_bar(&soil_overlay_state);
    draw_soil_overlay_legend(soil_overlay_state);
    draw_weather_scenario_bar(&weather_scenario_state);
    draw_game_speed_bar(&game_speed_state);
    draw_cursor(&texture_state, &tile_state, &objects, &selection, &map);

    bool any_hovered = false;
    for (int m = BUILD_LIVING_HOUSE; m < MODE_COUNT; m++) any_hovered = any_hovered || mode_state.hovered[m];
    for (int w = 0; w < WEATHER_COUNT; w++) any_hovered = any_hovered || weather_state.hovered[w];
    any_hovered = any_hovered || weather_state.wind_lever_hovered;
    for (int o = 0; o < SOIL_OVERLAY_COUNT; o++) any_hovered = any_hovered || soil_overlay_state.hovered[o];
    for (int ws = 0; ws < WEATHER_SCENARIO_COUNT; ws++) any_hovered = any_hovered || weather_scenario_state.hovered[ws];
    for (int gs = 0; gs < GAME_SPEED_COUNT; gs++) any_hovered = any_hovered || game_speed_state.hovered[gs];

    build_object(&objects, &preview, tile_state, mode_state, &map, &game_state, &texture_state, any_hovered);
    delete_object(&objects, &map, &game_state, delete_target, any_hovered, &flood_field_state);
    delete_field(&objects, &map, &game_state, delete_field_target, any_hovered);

    update_field_action(&field_assign_state, tile_state, &mode_state, &map, &game_state, any_hovered);
    update_road_drag(&road_drag_state, tile_state, &mode_state, &map, &objects, &texture_state, any_hovered);
    update_selection(&selection, tile_state, mode_state, &objects, any_hovered);
    update_crowd_density(&map, &objects);

    flood_field_refresh_timer += game_delta_time();
    if (flood_field_refresh_timer >= flood_field_refresh_interval) {
      flood_field_refresh_timer = 0.0f;
      refresh_flood_fields(&map, &objects, &flood_field_state);
    }

    command_selected_units(&selection, &map, &tile_state, &objects, &flood_field_state, &game_state);

    update_figures(&objects, &map, texture_state, &flood_field_state);
    update_grass_tufts(&objects, &texture_state, &weather_state, season_blend);
    update_trees(&objects, &texture_state, &weather_state, season_blend);
    update_wheat_tufts(&objects, &texture_state, &weather_state);
    update_clouds(&cloud_state, &weather_state);
    update_puddles(&objects, &map, &texture_state, &weather_state);
    update_md(&mode_state);
    update_weather_scenario(&weather_state, &weather_scenario_state, &map, &game_state);
    update_weather_state(&weather_state, season_state);
    update_soil_overlay_state(&soil_overlay_state);
    update_weather_scenario_state(&weather_scenario_state);
    update_game_speed_state(&game_speed_state);
    update_soil(&map, &weather_state, &game_state);
    update_crop_growth(&map, &objects, &texture_state, &game_state);
    update_season_state(&season_state);
    update_tile_state(&tile_state);

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

  CloseWindow();
  return 0;
}
