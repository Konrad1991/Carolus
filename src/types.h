#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

// Game state
// ----------------------
typedef enum {
  HOUSLER,
  QUARTER_HUFNER,
  HALF_HUFNER,
  FULLHUFNER,
  RICH_FARMER
} MansusLevel;

typedef enum {
  WHEAT,
  RYE,
  OAT,
  BARLEY
} Crop;

typedef enum {
  GRASS,
  PLOWED,
  SOWED,
  SMALL_GREEN_PLANTS,
  MEDIUM_GREEN_PLANTS,
  LARGE_GREEN_PLANTS,
  LARGE_YELLOW_PLANTS,
  FALLOW,
  DAMAGED_LARGE_GREEN_PLANTS,
  DAMAGED_LARGE_YELLOW_PLANTS
} FieldCondition;

typedef enum {
  WHEAT_STAGE_YOUNG,
  WHEAT_STAGE_MIDDLE,
  WHEAT_STAGE_LARGE_GREEN,
  WHEAT_STAGE_RIPE,
  WHEAT_STAGE_OVERRIPE,
  WHEAT_STAGE_HARVESTED,
  WHEAT_STAGE_DESTROYED,
  WHEAT_STAGE_COUNT
} WheatStage;

typedef struct {
  Crop crop;
  FieldCondition field_condition;
  int corners_field[2][2];
  float stage_progress_timer;
} Field;

typedef struct {
  Field *data;
  int count;
  int capacity;
} FieldArray;

typedef struct {
  MansusLevel mansus_level;
  int corners_floor_area[2][2];
  unsigned int living_house;
  unsigned int barn;
  FieldArray fields;
} Mansus;

typedef struct {
  Mansus *data;
  int count;
  int capacity;
} MansusArray;

typedef struct {
  MansusArray mansen;
} GameState;

#define MANSUS_FOOTPRINT_W 10
#define MANSUS_FOOTPRINT_H 5
#define QUARTER_HUFE_FOOTPRINT_W 8
#define QUARTER_HUFE_FOOTPRINT_H 8

typedef struct {
  bool assigning;
  int corners_field[2][2];
  Crop crop;
} FieldAssignState;

// Map types
// ----------------------
typedef enum {
  TILE_GRASS,
  TILE_ROAD,
  TILE_WATER,
  TILE_MANSUSYARD,
  TILE_SOIL
} TileType;

typedef struct {
  TileType type;
  float z;
  int occupied;
  bool figure_occupied;
  float crowd_density;
  float soil_water;
  float soil_minerals;
  float soil_temperature;
} Tile;

typedef struct {
  Tile *tiles;
  int w;
  int h;
} Map;

// Path finding result
// ----------------------
typedef struct {
  int node;
  int priority;
} HeapEntry;

typedef struct {
  HeapEntry *arr;
  int size;
  int *pos; /* pos[node] = current index in arr, or -1 if not in the heap */
} BinaryHeap;

typedef struct {
  int *solution;
  int solution_len;
  int cost; /* -1 if no path was found */
} PathResult;

typedef struct {
  bool dragging;
  int start_tx;
  int start_ty;
  PathResult path;
} RoadDragState;

typedef struct {
  int* cost;
  int* cost_no_density;
  int len;
  int n_figures;
  bool active;
} FloodField;

typedef struct {
  FloodField *data;
  int count;
  int capacity;
} FloodFieldArray;

// TileState
// ----------------------
typedef struct {
  int TILE_W;
  int TILE_H;
  int N_WIDTH_TILES;
  int N_HEIGHT_TILES;
  int OFFSET_X;
  int OFFSET_Y;
} TileState;

// Seasons
// ----------------------
#define SECONDS_PER_MONTH 300
typedef enum {
  WINTER,
  SPRING,
  SUMMER,
  AUTUMN,
  SEASON_COUNT
} Season;

typedef struct {
  int month;
  float month_progress;
  bool month_up_hovered;
  bool month_down_hovered;
} SeasonState;

typedef struct {
  Season base;
  Season fade_in;
  float alpha;
} SeasonBlend;

// SpriteAsset - a texture plus the anchor/scale needed to place it
// ----------------------
typedef struct {
  Texture2D tex;
  Vector2 anchor;
  float scale;
} SpriteAsset;

typedef enum {
  FIGURE_DIR_FRONT,       // ty+,        screen down-left
  FIGURE_DIR_FRONT_RIGHT, // tx+, ty+,   screen down
  FIGURE_DIR_RIGHT,       // tx+,        screen down-right
  FIGURE_DIR_BACK_RIGHT,  // tx+, ty-,   screen right
  FIGURE_DIR_BACK,        // ty-,        screen up-right
  FIGURE_DIR_BACK_LEFT,   // tx-, ty-,   screen up
  FIGURE_DIR_LEFT,        // tx-,        screen up-left
  FIGURE_DIR_FRONT_LEFT,  // tx-, ty+,   screen left
  FIGURE_DIR_COUNT
} FigureDirection;

typedef enum {
  BUILDING_DIR_SE,
  BUILDING_DIR_SW,
  BUILDING_DIR_NW,
  BUILDING_DIR_NE,
  BUILDING_DIR_COUNT
} BuildingDirection;

typedef enum {
  FIGURE_ACTION_STAND,
  FIGURE_ACTION_WALK,
  FIGURE_ACTION_SOW,
  FIGURE_ACTION_CHOP,
  FIGURE_ACTION_DIG,
  FIGURE_ACTION_HAMMER,
  FIGURE_ACTION_MOW,
  FIGURE_ACTION_COUNT
} FigureAction;

#define FIGURE_MAX_FRAMES 9
#define FARMER_TYPE_COUNT 2
#define FIGURE_SPEED_TILES_PER_SECOND 2.0f

#define OX_WALK_FRAMES 8
#define PLOW_WALK_FRAMES 9

typedef enum {
  FIGURE_SPECIES_FARMER1,
  FIGURE_SPECIES_FARMER2,
  FIGURE_SPECIES_OX,
  FIGURE_SPECIES_COUNT
} FigureSpecies;

typedef enum {
  PLOW_PHASE_NONE,
  PLOW_PHASE_APPROACH,
  PLOW_PHASE_SWEEP,
  PLOW_PHASE_STEP
} PlowPhase;

typedef struct {
  PlowPhase phase;
  bool row_along_tx;
  int min_tx, max_tx, min_ty, max_ty;
  int step_coord;
  int step_dir;
  bool sweep_positive;
} PlowRoute;

typedef enum {
  HARVEST_PHASE_NONE,
  HARVEST_PHASE_WALKING,
  HARVEST_PHASE_MOWING
} HarvestPhase;

typedef struct {
  HarvestPhase phase;
  bool row_along_tx;
  int min_tx, max_tx, min_ty, max_ty;
  int row;
  int step_dir;
  int cursor;
  int sweep_dir;
  float mow_timer;
} HarvestRoute;

// TextureState
// ----------------------
#define TILE_VARIANT_COUNT 16

typedef struct {
  Texture2D grass_flat[SEASON_COUNT][TILE_VARIANT_COUNT];
  Texture2D grass_edge[SEASON_COUNT][TILE_VARIANT_COUNT];
  Texture2D road_flat[TILE_VARIANT_COUNT];
  Texture2D road_high[TILE_VARIANT_COUNT];
  Texture2D water_flat[TILE_VARIANT_COUNT];
  Texture2D water_flow[FIGURE_MAX_FRAMES];
  Texture2D earth[TILE_VARIANT_COUNT];
  Texture2D arable_land[TILE_VARIANT_COUNT];
  SpriteAsset house1[BUILDING_DIR_COUNT];
  SpriteAsset house2[BUILDING_DIR_COUNT];
  Texture2D bridge[TILE_VARIANT_COUNT];
  SpriteAsset farmers[FARMER_TYPE_COUNT][FIGURE_ACTION_COUNT][FIGURE_DIR_COUNT][FIGURE_MAX_FRAMES];
  SpriteAsset ox_stand[FIGURE_DIR_COUNT];
  SpriteAsset ox_walk[FIGURE_DIR_COUNT][OX_WALK_FRAMES];
  SpriteAsset plow_stand[FIGURE_DIR_COUNT];
  SpriteAsset plow_walk[FIGURE_DIR_COUNT][PLOW_WALK_FRAMES];
  SpriteAsset oak[SEASON_COUNT][BUILDING_DIR_COUNT];
  SpriteAsset oak_sway[SEASON_COUNT][FIGURE_DIR_COUNT][FIGURE_MAX_FRAMES];

  SpriteAsset grass_tuft_idle[SEASON_COUNT][FIGURE_DIR_COUNT];
  SpriteAsset grass_tuft_sway[SEASON_COUNT][FIGURE_DIR_COUNT][FIGURE_MAX_FRAMES];

  SpriteAsset wheat_tuft_idle[WHEAT_STAGE_COUNT][FIGURE_DIR_COUNT];
  SpriteAsset wheat_tuft_sway[WHEAT_STAGE_COUNT][FIGURE_DIR_COUNT][FIGURE_MAX_FRAMES];
  SpriteAsset cloud_drift[FIGURE_DIR_COUNT][FIGURE_MAX_FRAMES];
  SpriteAsset puddle[FIGURE_DIR_COUNT];
  SpriteAsset boundary_stone;
  SpriteAsset wheat_sheaf;
  SpriteAsset cursor;
  SpriteAsset axe_cursor;
  SpriteAsset plow_cursor;
  SpriteAsset scythe_cursor;
  Texture2D mansus_area_icon;
  Texture2D remove_icon;
} Texture_State;


// Weather
// ----------------------
typedef enum {
  WEATHER_SUNNY,
  WEATHER_WINDY,
  WEATHER_RAIN,
  WEATHER_COUNT
} Weather;

typedef struct {
  Weather current;
  bool hovered[WEATHER_COUNT];
  FigureDirection wind_direction;
  bool wind_lever_hovered;
  float temperature_celsius;
  float water_level_z;
} WeatherState;

typedef enum {
  WEATHER_SCENARIO_NONE,
  WEATHER_SCENARIO_PERFECT_YEAR,
  WEATHER_SCENARIO_GOOD_YEAR,
  WEATHER_SCENARIO_MEDIOCRE_YEAR,
  WEATHER_SCENARIO_BAD_YEAR,
  WEATHER_SCENARIO_COUNT
} WeatherScenario;

typedef struct {
  WeatherScenario current;
  float elapsed;
  float sub_timer;
  bool hovered[WEATHER_SCENARIO_COUNT];
} WeatherScenarioState;

// Game speed
// ----------------------
#define GAME_SPEED_COUNT 5
typedef struct {
  int current_index;
  bool hovered[GAME_SPEED_COUNT];
} GameSpeedState;

// Soil overlay --> Pharaoh/Caesar-style "special maps"
// ----------------------
typedef enum {
  SOIL_OVERLAY_NONE,
  SOIL_OVERLAY_WATER,
  SOIL_OVERLAY_MINERALS,
  SOIL_OVERLAY_TEMPERATURE,
  SOIL_OVERLAY_COUNT
} SoilOverlay;

typedef struct {
  SoilOverlay current;
  bool hovered[SOIL_OVERLAY_COUNT];
} SoilOverlayState;

// Clouds
// ----------------------
#define CLOUD_COUNT 6

typedef struct {
  float x;
  float y;
  float speed;
  float phase;
} Cloud;

typedef struct {
  Cloud clouds[CLOUD_COUNT];
} CloudState;

// Minimap
// ----------------------
typedef struct {
  Texture2D terrain_texture;
  bool terrain_texture_loaded;
  float refresh_timer;
  bool hovered;
} MinimapState;

// Mode
// ----------------------
typedef enum {
  MOVEMENT,
  BUILD_LIVING_HOUSE,
  BUILD_BARN,
  BUILD_FIELD,
  BUILD_ROAD,
  BUILD_BRIDGE,
  BUILD_FARMER1,
  BUILD_FARMER2,
  BUILD_OX,
  BUILD_OAK,
  BUILD_GRASS,
  BUILD_MANSUS,
  BUILD_DELETE,
  MODE_COUNT
} Mode;

typedef struct {
  Mode current;
  bool hovered[MODE_COUNT];
  BuildingDirection build_direction;
} ModeState;

// Objects
// ----------------------
typedef enum {
  OBJECT_BUILDING,
  OBJECT_FIGURE,
  OBJECT_BRIDGE,
  OBJECT_MANSUS,
  OBJECT_ROAD,
  OBJECT_TREE,
  OBJECT_GRASS_TUFT,
  OBJECT_WHEAT_TUFT,
  OBJECT_PUDDLE,
  OBJECT_BOUNDARY_STONE
} ObjectKind;

typedef struct {
  int flood_field_idx; // -1: idle
  int target_tx;
  int target_ty;
  float progress;
  float speed;
  FigureAction action;
  float action_timer;
  FigureAction pending_action;
  int gather_tx;
  int gather_ty;
  FigureSpecies species;
  bool plowing;
  PlowRoute plow_route;
  HarvestRoute harvest_route;
  int step_tx;
  int step_ty;
  int prev_tile;
  int best_distance_to_target;
  int pacing_streak;
} FigureAttributes;

typedef struct {
  WheatStage wheat_stage;
  float wheat_progress_timer;
  float wheat_vigor;
  float wheat_stage_age;
  float water_absorbed;
  float minerals_absorbed;
} WheatAttributes;

typedef struct {
  float lifetime_total;
  float lifetime_remaining;
  float base_scale;
} PuddleAttributes;

typedef struct {
  unsigned int id;
  SpriteAsset sprite;
  int tx;
  int ty;
  int z;
  int footprint_w;
  int footprint_h;
  ObjectKind kind;
  FigureDirection facing;
  float draw_x;
  float draw_y;
  union {
    FigureAttributes figure;
    WheatAttributes wheat;
    PuddleAttributes puddle;
  };
} Object;

typedef struct {
  Object *data;
  int count;
  int capacity;
} ObjectArray;

typedef struct {
  int idx;
  float frontmost_point;
  float backmost_point;
  int z;
  int tile_x;
  int tile_y;
  float sprite_slice_from;
  float sprite_slice_to;
  bool is_figure;
  bool is_ground_decor;
  bool is_tall_decor;
  bool is_puddle;
  bool is_preview;
} ObjectOrder;

typedef struct {
  Object obj;
  bool active;
  bool show_ghost;
  bool placeable;
  Color tint;
} BuildPreview;

// Selection
// ----------------------
typedef struct {
  unsigned int *ids; // Object.id of the currently selected figures
  int count;
  int capacity;
  bool dragging;
  Vector2 drag_start;
} Selection;

// SidebarState
// ----------------------
typedef struct {
  int SIDEBAR_WIDTH;
  int SIDEBAR_HEIGHT;
  int SIDEBAR_X;
  int SIDEBAR_PADDING;
  int FONT_SIZE;
  float ICON_WIDTH;
  float ICON_HEIGHT;
  float si_h1;
  float si_h2;
  float si_road;
  float si_bridge;
  float si_farmer1;
  float si_farmer2;
  float si_ox;
  float si_oak;
  float si_wheat;
  float si_grass;
  float si_mansus;
  float si_delete;
} SidebarState;

// Scales
// ----------------------
typedef struct {
  float grass_flat_scale[TILE_VARIANT_COUNT];
  float grass_edge_scale[TILE_VARIANT_COUNT];
  float road_flat_scale[TILE_VARIANT_COUNT];
  float road_high_scale[TILE_VARIANT_COUNT];
  float water_flat_scale[TILE_VARIANT_COUNT];
  float water_flow_scale[FIGURE_MAX_FRAMES];
} Scales;

#endif
