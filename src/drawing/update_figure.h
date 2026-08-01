#ifndef UPDATE_FIGURE_H
#define UPDATE_FIGURE_H

#include "types.h"
#include "map/map.h"

FigureDirection detect_figure_direction(const int x0, const int x1, const int y0, const int y1);

void update_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, FloodFieldArray* flood_field_state,
                    GameState *game_state);

void figure_release_reservation(Map *map, Object *figure);

void figure_walk_to(Object *figure, Map *map, FloodFieldArray *flood_field_state, int target_tx, int target_ty);

void harvest_route_field_tile(const HarvestRoute *route, int *out_tx, int *out_ty);
void harvest_route_stand_tile(const HarvestRoute *route, int *out_tx, int *out_ty);

#endif
