#ifndef MODE_TYPES_H
#define MODE_TYPES_H

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
  BUILD_CLEAR_FOREST,
  BUILD_MANSUS,
  BUILD_DELETE,
  BUILD_HIVE,
  BUILD_WELL,
  MODE_COUNT
} Mode;

typedef struct {
  Mode current;
  bool hovered[MODE_COUNT];
  BuildingDirection build_direction;
} ModeState;

#endif
