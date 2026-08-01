#ifndef MINIMAP_TYPES_H
#define MINIMAP_TYPES_H

// Minimap
// ----------------------
typedef struct {
  Texture2D terrain_texture;
  bool terrain_texture_loaded;
  float refresh_timer;
  bool hovered;
} MinimapState;

#endif
