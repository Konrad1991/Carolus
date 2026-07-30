#ifndef DRAWING_HELPER_H
#define DRAWING_HELPER_H

#include "raylib.h"
#include "types.h"

int tile_variant(int tx, int ty, int num_variants);
Vector2 screen_direction_for_facing(FigureDirection dir);

Vector2 iso_to_screen(TileState tile_state, const float x, const float y);
int screen_to_tile_x(TileState tile_state, const float sx, const float sy);
int screen_to_tile_y(TileState tile_state, const float sx, const float sy);

void draw_diamond(TileState tile_state, int sx, int sy, Color col);
void draw_diamond_outline(TileState tile_state, int sx, int sy, Color col);
void draw_ground_tile(TileState tile_state, int sx, int sy, Texture2D *tex, float scale);
void draw_relief_tile(int sx, int sy, Texture2D *tex, float scale);
Rectangle calc_object_screen_rectangle(TileState tile_state, const Object* o);
void draw_object_slice(TileState tile_state, const Object *o, Color tint, float slice_from, float slice_to);
void draw_object(TileState tile_state, const Object *o, Color tint);
int find_object_at_tile(const ObjectArray *objects, int tx, int ty);

void draw_relief_tile_tinted(int sx, int sy, Texture2D *tex, float scale, Color tint);
void draw_ground_tile_tinted(TileState tile_state, int sx, int sy, Texture2D *tex, float scale, Color tint);

#endif
