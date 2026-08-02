#include "drawing/build_clear_forest.h"
#include "drawing/drawing_helper.h"
#include "units/units.h"

static const Color CLEAR_FOREST_RECT_COL = {200, 120, 0, 110};

static void draw_clear_forest_preview(TileState tile_state, Map *map, int min_tx, int max_tx, int min_ty, int max_ty) {
  for (int ty = min_ty; ty <= max_ty; ty++) {
    for (int tx = min_tx; tx <= max_tx; tx++) {
      const Tile *t = map_tile(map, tx, ty);
      int tz = t ? t->z : 0;
      int sx = (tx - ty) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
      int sy = (tx + ty) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - tz * tile_state.TILE_H;
      draw_diamond(tile_state, sx, sy, CLEAR_FOREST_RECT_COL);
    }
  }
}

// Screen-space drag rectangles don't map to tile-space rectangles under the
// isometric projection (a screen-axis-aligned box is a rotated diamond in
// tile space), so dragging like figure-selection does (a plain screen box)
// while still ending up with a proper tile-aligned RouteBounds means running
// all 4 screen corners through screen_to_tile and taking their min/max,
// instead of converting just the drag start/current point.
static void screen_rect_to_tile_bounds(TileState tile_state, Vector2 a, Vector2 b,
                                       int *out_min_tx, int *out_max_tx, int *out_min_ty, int *out_max_ty) {
  float min_sx = fminf(a.x, b.x), max_sx = fmaxf(a.x, b.x);
  float min_sy = fminf(a.y, b.y), max_sy = fmaxf(a.y, b.y);
  Vector2 corners[4] = {
    {min_sx, min_sy}, {max_sx, min_sy},
    {min_sx, max_sy}, {max_sx, max_sy},
  };
  int min_tx = 0, max_tx = 0, min_ty = 0, max_ty = 0;
  for (int i = 0; i < 4; i++) {
    int tx = screen_to_tile_x(tile_state, corners[i].x, corners[i].y);
    int ty = screen_to_tile_y(tile_state, corners[i].x, corners[i].y);
    if (i == 0 || tx < min_tx) min_tx = tx;
    if (i == 0 || tx > max_tx) max_tx = tx;
    if (i == 0 || ty < min_ty) min_ty = ty;
    if (i == 0 || ty > max_ty) max_ty = ty;
  }
  *out_min_tx = min_tx; *out_max_tx = max_tx;
  *out_min_ty = min_ty; *out_max_ty = max_ty;
}

void update_clear_forest_drag(ClearForestDragState *drag, TileState tile_state, ModeState *mode_state,
                              Map *map, FloodFieldArray *flood_field_state, ObjectArray *objects,
                              GameState *game_state, Selection *selection, bool icon_hovered) {
  if (mode_state->current != BUILD_CLEAR_FOREST) {
    drag->dragging = false;
    return;
  }

  Vector2 current = GetMousePosition();

  if (!drag->dragging) {
    if (icon_hovered || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    drag->dragging = true;
    drag->drag_start = current;
  }

  int min_tx, max_tx, min_ty, max_ty;
  screen_rect_to_tile_bounds(tile_state, drag->drag_start, current, &min_tx, &max_tx, &min_ty, &max_ty);
  draw_clear_forest_preview(tile_state, map, min_tx, max_tx, min_ty, max_ty);

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    RouteBounds bounds = {
      .min_tx = min_tx, .max_tx = max_tx,
      .min_ty = min_ty, .max_ty = max_ty,
      .row_along_tx = (max_tx - min_tx) >= (max_ty - min_ty),
    };

    if (selection->mansus_idx >= 0 && selection->mansus_idx < game_state->mansen.count) {
      // A Mansus (not loose figures) is selected - "this household clears
      // this area" instead of "whichever figures happen to be drag-selected".
      // Wrap its farmer/farmhand ids in a throwaway Selection so it can go
      // through the exact same assignment path as the free-selection case.
      const Mansus *m = &game_state->mansen.data[selection->mansus_idx];
      unsigned int worker_ids[2];
      int n_workers = 0;
      if (m->farmer_object_id != 0) worker_ids[n_workers++] = m->farmer_object_id;
      if (m->farmhand_object_id != 0) worker_ids[n_workers++] = m->farmhand_object_id;
      Selection mansus_workers = {.ids = worker_ids, .count = n_workers};
      assign_clear_forest_job(&mansus_workers, objects, map, flood_field_state, game_state, bounds);
    } else {
      assign_clear_forest_job(selection, objects, map, flood_field_state, game_state, bounds);
    }

    // Deselect right after handing off the job - otherwise the still-selected
    // figures/mansus are one stray left-click away from being re-tasked to
    // whatever single tile that click lands on, since a plain click can't
    // deselect while still in BUILD_CLEAR_FOREST mode (only MOVEMENT does that).
    selection_clear(selection);
    selection->mansus_idx = -1;
    drag->dragging = false;
  }
}
