#include "selection/selection.h"
#include "drawing/drawing_helper.h"
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

void update_selection(Selection *sel, TileState tile_state, ModeState mode_state,
                      const ObjectArray *objects, bool icon_hovered) {
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
