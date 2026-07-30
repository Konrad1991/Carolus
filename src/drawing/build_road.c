#include "drawing/build_road.h"
#include "drawing/drawing_helper.h"
#include "drawing/build_object.h"
#include "a_star/a_star.h"

static const Color ROAD_PATH_OK_COL = {0, 200, 0, 110};
static const Color ROAD_PATH_BLOCKED_COL = {200, 0, 0, 110};

static void draw_path_preview(TileState tile_state, Map *map, const PathResult *path) {
  for (int i = 0; i < path->solution_len; i++) {
    int x, y;
    node_to_xy(path->solution[i], map->w, &x, &y);
    const Tile *t = map_tile(map, x, y);
    int tz = t ? t->z : 0;
    int sx = (x - y) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
    int sy = (x + y) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - tz * tile_state.TILE_H;
    bool placeable = map_is_placeable(map, x, y, 1, 1);
    draw_diamond(tile_state, sx, sy, placeable ? ROAD_PATH_OK_COL : ROAD_PATH_BLOCKED_COL);
  }
}

void update_road_drag(RoadDragState *drag, TileState tile_state, ModeState *mode_state,
                       Map *map, ObjectArray *objects, Texture_State *texture_state,
                       bool icon_hovered) {
  if (mode_state->current != BUILD_ROAD) {
    if (drag->dragging) {
      path_result_free(&drag->path);
      drag->dragging = false;
    }
    return;
  }

  Vector2 current = GetMousePosition();
  int tx = screen_to_tile_x(tile_state, current.x, current.y);
  int ty = screen_to_tile_y(tile_state, current.x, current.y);

  if (!drag->dragging) {
    if (icon_hovered || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    drag->dragging = true;
    drag->start_tx = tx;
    drag->start_ty = ty;
  }

  path_result_free(&drag->path);
  int start_node = node_index(drag->start_tx, drag->start_ty, map->w);
  int end_node = node_index(tx, ty, map->w);
  drag->path = a_star(map, start_node, end_node, false);
  draw_path_preview(tile_state, map, &drag->path);

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    for (int i = 0; i < drag->path.solution_len; i++) {
      int x;
      int y;
      node_to_xy(drag->path.solution[i], map->w, &x, &y);
      place_road_tile(objects, map, texture_state, x, y);
    }
    path_result_free(&drag->path);
    drag->dragging = false;
  }
}
