#ifndef UPDATE_FIGURE_H
#define UPDATE_FIGURE_H

#include "types.h"
#include "map/map.h"

FigureDirection detect_figure_direction(Position from, Position to);

void update_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, FloodFieldArray* flood_field_state,
                    GameState *game_state);

void figure_release_reservation(Map *map, Object *figure);

void figure_walk_to(Object *figure, Map *map, FloodFieldArray *flood_field_state, Position position);

void harvest_route_field_tile(const HarvestRoute *route, Position* position);
void harvest_route_stand_tile(const HarvestRoute *route, Position* position);

#endif
