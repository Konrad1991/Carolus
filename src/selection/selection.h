#ifndef SELECTION_H
#define SELECTION_H

#include "types.h"

void selection_clear(Selection *sel);
void selection_add(Selection *sel, unsigned int id);
bool selection_contains(const Selection *sel, unsigned int id);
void selection_free(Selection *sel);

void update_selection(Selection *sel, TileState tile_state, ModeState mode_state,
                      const ObjectArray *objects, bool icon_hovered);
void draw_selection_rect(const Selection *sel);

#endif
