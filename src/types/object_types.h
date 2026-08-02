#ifndef OBJECT_TYPES_H
#define OBJECT_TYPES_H

// Objects
// ----------------------
typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  float x;
  float y;
} PositionFloat;

typedef struct {
  int min_tx;
  int max_tx;
  int min_ty;
  int max_ty;
  bool row_along_tx;
} RouteBounds;

typedef struct {
  int row;
  int step_dir;
  int cursor;
  int sweep_dir;
} RowSweepState;

typedef enum {
  PLOW_PHASE_NONE,
  PLOW_PHASE_APPROACH,
  PLOW_PHASE_SWEEP,
  PLOW_PHASE_STEP
} PlowPhase;

typedef struct {
  PlowPhase phase;
  RouteBounds route_bounds;
  int step_coord;
  int step_dir;
  bool sweep_positive;
} PlowRoute;

typedef enum {
  HARVEST_PHASE_NONE,
  HARVEST_PHASE_WALKING,
  HARVEST_PHASE_MOWING,
  HARVEST_PHASE_TO_SHEAF,
  HARVEST_PHASE_PICKING,
  HARVEST_PHASE_TO_BARN
} HarvestPhase;

typedef struct {
  HarvestPhase phase;
  RouteBounds route_bounds;
  RowSweepState row_sweep_state;
  float mow_timer;
  float pick_timer;
  int carried_sheaves;
  bool wearing_basket;
  bool mowing_done;
} HarvestRoute;

typedef enum {
  SOW_PHASE_NONE,
  SOW_PHASE_WALKING
} SowPhase;

typedef struct {
  SowPhase phase;
  RouteBounds route_bounds;
  RowSweepState row_sweep_state;
} SowRoute;

typedef enum {
  DIG_PHASE_NONE,
  DIG_PHASE_WALKING,
  DIG_PHASE_DIGGING
} DigPhase;

typedef struct {
  DigPhase phase;
  RouteBounds route_bounds;
  RowSweepState row_sweep_state;
  float dig_timer;
} DigRoute;

typedef enum {
  WOOD_PHASE_NONE,
  WOOD_PHASE_PICKING,
  WOOD_PHASE_TO_BARN
} WoodPhase;

typedef struct {
  WoodPhase phase;
  float pick_timer;
  unsigned int tree_id;
} WoodRoute;

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
  OBJECT_BOUNDARY_STONE,
  OBJECT_ROCK
} ObjectKind;

typedef struct {
  int flood_field_idx; // -1: idle
  bool direct_walking; // short local hop within a field/route, no flood field needed
  Position target;
  float progress;
  float speed;
  FigureAction action;
  float action_timer;
  FigureAction pending_action;
  Position gather;
  FigureSpecies species;
  bool plowing;
  PlowRoute plow_route;
  HarvestRoute harvest_route;
  SowRoute sow_route;
  DigRoute dig_route;
  WoodRoute wood_route;
  Position step;
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
  bool is_field_boundary; // true: marks a field's corner, drawn tinted red; false: a Mansus yard corner
} BoundaryStoneAttributes;

typedef enum {
  TREE_STATE_STANDING,
  TREE_STATE_FELLED,
} TreeState;

typedef struct {
  TreeState state;
  float chop_seconds;
} TreeAttributes;

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
  PositionFloat draw;
  union {
    FigureAttributes figure;
    WheatAttributes wheat;
    PuddleAttributes puddle;
    BoundaryStoneAttributes boundary_stone;
    TreeAttributes tree;
  };
} Object;

typedef struct {
  Object *data;
  int count;
  int capacity;
  int *id_to_index; // id_to_index[id] = current index in data, or -1 if not present
  int id_to_index_capacity;
} ObjectArray;

typedef struct {
  int idx;
  float frontmost_point;
  float backmost_point;
  int z;
  Position tile;
  float sprite_slice_from;
  float sprite_slice_to;
  bool slice_horizontal; // true: slice_from/to cut the sprite left-to-right instead of top-to-bottom
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

#endif // !OBJECT_TYPES_H
