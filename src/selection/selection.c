#include "selection/selection.h"
#include "drawing/drawing_helper.h"
#include "drawing/build_object.h"
#include "units/units.h"
#include "raylib.h"
#include <stdlib.h>
#include <math.h>

void selection_clear(Selection *sel) {
  sel->count = 0;
}

void selection_add(Selection *sel, unsigned int id) {
  if (sel->count >= sel->capacity) {
    sel->capacity = sel->capacity > 0 ? sel->capacity * 2 : 8;
    sel->ids = realloc(sel->ids, sel->capacity * sizeof(unsigned int));
  }
  sel->ids[sel->count++] = id;
}

bool selection_contains(const Selection *sel, unsigned int id) {
  for (int i = 0; i < sel->count; i++) {
    if (sel->ids[i] == id) return true;
  }
  return false;
}

void selection_free(Selection *sel) {
  free(sel->ids);
  sel->ids = NULL;
  sel->count = 0;
  sel->capacity = 0;
}

// Returns the Mansus index for a click at (tx, ty): either its barn/living house
// building or a tile inside its yard. Sets *out_barn_selected when the barn itself was hit.
static int mansus_at_click(GameState *game_state, const ObjectArray *objects, int tx, int ty, bool *out_barn_selected) {
  *out_barn_selected = false;

  int obj_idx = find_object_at_tile(objects, tx, ty);
  if (obj_idx >= 0 && objects->data[obj_idx].kind == OBJECT_BUILDING) {
    unsigned int building_id = objects->data[obj_idx].id;
    for (int mi = 0; mi < game_state->mansen.count; mi++) {
      const Mansus *m = &game_state->mansen.data[mi];
      if (m->barn == building_id) { *out_barn_selected = true; return mi; }
      if (m->living_house == building_id) return mi;
    }
  }

  return find_mansus_at(game_state, tx, ty);
}

void update_selection(Selection *sel, TileState tile_state, ModeState mode_state,
                      const ObjectArray *objects, GameState *game_state, bool icon_hovered) {
  if (mode_state.current != MOVEMENT) return;

  if (!icon_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    sel->dragging = true;
    sel->drag_start = GetMousePosition();
  }

  if (sel->dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    sel->dragging = false;
    Vector2 drag_end = GetMousePosition();

    const Rectangle selection_rectangle = {
      fminf(sel->drag_start.x, drag_end.x),
      fminf(sel->drag_start.y, drag_end.y),
      fabsf(drag_end.x - sel->drag_start.x),
      fabsf(drag_end.y - sel->drag_start.y),
    };

    const float click_tolerance_px = 4.0f;
    bool is_click = selection_rectangle.width <= click_tolerance_px && selection_rectangle.height <= click_tolerance_px;

    if (is_click) {
      int tx = screen_to_tile_x(tile_state, drag_end.x, drag_end.y);
      int ty = screen_to_tile_y(tile_state, drag_end.x, drag_end.y);
      bool barn_selected = false;
      int mansus_idx = mansus_at_click(game_state, objects, tx, ty, &barn_selected);
      if (mansus_idx >= 0) {
        selection_clear(sel);
        sel->mansus_idx = mansus_idx;
        sel->barn_selected = barn_selected;
        sel->field_mansus_idx = -1;
        sel->field_idx = -1;
        return;
      }

      int field_mansus_idx = -1;
      Field *field = find_field_at(game_state, tx, ty, &field_mansus_idx);
      if (field) {
        selection_clear(sel);
        sel->mansus_idx = -1;
        sel->barn_selected = false;
        sel->field_mansus_idx = field_mansus_idx;
        sel->field_idx = (int)(field - game_state->mansen.data[field_mansus_idx].fields.data);
        return;
      }
    }

    sel->mansus_idx = -1;
    sel->barn_selected = false;
    sel->field_mansus_idx = -1;
    sel->field_idx = -1;
    selection_clear(sel);
    for (int i = 0; i < objects->count; i++) {
      const Object *o = &objects->data[i];
      if (o->kind != OBJECT_FIGURE) continue;
      const Rectangle figure_rectangle =  calc_object_screen_rectangle(tile_state, o);
      if (CheckCollisionRecs(selection_rectangle, figure_rectangle)) {
        selection_add(sel, o->id);
      }
    }
  }
}

void draw_selection_rect(const Selection *sel) {
  if (!sel->dragging) return;
  Vector2 current = GetMousePosition();
  int x = (int)fminf(sel->drag_start.x, current.x);
  int y = (int)fminf(sel->drag_start.y, current.y);
  int w = (int)fabsf(current.x - sel->drag_start.x);
  int h = (int)fabsf(current.y - sel->drag_start.y);
  DrawRectangle(x, y, w, h, Fade(SKYBLUE, 0.25f));
  DrawRectangleLines(x, y, w, h, SKYBLUE);
}
