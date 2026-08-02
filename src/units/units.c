#include "units/units.h"
#include "flood_fill/flood_fill.h"
#include "containers/arrays.h"
#include "drawing/drawing_helper.h"
#include "drawing/build_object.h"
#include "drawing/update_figure.h"
#include "raylib.h"
#include <stdlib.h>

static void set_figure_action(Object *fig, FigureAction action, Map *map, FloodFieldArray* flood_field_state, GameState *game_state) {
  if (fig->figure.flood_field_idx >= 0) {
    figure_release_reservation(map, fig);
    flood_field_array_release(flood_field_state, fig->figure.flood_field_idx);
    fig->figure.flood_field_idx = -1;
  } else if (fig->figure.direct_walking) {
    figure_release_reservation(map, fig);
  }
  fig->figure.direct_walking = false;
  fig->figure.progress = 0.0f;
  fig->figure.prev_tile = -1;
  fig->figure.best_distance_to_target = -1;
  fig->figure.pacing_streak = 0;
  fig->figure.action = action;
  fig->figure.action_timer = 0.0f;
  fig->figure.pending_action = FIGURE_ACTION_STAND;
  fig->figure.gather = (Position){fig->tx, fig->ty};
  fig->figure.plow_route.phase = PLOW_PHASE_NONE;
  fig->figure.plowing = false;
  fig->figure.harvest_route.phase = HARVEST_PHASE_NONE;
  fig->figure.sow_route.phase = SOW_PHASE_NONE;
  fig->figure.dig_route.phase = DIG_PHASE_NONE;
  fig->figure.wood_route.phase = WOOD_PHASE_NONE;
  release_field_lock(game_state, fig->id);
}

static void selected_figures_do(const Selection *sel, ObjectArray *objects, FigureAction action, Map *map, FloodFieldArray* flood_field_state, GameState *game_state) {
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind == OBJECT_FIGURE && selection_contains(sel, objects->data[i].id)) {
      set_figure_action(&objects->data[i], action, map, flood_field_state, game_state);
    }
  }
}

Field *find_field_at(GameState *game_state, int tx, int ty, int *out_mansus_idx) {
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      Field *f = &m->fields.data[fi];
      if (tx < f->corners_field[0][0] || tx > f->corners_field[1][0]) continue;
      if (ty < f->corners_field[0][1] || ty > f->corners_field[1][1]) continue;
      *out_mansus_idx = mi;
      return f;
    }
  }
  return NULL;
}

// Releases the lock this figure holds on whatever field it was working, if any.
// Called before starting a new route and whenever a figure's route is interrupted,
// so a field never stays locked once nobody is actually working it anymore.
void release_field_lock(GameState *game_state, unsigned int figure_id) {
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      Field *f = &m->fields.data[fi];
      if (f->worked_by_figure_id == figure_id) f->worked_by_figure_id = 0;
    }
  }
}

static void set_route_bounds(RouteBounds* route_bounds, Field* field) {
  int min_tx = field->corners_field[0][0];
  int max_tx = field->corners_field[1][0];
  int min_ty = field->corners_field[0][1];
  int max_ty = field->corners_field[1][1];
  route_bounds->row_along_tx = (max_tx - min_tx) >= (max_ty - min_ty);
  route_bounds->min_tx = min_tx;
  route_bounds->max_tx = max_tx;
  route_bounds->min_ty = min_ty;
  route_bounds->max_ty = max_ty;
}

static Position calc_entry_point(RouteBounds* route_bounds, Object* fig, bool* near_min_tx, bool* near_min_ty) {
  Position position = {0};
  *near_min_tx = abs(fig->tx - route_bounds->min_tx) <= abs(fig->tx - route_bounds->max_tx);
  *near_min_ty = abs(fig->ty - route_bounds->min_ty) <= abs(fig->ty - route_bounds->max_ty);
  position.x = (*near_min_tx) ? route_bounds->min_tx : route_bounds->max_tx;
  position.y = (*near_min_ty) ? route_bounds->min_ty : route_bounds->max_ty;
  return position;
}

static RowSweepState calc_row_sweep_state(const RouteBounds* route_bounds, Position position, bool near_min_tx, bool near_min_ty) {
  RowSweepState row_sweep_state;
  if (route_bounds->row_along_tx) {
    row_sweep_state.row = position.y;
    row_sweep_state.step_dir = near_min_ty ? 1 : -1;
    row_sweep_state.cursor = position.x;
    row_sweep_state.sweep_dir = near_min_tx ? 1 : -1;
  } else {
    row_sweep_state.row = position.x;
    row_sweep_state.step_dir = near_min_tx ? 1 : -1;
    row_sweep_state.cursor = position.y;
    row_sweep_state.sweep_dir = near_min_ty ? 1 : -1;
  }
  return row_sweep_state;
}

void start_plow_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                      GameState *game_state, Field *field) {
  release_field_lock(game_state, fig->id);
  field->worked_by_figure_id = fig->id;

  PlowRoute *route = &fig->figure.plow_route;
  set_route_bounds(&route->route_bounds, field);
  bool near_min_tx;
  bool near_min_ty;
  Position position = calc_entry_point(&route->route_bounds, fig, &near_min_tx, &near_min_ty);
  route->phase = PLOW_PHASE_APPROACH;
  if (route->route_bounds.row_along_tx) {
    route->step_coord = position.y;
    route->step_dir = near_min_ty ? 1 : -1;
    route->sweep_positive = near_min_tx;
  } else {
    route->step_coord = position.x;
    route->step_dir = near_min_tx ? 1 : -1;
    route->sweep_positive = near_min_ty;
  }
  fig->figure.plowing = false;
  figure_walk_to(fig, map, flood_field_state, position);
}

static bool is_farmer_species(FigureSpecies species) {
  return species == FIGURE_SPECIES_FARMER1 || species == FIGURE_SPECIES_FARMER2;
}

static const Familia MANSUS_LEVEL_FAMILIA[] = {
  [HOUSLER]        = {.children = 1, .adult_children = 0, .elders = 0, .farmhands = 0, .maids = 0},
  [QUARTER_HUFNER] = {.children = 2, .adult_children = 0, .elders = 0, .farmhands = 0, .maids = 0},
  [HALF_HUFNER]    = {.children = 3, .adult_children = 1, .elders = 1, .farmhands = 1, .maids = 0},
  [FULLHUFNER]     = {.children = 4, .adult_children = 1, .elders = 1, .farmhands = 1, .maids = 1},
  [RICH_FARMER]    = {.children = 5, .adult_children = 2, .elders = 1, .farmhands = 2, .maids = 2},
};

static bool figure_already_assigned(const GameState *game_state, unsigned int figure_id) {
  for (int i = 0; i < game_state->mansen.count; i++) {
    if (game_state->mansen.data[i].farmer_object_id == figure_id) return true;
    if (game_state->mansen.data[i].farmhand_object_id == figure_id) return true;
  }
  return false;
}

// FARMER1 (straw hat) becomes the Bauer (head of household), FARMER2 becomes the Knecht (farmhand).
static bool assign_figures_to_mansus(const Selection *sel, ObjectArray *objects, GameState *game_state, int mansus_idx) {
  Mansus *mansus = &game_state->mansen.data[mansus_idx];
  bool assigned_any = false;

  for (int i = 0; i < objects->count; i++) {
    Object *fig = &objects->data[i];
    if (fig->kind != OBJECT_FIGURE || !selection_contains(sel, fig->id)) continue;
    if (!is_farmer_species(fig->figure.species)) continue;
    if (figure_already_assigned(game_state, fig->id)) continue;

    if (fig->figure.species == FIGURE_SPECIES_FARMER1 && mansus->farmer_object_id == 0) {
      mansus->farmer_object_id = fig->id;
      mansus->familia = MANSUS_LEVEL_FAMILIA[mansus->mansus_level];
      mansus->assign_flash_timer = MANSUS_ASSIGN_FLASH_SECONDS;
      assigned_any = true;
    } else if (fig->figure.species == FIGURE_SPECIES_FARMER2 && mansus->farmhand_object_id == 0) {
      mansus->farmhand_object_id = fig->id;
      mansus->assign_flash_timer = MANSUS_ASSIGN_FLASH_SECONDS;
      assigned_any = true;
    }
  }
  return assigned_any;
}

void start_harvest_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                         GameState *game_state, Field *field) {
  release_field_lock(game_state, fig->id);
  field->worked_by_figure_id = fig->id;
  HarvestRoute *route = &fig->figure.harvest_route;
  set_route_bounds(&route->route_bounds, field);
  bool near_min_tx;
  bool near_min_ty;
  Position position = calc_entry_point(&route->route_bounds, fig, &near_min_tx, &near_min_ty);
  route->phase = HARVEST_PHASE_WALKING;
  route->row_sweep_state = calc_row_sweep_state(&route->route_bounds, position, near_min_tx, near_min_ty);
  route->mow_timer = 0.0f;
  route->mowing_done = false;
  Position stand_position;
  harvest_route_stand_tile(route, &stand_position);
  figure_walk_to(fig, map, flood_field_state, stand_position);
  fig->figure.gather = position;
}

void start_dig_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                     GameState *game_state, Field *field) {
  release_field_lock(game_state, fig->id);
  field->worked_by_figure_id = fig->id;
  DigRoute *route = &fig->figure.dig_route;
  route->phase = DIG_PHASE_WALKING;
  set_route_bounds(&route->route_bounds, field);
  bool near_min_tx;
  bool near_min_ty;
  Position position = calc_entry_point(&route->route_bounds, fig, &near_min_tx, &near_min_ty);
  route->row_sweep_state = calc_row_sweep_state(&route->route_bounds, position, near_min_tx, near_min_ty);
  route->dig_timer = 0.0f;
  figure_walk_to(fig, map, flood_field_state, position);
}

void start_sow_route(Object *fig, Map *map, FloodFieldArray *flood_field_state,
                     GameState *game_state, Mansus *mansus, Field *field) {
  release_field_lock(game_state, fig->id);
  field->worked_by_figure_id = fig->id;
  mansus->goods.grains -= FIELD_SOW_GRAIN_COST;
  SowRoute *route = &fig->figure.sow_route;
  route->phase = SOW_PHASE_WALKING;
  set_route_bounds(&route->route_bounds, field);
  bool near_min_tx;
  bool near_min_ty;
  Position position = calc_entry_point(&route->route_bounds, fig, &near_min_tx, &near_min_ty);
  route->row_sweep_state = calc_row_sweep_state(&route->route_bounds, position, near_min_tx, near_min_ty);
  // Sowing walks the figure onto the tile itself, unlike harvesting's stand-beside offset.
  figure_walk_to(fig, map, flood_field_state, position);
}

static int calc_n_walkers(const Selection* sel, ObjectArray* objects,
                          const bool is_tree_target,
                          Map* map, FloodFieldArray* flood_field_state,
                          GameState *game_state, Field *field,
                          const int tx, const int ty) {
  int n_walkers = 0;
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind != OBJECT_FIGURE || !selection_contains(sel, objects->data[i].id)) continue;
    unsigned int fig_id = objects->data[i].id;

    if (field && objects->data[i].figure.species == FIGURE_SPECIES_OX) {
      if (field->worked_by_figure_id != 0 && field->worked_by_figure_id != fig_id) continue;
      start_plow_route(&objects->data[i], map, flood_field_state, game_state, field);
      continue;
    }
    bool adjacent = is_tree_target &&
      abs(objects->data[i].tx - tx) <= 1 && abs(objects->data[i].ty - ty) <= 1;
    if (adjacent) {
      set_figure_action(&objects->data[i], FIGURE_ACTION_CHOP, map, flood_field_state, game_state);
      objects->data[i].figure.gather = (Position){tx, ty};
    } else {
      n_walkers++;
    }
  }
  return n_walkers;
}

static void update_figure_attributes(ObjectArray* objects, GameState *game_state,
                                     const int i, const int field_idx,
                                     const int my_target_tx, const int my_target_ty,
                                     const int tx, const int ty,
                                     const bool lands_adjacent) {
  objects->data[i].figure.flood_field_idx = field_idx;
  objects->data[i].figure.direct_walking = false;
  objects->data[i].figure.target = (Position){my_target_tx, my_target_ty};
  objects->data[i].figure.prev_tile = -1;
  objects->data[i].figure.best_distance_to_target = -1;
  objects->data[i].figure.pacing_streak = 0;
  objects->data[i].figure.speed = FIGURE_SPEED_TILES_PER_SECOND;
  objects->data[i].figure.pending_action = lands_adjacent ? FIGURE_ACTION_CHOP : FIGURE_ACTION_STAND;
  objects->data[i].figure.gather = (Position){tx, ty};

  if (objects->data[i].figure.species == FIGURE_SPECIES_OX) {
    objects->data[i].figure.plow_route.phase = PLOW_PHASE_NONE;
    objects->data[i].figure.plowing = false;
  }
  if (is_farmer_species(objects->data[i].figure.species)) {
    objects->data[i].figure.harvest_route.phase = HARVEST_PHASE_NONE;
    objects->data[i].figure.sow_route.phase = SOW_PHASE_NONE;
    objects->data[i].figure.dig_route.phase = DIG_PHASE_NONE;
    objects->data[i].figure.wood_route.phase = WOOD_PHASE_NONE;
  }
  release_field_lock(game_state, objects->data[i].id);
}

void command_selected_units(const Selection* sel, Map* map, TileState tile_state, ObjectArray *objects, FloodFieldArray* flood_field_state, GameState *game_state, bool any_hovered) {
  if (IsKeyPressed(KEY_D)) selected_figures_do(sel, objects, FIGURE_ACTION_DIG, map, flood_field_state, game_state);
  if (IsKeyPressed(KEY_C)) selected_figures_do(sel, objects, FIGURE_ACTION_CHOP, map, flood_field_state, game_state);
  if (IsKeyPressed(KEY_H)) selected_figures_do(sel, objects, FIGURE_ACTION_HAMMER, map, flood_field_state, game_state);
  if (IsKeyPressed(KEY_M)) selected_figures_do(sel, objects, FIGURE_ACTION_MOW, map, flood_field_state, game_state);

  if (sel->count == 0 || !IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || any_hovered) return;

  Vector2 current = GetMousePosition();
  int tree_idx = find_tree_at_screen_pos(objects, tile_state, current);
  bool is_tree_target = tree_idx >= 0;
  const int tx = is_tree_target ? objects->data[tree_idx].tx : screen_to_tile_x(tile_state, current.x, current.y);
  const int ty = is_tree_target ? objects->data[tree_idx].ty : screen_to_tile_y(tile_state, current.x, current.y);
  Tile *target_tile = map_tile(map, tx, ty);
  if (!target_tile) return;

  if (!is_tree_target && target_tile->type == TILE_MANSUSYARD) {
    int mansus_idx = find_mansus_at(game_state, tx, ty);
    if (mansus_idx >= 0 && assign_figures_to_mansus(sel, objects, game_state, mansus_idx)) return;
  }

  bool on_field_ground = !is_tree_target && (target_tile->type == TILE_SOIL || target_tile->type == TILE_GRASS);
  int field_mansus_idx = -1;
  Field *field = on_field_ground ? find_field_at(game_state, tx, ty, &field_mansus_idx) : NULL;
  bool field_found = field != NULL;

  int n_walkers = calc_n_walkers(sel, objects, is_tree_target,
                                 map, flood_field_state,
                                 game_state, field, tx, ty);
  if (n_walkers == 0) return;

  int *target_x = malloc(n_walkers * sizeof(int));
  int *target_y = malloc(n_walkers * sizeof(int));
  int n_targets = map_free_tiles_near(map, tx, ty, n_walkers, target_x, target_y);

  int *figure_nodes = malloc(n_targets * sizeof(int));
  for (int t = 0; t < n_targets; t++) {
    figure_nodes[t] = node_index(target_x[t], target_y[t], map->w);
  }
  FloodField flood_field = flood_fill(map, figure_nodes, n_targets, true);
  flood_field.n_figures = n_targets;
  flood_field.active = true;
  int field_idx = flood_field_array_push(flood_field_state, flood_field);
  free(figure_nodes);

  bool *target_used = calloc(n_targets, sizeof(bool));

  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind != OBJECT_FIGURE || !selection_contains(sel, objects->data[i].id)) continue;
    if (field_found && objects->data[i].figure.species == FIGURE_SPECIES_OX) continue;
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
    update_figure_attributes(objects, game_state, i, field_idx, my_target_tx, my_target_ty, tx, ty, lands_adjacent);
  }

  free(target_used);
  free(target_x);
  free(target_y);
}
