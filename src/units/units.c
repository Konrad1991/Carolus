#include "units/units.h"
#include "flood_fill/flood_fill.h"
#include "containers/arrays.h"
#include "drawing/drawing_helper.h"
#include "drawing/build_object.h"
#include "drawing/update_figure.h"
#include "raylib.h"
#include <stdlib.h>

static void set_figure_action(Object *fig, FigureAction action, Map *map, FloodFieldArray* flood_field_state) {
  if (fig->figure.flood_field_idx >= 0) {
    figure_release_reservation(map, fig);
    flood_field_array_release(flood_field_state, fig->figure.flood_field_idx);
    fig->figure.flood_field_idx = -1;
  }
  fig->figure.progress = 0.0f;
  fig->figure.prev_tile = -1;
  fig->figure.best_distance_to_target = -1;
  fig->figure.pacing_streak = 0;
  fig->figure.action = action;
  fig->figure.action_timer = 0.0f;
  fig->figure.pending_action = FIGURE_ACTION_STAND;
  fig->figure.gather_tx = fig->tx;
  fig->figure.gather_ty = fig->ty;
  fig->figure.plow_route.phase = PLOW_PHASE_NONE;
  fig->figure.plowing = false;
  fig->figure.harvest_route.phase = HARVEST_PHASE_NONE;
}

static void selected_figures_do(const Selection *sel, ObjectArray *objects, FigureAction action, Map *map, FloodFieldArray* flood_field_state) {
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind == OBJECT_FIGURE && selection_contains(sel, objects->data[i].id)) {
      set_figure_action(&objects->data[i], action, map, flood_field_state);
    }
  }
}

static bool find_field_at(const GameState *game_state, int tx, int ty, int *min_tx, int *max_tx, int *min_ty, int *max_ty) {
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      if (tx < f->corners_field[0][0] || tx > f->corners_field[1][0]) continue;
      if (ty < f->corners_field[0][1] || ty > f->corners_field[1][1]) continue;
      *min_tx = f->corners_field[0][0];
      *min_ty = f->corners_field[0][1];
      *max_tx = f->corners_field[1][0];
      *max_ty = f->corners_field[1][1];
      return true;
    }
  }
  return false;
}

static void start_plow_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                             int min_tx, int max_tx, int min_ty, int max_ty) {
  bool near_min_tx = abs(fig->tx - min_tx) <= abs(fig->tx - max_tx);
  bool near_min_ty = abs(fig->ty - min_ty) <= abs(fig->ty - max_ty);
  int entry_tx = near_min_tx ? min_tx : max_tx;
  int entry_ty = near_min_ty ? min_ty : max_ty;

  PlowRoute *route = &fig->figure.plow_route;
  route->phase = PLOW_PHASE_APPROACH;
  route->row_along_tx = (max_tx - min_tx) >= (max_ty - min_ty);
  route->min_tx = min_tx;
  route->max_tx = max_tx;
  route->min_ty = min_ty;
  route->max_ty = max_ty;
  if (route->row_along_tx) {
    route->step_coord = entry_ty;
    route->step_dir = near_min_ty ? 1 : -1;
    route->sweep_positive = near_min_tx;
  } else {
    route->step_coord = entry_tx;
    route->step_dir = near_min_tx ? 1 : -1;
    route->sweep_positive = near_min_ty;
  }
  fig->figure.plowing = false;

  figure_walk_to(fig, map, flood_field_state, entry_tx, entry_ty);
}

static bool is_farmer_species(FigureSpecies species) {
  return species == FIGURE_SPECIES_FARMER1 || species == FIGURE_SPECIES_FARMER2;
}

static void start_harvest_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                                int min_tx, int max_tx, int min_ty, int max_ty) {
  bool near_min_tx = abs(fig->tx - min_tx) <= abs(fig->tx - max_tx);
  bool near_min_ty = abs(fig->ty - min_ty) <= abs(fig->ty - max_ty);
  int entry_tx = near_min_tx ? min_tx : max_tx;
  int entry_ty = near_min_ty ? min_ty : max_ty;

  HarvestRoute *route = &fig->figure.harvest_route;
  route->phase = HARVEST_PHASE_WALKING;
  route->row_along_tx = (max_tx - min_tx) >= (max_ty - min_ty);
  route->min_tx = min_tx;
  route->max_tx = max_tx;
  route->min_ty = min_ty;
  route->max_ty = max_ty;
  if (route->row_along_tx) {
    route->row = entry_ty;
    route->step_dir = near_min_ty ? 1 : -1;
    route->cursor = entry_tx;
    route->sweep_dir = near_min_tx ? 1 : -1;
  } else {
    route->row = entry_tx;
    route->step_dir = near_min_tx ? 1 : -1;
    route->cursor = entry_ty;
    route->sweep_dir = near_min_ty ? 1 : -1;
  }
  route->mow_timer = 0.0f;

  figure_walk_to(fig, map, flood_field_state, entry_tx, entry_ty);
}

static int calc_n_walkers(const Selection* sel, ObjectArray* objects,
                          const bool field_found, const bool is_tree_target,
                          Map* map, FloodFieldArray* flood_field_state,
                          const int field_min_tx, const int field_max_tx,
                          const int field_min_ty, const int field_max_ty,
                          const int tx, const int ty) {
  int n_walkers = 0;
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind != OBJECT_FIGURE || !selection_contains(sel, objects->data[i].id)) continue;

    if (field_found && objects->data[i].figure.species == FIGURE_SPECIES_OX) {
      start_plow_route(&objects->data[i], map, flood_field_state, field_min_tx, field_max_tx, field_min_ty, field_max_ty);
      continue;
    }
    if (field_found && is_farmer_species(objects->data[i].figure.species)) {
      start_harvest_route(&objects->data[i], map, flood_field_state, field_min_tx, field_max_tx, field_min_ty, field_max_ty);
      continue;
    }
    bool adjacent = is_tree_target &&
      abs(objects->data[i].tx - tx) <= 1 && abs(objects->data[i].ty - ty) <= 1;
    if (adjacent) {
      set_figure_action(&objects->data[i], FIGURE_ACTION_CHOP, map, flood_field_state);
      objects->data[i].figure.gather_tx = tx;
      objects->data[i].figure.gather_ty = ty;
    } else {
      n_walkers++;
    }
  }
  return n_walkers;
}

static void update_figure_attributes(ObjectArray* objects,
                                     const int i, const int field_idx,
                                     const int my_target_tx, const int my_target_ty,
                                     const int tx, const int ty,
                                     const bool lands_adjacent) {
  objects->data[i].figure.flood_field_idx = field_idx;
  objects->data[i].figure.target_tx = my_target_tx;
  objects->data[i].figure.target_ty = my_target_ty;
  objects->data[i].figure.prev_tile = -1;
  objects->data[i].figure.best_distance_to_target = -1;
  objects->data[i].figure.pacing_streak = 0;
  objects->data[i].figure.speed = FIGURE_SPEED_TILES_PER_SECOND;
  objects->data[i].figure.pending_action = lands_adjacent ? FIGURE_ACTION_CHOP : FIGURE_ACTION_STAND;
  objects->data[i].figure.gather_tx = tx;
  objects->data[i].figure.gather_ty = ty;

  if (objects->data[i].figure.species == FIGURE_SPECIES_OX) {
    objects->data[i].figure.plow_route.phase = PLOW_PHASE_NONE;
    objects->data[i].figure.plowing = false;
  }
  if (is_farmer_species(objects->data[i].figure.species)) {
    objects->data[i].figure.harvest_route.phase = HARVEST_PHASE_NONE;
  }
}

void command_selected_units(const Selection* sel, Map* map, TileState tile_state, ObjectArray *objects, FloodFieldArray* flood_field_state, GameState *game_state, bool any_hovered) {
  if (IsKeyPressed(KEY_D)) selected_figures_do(sel, objects, FIGURE_ACTION_DIG, map, flood_field_state);
  if (IsKeyPressed(KEY_C)) selected_figures_do(sel, objects, FIGURE_ACTION_CHOP, map, flood_field_state);
  if (IsKeyPressed(KEY_H)) selected_figures_do(sel, objects, FIGURE_ACTION_HAMMER, map, flood_field_state);
  if (IsKeyPressed(KEY_M)) selected_figures_do(sel, objects, FIGURE_ACTION_MOW, map, flood_field_state);
  if (IsKeyPressed(KEY_S)) {
    for (int i = 0; i < objects->count; i++) {
      if (objects->data[i].kind != OBJECT_FIGURE || !selection_contains(sel, objects->data[i].id)) continue;
      objects->data[i].figure.action = FIGURE_ACTION_SOW;
      objects->data[i].figure.action_timer = 0.0f;
    }
  }

  if (sel->count == 0 || !IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || any_hovered) return;

  Vector2 current = GetMousePosition();
  const int tx = screen_to_tile_x(tile_state, current.x, current.y);
  const int ty = screen_to_tile_y(tile_state, current.x, current.y);
  Tile *target_tile = map_tile(map, tx, ty);
  if (!target_tile) return;

  int field_min_tx, field_max_tx, field_min_ty, field_max_ty;
  bool field_found = target_tile->type == TILE_SOIL &&
    find_field_at(game_state, tx, ty, &field_min_tx, &field_max_tx, &field_min_ty, &field_max_ty);

  int tree_idx = find_object_at_tile(objects, tx, ty);
  bool is_tree_target = tree_idx >= 0 && objects->data[tree_idx].kind == OBJECT_TREE;

  int n_walkers = calc_n_walkers(sel, objects, field_found, is_tree_target,
                                 map, flood_field_state,
                                 field_min_tx, field_max_tx, field_min_ty, field_max_ty,
                                 tx, ty);
  if (n_walkers == 0) return;

  int *target_x = malloc(n_walkers * sizeof(int));
  int *target_y = malloc(n_walkers * sizeof(int));
  int n_targets = map_free_tiles_near(map, tx, ty, n_walkers, target_x, target_y);

  int *figure_nodes = malloc(n_targets * sizeof(int));
  for (int t = 0; t < n_targets; t++) {
    figure_nodes[t] = node_index(target_x[t], target_y[t], map->w);
  }
  FloodField field = flood_fill(map, figure_nodes, n_targets, true);
  field.n_figures = n_targets;
  field.active = true;
  int field_idx = flood_field_array_push(flood_field_state, field);
  free(figure_nodes);

  bool *target_used = calloc(n_targets, sizeof(bool));

  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind != OBJECT_FIGURE || !selection_contains(sel, objects->data[i].id)) continue;
    if (field_found && objects->data[i].figure.species == FIGURE_SPECIES_OX) continue;
    if (field_found && is_farmer_species(objects->data[i].figure.species)) continue;
    bool adjacent = is_tree_target && abs(objects->data[i].tx - tx) <= 1 && abs(objects->data[i].ty - ty) <= 1;
    if (adjacent) continue;

    int best_t = -1;
    int best_dist2 = 0;
    for (int t = 0; t < n_targets; t++) {
      if (target_used[t]) continue;
      int dtx = objects->data[i].tx - target_x[t];
      int dty = objects->data[i].ty - target_y[t];
      int dist2 = dtx * dtx + dty * dty;
      if (best_t < 0 || dist2 < best_dist2) {
        best_t = t;
        best_dist2 = dist2;
      }
    }
    if (best_t < 0) break;
    target_used[best_t] = true;

    bool lands_adjacent = is_tree_target &&
      abs(target_x[best_t] - tx) <= 1 && abs(target_y[best_t] - ty) <= 1;
    int my_target_tx = target_x[best_t];
    int my_target_ty = target_y[best_t];

    if (objects->data[i].figure.flood_field_idx >= 0) {
      flood_field_array_release(flood_field_state, objects->data[i].figure.flood_field_idx);
    }
    update_figure_attributes(objects, i, field_idx, my_target_tx, my_target_ty, tx, ty, lands_adjacent);
  }

  free(target_used);
  free(target_x);
  free(target_y);
}
