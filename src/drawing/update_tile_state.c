#include "drawing/update_tile_state.h"
#include <math.h>

void init_tile_state(TileState* tile_state, const int TILE_W, const int TILE_H,
                     const int N_WIDTH_TILES, const int N_HEIGHT_TILES) {
  tile_state->TILE_W = TILE_W;
  tile_state->TILE_H = TILE_H;
  tile_state->N_WIDTH_TILES = N_WIDTH_TILES;
  tile_state->N_HEIGHT_TILES = N_HEIGHT_TILES;
  tile_state->OFFSET_X = GetScreenWidth() / 2;
  tile_state->OFFSET_Y =
    (GetScreenHeight()/2) -
    (tile_state->N_WIDTH_TILES/2 + tile_state->N_HEIGHT_TILES/2) *
    tile_state->TILE_H/2 - tile_state->TILE_H;
}

static const float move_by = 10;
static void react_to_scrolling(TileState* tile_state) {
  // Scrolling
  float wheel = GetMouseWheelMove();
  float zoom = (int)(wheel * 8);
  if (IsKeyPressed(93) || IsKeyDown(93)) zoom = 5; // 93 is + on german keyboard
  if (IsKeyPressed(47) || IsKeyDown(47)) zoom = -5; // 47 is - on german keyboard
  if (zoom != 0) {
    Vector2 m = GetMousePosition();
    int new_tw = (int)Clamp((float)(tile_state->TILE_W + zoom), 32.0f, 144.0f);
    if (new_tw == tile_state->TILE_W) return;
    int new_th = new_tw / 2;

    float dx = (m.x - tile_state->OFFSET_X) / (tile_state->TILE_W / 2.0f);
    float dy = (m.y - tile_state->OFFSET_Y - tile_state->TILE_H) / (tile_state->TILE_H / 2.0f);
    float fx = (dx + dy) / 2.0f;
    float fy = (dy - dx) / 2.0f;

    tile_state->OFFSET_X = (int)(m.x - (fx - fy) * new_tw / 2.0f);
    tile_state->OFFSET_Y = (int)(m.y - (fx + fy) * new_th / 2.0f - new_th);
    tile_state->TILE_W = new_tw;
    tile_state->TILE_H = new_th;
  }
}

static void move_by_moving_mouse_to_borders(TileState* tile_state) {
  const float w = GetScreenWidth();
  const float h = GetScreenHeight();
  Vector2 c = GetMousePosition();

  const float threshold = 30.0;
  if (fabs(c.x) < threshold && fabs(c.y - 0.0) < threshold) {
    tile_state->OFFSET_X += move_by;
    tile_state->OFFSET_Y += move_by;
  }
  if (fabs(c.x - w) < threshold && fabs(c.y - 0.0) < threshold) {
    tile_state->OFFSET_X -= move_by;
    tile_state->OFFSET_Y += move_by;
  }
  if (fabs(c.x) < threshold && fabs(c.y - h) < threshold) {
    tile_state->OFFSET_X += move_by;
    tile_state->OFFSET_Y -= move_by;
  }
  if (fabs(c.x - w) < threshold && fabs(c.y - h) < threshold) {
    tile_state->OFFSET_X -= move_by;
    tile_state->OFFSET_Y -= move_by;
  }
}

static void move_with_keys(TileState* tile_state) {
  // Movement
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    Vector2 delta = GetMouseDelta();
    tile_state->OFFSET_X += (int)delta.x;
    tile_state->OFFSET_Y += (int)delta.y;
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_RIGHT)) {
    tile_state->OFFSET_X -= move_by;
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyDown(KEY_LEFT)) {
    tile_state->OFFSET_X += move_by;
  }
  if (IsKeyPressed(KEY_UP) || IsKeyDown(KEY_UP)) {
    tile_state->OFFSET_Y += move_by;
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyDown(KEY_DOWN)) {
    tile_state->OFFSET_Y -= move_by;
  }
}

void update_tile_state(TileState* tile_state) {
  move_by_moving_mouse_to_borders(tile_state);
  move_with_keys(tile_state);
  react_to_scrolling(tile_state);
}
