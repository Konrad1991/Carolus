#ifndef SCALE_TYPES_H
#define SCALE_TYPES_H

// Scales
// ----------------------
typedef struct {
  float grass_flat_scale[TILE_VARIANT_COUNT];
  float grass_edge_scale[TILE_VARIANT_COUNT];
  float road_flat_scale[TILE_VARIANT_COUNT];
  float road_high_scale[TILE_VARIANT_COUNT];
  float water_flat_scale[TILE_VARIANT_COUNT];
  float water_flow_scale[FIGURE_MAX_FRAMES];
  float swamp_flat_scale[TILE_VARIANT_COUNT];
  float swamp_edge_scale[TILE_VARIANT_COUNT];
} Scales;

#endif
