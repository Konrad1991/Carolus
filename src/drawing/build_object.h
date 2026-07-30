#ifndef BUILD_OBJECT_H
#define BUILD_OBJECT_H

#include "types.h"
#include "map/map.h"

void place_road_tile(ObjectArray *objects, Map *map, const Texture_State *texture_state, int tx, int ty);
int hovered_delete_target(const ObjectArray *objects, TileState tile_state, ModeState mode_state);
void delete_object(ObjectArray *objects, Map *map, GameState *game_state, int index, bool icon_hovered, FloodFieldArray* flood_field_state);
int hovered_delete_field(const GameState *game_state, TileState tile_state, ModeState mode_state);
void delete_field(ObjectArray *objects, Map *map, GameState *game_state, int mansus_idx, bool icon_hovered);

BuildPreview build_object_preview(TileState tile_state, ModeState mode_state, const Texture_State *texture_state,
                                  Map *map, GameState *game_state, SeasonBlend season_blend);

void build_object(ObjectArray *objects, const BuildPreview *preview,
                  TileState tile_state, ModeState mode_state,
                  Map* map, GameState *game_state, const Texture_State *texture_state,
                  bool icon_hovered);

void update_field_action(FieldAssignState *field_state, TileState tile_state, ModeState *mode_state,
                         Map *map, GameState *game_state, bool icon_hovered);

#endif
