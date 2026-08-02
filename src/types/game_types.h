#ifndef GAME_TYPES_H
#define GAME_TYPES_H

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
  SUMMER_WHEAT,
  WINTER_WHEAT
} Crop;

#define WHEAT_TUFTS_PER_TILE 7

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
  unsigned int worked_by_figure_id;
  int cultivated_last_in_year;
} Field;

typedef struct {
  Field *data;
  int count;
  int capacity;
} FieldArray;

typedef struct {
  int children;
  int adult_children;
  int elders;
  int farmhands;
  int maids;
} Familia;

typedef struct {
  int grains;
  int straw;
  int timber;
  int clay;
  int limestone;
  int slaked_lime;
  int willow_branches;
} Goods;

#define MANSUS_ASSIGN_FLASH_SECONDS 1.0f

typedef struct {
  MansusLevel mansus_level;
  int corners_floor_area[2][2];
  unsigned int living_house;
  unsigned int barn;
  FieldArray fields;
  unsigned int farmer_object_id;
  unsigned int farmhand_object_id;
  Familia familia;
  Goods goods;
  float assign_flash_timer;
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

// Grain cost to sow one field (fixed 8x8 = 64 tiles), and the starting seed
// grain a new Mansus is given so its first field isn't unaffordable.
#define FIELD_SOW_GRAIN_COST 112

// A Mansus can hold two fields so it can rotate sowing between them year by year.
#define MANSUS_MAX_FIELDS 2

typedef struct {
  bool assigning;
  int corners_field[2][2];
  Crop crop;
} FieldAssignState;

#endif
