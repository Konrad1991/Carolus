#ifndef MINIMAP_H
#define MINIMAP_H

#include "types.h"

void init_minimap_state(MinimapState *state, const Map *map, const ObjectArray *objects, const GameState *game_state);
void free_minimap_state(MinimapState *state);
void draw_minimap(MinimapState *state, const Map *map, const TileState *tile_state,
                  const ObjectArray *objects, const GameState *game_state);
void update_minimap_state(MinimapState *state, TileState *tile_state, const Map *map);

#endif
