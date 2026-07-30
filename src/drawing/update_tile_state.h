#ifndef UPDATE_TILE_STATE_H
#define UPDATE_TILE_STATE_H

#include "types.h"

void init_tile_state(TileState* tile_state, const int TILE_W, const int TILE_H,
                     const int N_WIDTH_TILES, const int N_HEIGHT_TILES);

void update_tile_state(TileState* tile_state);

#endif
