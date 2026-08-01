#ifndef UNITS_H
#define UNITS_H

#include "types.h"
#include "map/map.h"
#include "selection/selection.h"

void start_plow_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                             GameState *game_state, Field *field);
void start_harvest_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                                GameState *game_state, Field *field);
void start_dig_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                            GameState *game_state, Field *field);
void start_sow_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                            GameState *game_state, Mansus *mansus, Field *field);
void command_selected_units(const Selection *sel, Map *map, TileState tile_state, ObjectArray *objects, FloodFieldArray* flood_field_state, GameState *game_state, bool any_hovered);
Field *find_field_at(GameState *game_state, int tx, int ty, int *out_mansus_idx);
void release_field_lock(GameState *game_state, unsigned int figure_id);

#endif
