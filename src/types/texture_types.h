#ifndef TEXTURE_TYPES_H
#define TEXTURE_TYPES_H

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
  FIGURE_ACTION_CARRY_STAND,
  FIGURE_ACTION_CARRY_PICK,
  FIGURE_ACTION_CARRY_WALK,
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
  SpriteAsset oak_trunk[FIGURE_DIR_COUNT];
  SpriteAsset oak_beam[FIGURE_DIR_COUNT];

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
  SpriteAsset dig_cursor;
  SpriteAsset sow_cursor;
  Texture2D mansus_area_icon;
  Texture2D remove_icon;
} Texture_State;

#endif
