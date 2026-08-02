#ifndef MAP_TYPES_H
#define MAP_TYPES_H

// Map types
// ----------------------
typedef enum {
  TILE_GRASS,
  TILE_ROAD,
  TILE_WATER,
  TILE_MANSUSYARD,
  TILE_SOIL,
  TILE_SWAMP
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
  unsigned int wheat_tuft_ids[WHEAT_TUFTS_PER_TILE];
  int wheat_tuft_count;
  bool has_puddle;
  float fallow_grass_blend; // 0: bare soil, 1: fully overgrown; how much a fallow field tile has grassed over
} Tile;

typedef struct {
  Tile *tiles;
  int w;
  int h;
} Map;

#endif
