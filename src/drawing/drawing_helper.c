#include "drawing/drawing_helper.h"
#include <math.h>

int tile_variant(int tx, int ty, int num_variants) {
  unsigned int h = (unsigned int)(tx * 73856093) ^ (unsigned int)(ty * 19349663);
  h ^= h >> 13;
  h *= 0x5bd1e995;
  h ^= h >> 15;
  return h % num_variants;
}

Vector2 screen_direction_for_facing(FigureDirection dir) {
  static const Vector2 dirs[FIGURE_DIR_COUNT] = {
    {-1.0f,  1.0f}, // FIGURE_DIR_FRONT       screen down-left
    { 0.0f,  1.0f}, // FIGURE_DIR_FRONT_RIGHT screen down
    { 1.0f,  1.0f}, // FIGURE_DIR_RIGHT       screen down-right
    { 1.0f,  0.0f}, // FIGURE_DIR_BACK_RIGHT  screen right
    { 1.0f, -1.0f}, // FIGURE_DIR_BACK        screen up-right
    { 0.0f, -1.0f}, // FIGURE_DIR_BACK_LEFT   screen up
    {-1.0f, -1.0f}, // FIGURE_DIR_LEFT        screen up-left
    {-1.0f,  0.0f}, // FIGURE_DIR_FRONT_LEFT  screen left
  };
  Vector2 raw = dirs[dir];
  float len = sqrtf(raw.x * raw.x + raw.y * raw.y);
  return (Vector2){raw.x / len, raw.y / len};
}

Vector2 iso_to_screen(TileState tile_state, const float x, const float y) {
  return (Vector2){
    (x - y) * tile_state.TILE_W / 2 + tile_state.OFFSET_X,
    (x + y) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y + tile_state.TILE_H,
  };
}

int screen_to_tile_x(TileState tile_state, const float sx, const float sy) {
  float dx = (sx - tile_state.OFFSET_X) / (tile_state.TILE_W / 2.0f);
  float dy = (sy - tile_state.OFFSET_Y) / (tile_state.TILE_H / 2.0f);
  return (int)((dx + dy) / 2);
}

int screen_to_tile_y(TileState tile_state, const float sx, const float sy) {
  float dx = (sx - tile_state.OFFSET_X) / (tile_state.TILE_W / 2.0f);
  float dy = (sy - tile_state.OFFSET_Y) / (tile_state.TILE_H / 2.0f);
  return (int)((dy - dx) / 2);
}

void draw_diamond(TileState tile_state, int sx, int sy, Color col) {
  Vector2 top = {sx, sy};
  Vector2 right = {sx + tile_state.TILE_W/2, sy + tile_state.TILE_H/2};
  Vector2 bottom = {sx, sy + tile_state.TILE_H};
  Vector2 left = {sx - tile_state.TILE_W/2, sy + tile_state.TILE_H/2};
  DrawTriangle(top, left, bottom, col);
  DrawTriangle(top, bottom, right, col);
}

void draw_diamond_outline(TileState tile_state, int sx, int sy, Color col) {
  Vector2 top = {sx, sy};
  Vector2 right = {sx + tile_state.TILE_W/2, sy + tile_state.TILE_H/2};
  Vector2 bottom = {sx, sy + tile_state.TILE_H};
  Vector2 left = {sx - tile_state.TILE_W/2, sy + tile_state.TILE_H/2};
  DrawLineV(top, right, col);
  DrawLineV(right, bottom, col);
  DrawLineV(bottom, left, col);
  DrawLineV(left, top, col);
}

void draw_ground_tile_tinted(TileState tile_state, int sx, int sy, const Texture2D *tex, float scale, Color tint) {
  Vector2 pos = {
    sx - (tex->width  * scale) / 2.0f,
    sy + tile_state.TILE_H - tex->height * scale,
  };
  DrawTextureEx(*tex, pos, 0, scale, tint);
}

void draw_ground_tile(TileState tile_state, int sx, int sy, const Texture2D *tex, float scale) {
  draw_ground_tile_tinted(tile_state, sx, sy, tex, scale, WHITE);
}

void draw_relief_tile_tinted(int sx, int sy, const Texture2D *tex, float scale, Color tint) {
  Vector2 pos = {
    sx - (tex->width * scale) / 2.0f,
    sy,
  };
  DrawTextureEx(*tex, pos, 0, scale, tint);
}

void draw_relief_tile(int sx, int sy, const Texture2D *tex, float scale) {
  draw_relief_tile_tinted(sx, sy, tex, scale, WHITE);
}

Rectangle calc_object_screen_rectangle(TileState tile_state, const Object* o) {
  float scale = o->sprite.scale * tile_state.TILE_W / 64.0f;
  bool use_draw_pos = (o->kind == OBJECT_FIGURE && o->id != 0) || o->kind == OBJECT_WHEAT_TUFT;
  Vector2 anchor = use_draw_pos
    ? iso_to_screen(tile_state, o->draw.x, o->draw.y)
    : iso_to_screen(tile_state, o->tx, o->ty);
  anchor.y -= o->z * tile_state.TILE_H;
  Vector2 pos = {
    anchor.x - o->sprite.tex.width  * scale * o->sprite.anchor.x,
    anchor.y - o->sprite.tex.height * scale * o->sprite.anchor.y,
  };
  Rectangle dest = {
    pos.x, pos.y,
    o->sprite.tex.width * scale, o->sprite.tex.height * scale,
  };
  return dest;
}

void draw_object_slice(TileState tile_state, const Object *o, Color tint, float slice_from, float slice_to) {
  Rectangle dest_full = calc_object_screen_rectangle(tile_state, o);
  Rectangle source = {
    0, o->sprite.tex.height * slice_from,
    (float)o->sprite.tex.width, o->sprite.tex.height * (slice_to - slice_from),
  };
  Rectangle dest = {
    dest_full.x, dest_full.y + dest_full.height * slice_from,
    dest_full.width, dest_full.height * (slice_to - slice_from),
  };
  DrawTexturePro(o->sprite.tex, source, dest, (Vector2){0, 0}, 0, tint);
}

void draw_object_slice_x(TileState tile_state, const Object *o, Color tint, float slice_from, float slice_to) {
  Rectangle dest_full = calc_object_screen_rectangle(tile_state, o);
  Rectangle source = {
    o->sprite.tex.width * slice_from, 0,
    o->sprite.tex.width * (slice_to - slice_from), (float)o->sprite.tex.height,
  };
  Rectangle dest = {
    dest_full.x + dest_full.width * slice_from, dest_full.y,
    dest_full.width * (slice_to - slice_from), dest_full.height,
  };
  DrawTexturePro(o->sprite.tex, source, dest, (Vector2){0, 0}, 0, tint);
}

void draw_object(TileState tile_state, const Object *o, Color tint) {
  draw_object_slice(tile_state, o, tint, 0.0f, 1.0f);
}

int find_object_at_tile(const ObjectArray *objects, int tx, int ty) {
  int best = -1;
  int best_area = 0;
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (tx <= o->tx && tx > o->tx - o->footprint_w &&
      ty <= o->ty && ty > o->ty - o->footprint_h) {
      int area = o->footprint_w * o->footprint_h;
      if (best < 0 || area < best_area) {
        best = i;
        best_area = area;
      }
    }
  }
  return best;
}

int find_tree_at_screen_pos(const ObjectArray *objects, TileState tile_state, Vector2 mouse) {
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (o->kind != OBJECT_TREE) continue;
    if (CheckCollisionPointRec(mouse, calc_object_screen_rectangle(tile_state, o))) return i;
  }
  return -1;
}
