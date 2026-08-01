#ifndef SOIL_OVERLAY_TYPES_H
#define SOIL_OVERLAY_TYPES_H

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

#endif // !SOIL_OVERLAY_TYPES_H
