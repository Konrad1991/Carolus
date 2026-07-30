#ifndef SOIL_OVERLAY_H
#define SOIL_OVERLAY_H

#include "types.h"

void draw_soil_overlay_bar(SoilOverlayState *state);

void draw_soil_overlay_legend(SoilOverlayState state);

void update_soil_overlay_state(SoilOverlayState *state);

Color soil_overlay_tint(SoilOverlayState state, const Tile *tile);

#endif
