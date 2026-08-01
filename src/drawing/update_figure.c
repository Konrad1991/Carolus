#include "containers/arrays.h"
#include "flood_fill/flood_fill.h"
#include "drawing/update_figure.h"
#include "drawing/drawing_helper.h"
#include "textures/textures.h"
#include "utils/game_time.h"
#include "units/units.h"
#include <math.h>


FigureDirection detect_figure_direction(const int x0, const int x1, const int y0, const int y1) {
  const int dx = x1 - x0;
  const int dy = y1 - y0;
  if (dx == 0 && dy > 0) return FIGURE_DIR_FRONT;
  if (dx > 0 && dy > 0) return FIGURE_DIR_FRONT_RIGHT;
  if (dx > 0 && dy == 0) return FIGURE_DIR_RIGHT;
  if (dx > 0 && dy < 0) return FIGURE_DIR_BACK_RIGHT;
  if (dx == 0 && dy < 0) return FIGURE_DIR_BACK;
  if (dx < 0 && dy < 0) return FIGURE_DIR_BACK_LEFT;
  if (dx < 0 && dy == 0) return FIGURE_DIR_LEFT;
  if (dx < 0 && dy > 0) return FIGURE_DIR_FRONT_LEFT;
  return FIGURE_DIR_COUNT;
}

static void apply_gather_pose(Object *figure) {
  int gather_x = figure->figure.gather_tx;
  int gather_y = figure->figure.gather_ty;
  float delta_x = (float)(gather_x - figure->tx);
  float delta_y = (float)(gather_y - figure->ty);
  float distance = sqrtf(delta_x * delta_x + delta_y * delta_y);
  if (distance < 0.001f) {
    figure->draw_x = (float)figure->tx;
    figure->draw_y = (float)figure->ty;
    return;
  }
  if (figure->figure.action == FIGURE_ACTION_CHOP || figure->figure.action == FIGURE_ACTION_MOW) {
    FigureDirection direction = detect_figure_direction(figure->tx, gather_x, figure->ty, gather_y);
    if (direction != FIGURE_DIR_COUNT) figure->facing = direction;
  }
  const float lean = 0.3f;
  figure->draw_x = (float)figure->tx + (delta_x / distance) * lean;
  figure->draw_y = (float)figure->ty + (delta_y / distance) * lean;
}

static void apply_stationary_sprite(Object *figure, const Texture_State *texture_state) {
  const float figure_action_fps = 8.0f;
  if (figure->figure.species == FIGURE_SPECIES_OX) {
    figure->figure.action_timer = 0.0f; // ox stand pose has no animation
    figure->sprite = figure->figure.plowing
      ? texture_state->plow_stand[figure->facing]
      : texture_state->ox_stand[figure->facing];
    return;
  }
  FigureAction display_action = figure->figure.action;
  if (display_action == FIGURE_ACTION_SOW) display_action = FIGURE_ACTION_STAND;
  int frame_count = figure_action_frame_count(display_action);
  int frame = 0;
  if (frame_count > 1) {
    figure->figure.action_timer += game_delta_time();
    frame = (int)(figure->figure.action_timer * figure_action_fps) % frame_count;
  } else {
    figure->figure.action_timer = 0.0f;
  }
  const SpriteAsset *sprite = &texture_state->farmers[figure->figure.species][display_action][figure->facing][frame];
  figure->sprite.tex = sprite->tex;
  figure->sprite.anchor = sprite->anchor;
  figure->sprite.scale = sprite->scale;
}

static void handle_idle(Object *figure, const Texture_State *texture_state) {
  if (figure->figure.action == FIGURE_ACTION_WALK) {
    figure->figure.action = FIGURE_ACTION_STAND;
  }
  apply_gather_pose(figure);
  apply_stationary_sprite(figure, texture_state);
}

void figure_release_reservation(Map *map, Object *figure) {
  if (figure->figure.progress <= 0.0f) return;
  Tile *tile = map_tile(map, figure->figure.step_tx, figure->figure.step_ty);
  if (tile) tile->figure_occupied = 0;
}

static void apply_walk_sprite(Object *figure, const Texture_State *texture_state,
                              int segment_start_x, int segment_start_y, int segment_end_x, int segment_end_y) {
  FigureDirection direction = detect_figure_direction(segment_start_x, segment_end_x, segment_start_y, segment_end_y);
  if (direction == FIGURE_DIR_COUNT) return;
  figure->facing = direction;
  if (figure->figure.species == FIGURE_SPECIES_OX) {
    if (figure->figure.plowing) {
      int frame = (int)(figure->figure.progress * PLOW_WALK_FRAMES);
      if (frame >= PLOW_WALK_FRAMES) frame = PLOW_WALK_FRAMES - 1;
      figure->sprite = texture_state->plow_walk[direction][frame];
    } else {
      int frame = (int)(figure->figure.progress * OX_WALK_FRAMES);
      if (frame >= OX_WALK_FRAMES) frame = OX_WALK_FRAMES - 1;
      figure->sprite = texture_state->ox_walk[direction][frame];
    }
    return;
  }
  int frame_count = figure_action_frame_count(figure->figure.action);
  int walk_frame = (int)(figure->figure.progress * frame_count);
  if (walk_frame >= frame_count) walk_frame = frame_count - 1;
  const SpriteAsset *sprite = &texture_state->farmers[figure->figure.species][figure->figure.action][direction][walk_frame];
  figure->sprite.tex = sprite->tex;
  figure->sprite.anchor = sprite->anchor;
  figure->sprite.scale = sprite->scale;
}

static float step_distance_between_tiles(int start_x, int start_y, int end_x, int end_y) {
  bool diagonal = start_x != end_x && start_y != end_y;
  return diagonal ? sqrtf(2.0f) : 1.0f;
}

static void interpolate_draw_position(Object *figure, int segment_start_x, int segment_start_y, int segment_end_x, int segment_end_y) {
  float progress = figure->figure.progress;
  figure->draw_x = segment_start_x + (segment_end_x - segment_start_x) * progress;
  figure->draw_y = segment_start_y + (segment_end_y - segment_start_y) * progress;
}

static void commit_tile_arrival(Object *figure, Map *map, int departed_tile, int arrival_x, int arrival_y) {
  figure->figure.progress = 0.0f;
  figure->figure.prev_tile = departed_tile;
  Tile *departed_map_tile = map_tile(map, figure->tx, figure->ty);
  if (departed_map_tile) departed_map_tile->figure_occupied = 0;
  figure->tx = arrival_x;
  figure->ty = arrival_y;
  Tile *arrival_map_tile = map_tile(map, figure->tx, figure->ty);
  if (arrival_map_tile) arrival_map_tile->figure_occupied = 1;
  figure->draw_x = (float)figure->tx;
  figure->draw_y = (float)figure->ty;
}

static bool advance_along_segment(Object *figure, Map *map, int current_tile, int next_tile, float speed,
                                  int *segment_start_x, int *segment_start_y, int *segment_end_x, int *segment_end_y) {
  node_to_xy(current_tile, map->w, segment_start_x, segment_start_y);
  node_to_xy(next_tile, map->w, segment_end_x, segment_end_y);
  float step_distance = step_distance_between_tiles(*segment_start_x, *segment_start_y, *segment_end_x, *segment_end_y);
  figure->figure.progress += (speed / step_distance) * game_delta_time();
  if (figure->figure.progress < 1.0f) {
    interpolate_draw_position(figure, *segment_start_x, *segment_start_y, *segment_end_x, *segment_end_y);
    return false;
  }
  commit_tile_arrival(figure, map, current_tile, *segment_end_x, *segment_end_y);
  return true;
}

void figure_walk_to(Object *figure, Map *map, FloodFieldArray *flood_field_state, int target_tx, int target_ty) {
  if (figure->figure.flood_field_idx >= 0) {
    figure_release_reservation(map, figure);
    flood_field_array_release(flood_field_state, figure->figure.flood_field_idx);
  } else if (figure->figure.direct_walking) {
    figure_release_reservation(map, figure);
  }
  figure->figure.direct_walking = false;
  int node = node_index(target_tx, target_ty, map->w);
  FloodField field = flood_fill(map, &node, 1, true);
  field.n_figures = 1;
  field.active = true;

  figure->figure.flood_field_idx = flood_field_array_push(flood_field_state, field);
  figure->figure.target_tx = target_tx;
  figure->figure.target_ty = target_ty;
  figure->figure.prev_tile = -1;
  figure->figure.best_distance_to_target = -1;
  figure->figure.pacing_streak = 0;
  figure->figure.speed = FIGURE_SPEED_TILES_PER_SECOND;
  figure->figure.pending_action = FIGURE_ACTION_STAND;
  figure->figure.gather_tx = target_tx;
  figure->figure.gather_ty = target_ty;
}

// For short local hops within an already-known-clear area (harvest/plow route steps):
// steps straight toward the target one tile at a time instead of flooding the whole map.
static void figure_walk_to_direct(Object *figure, Map *map, FloodFieldArray *flood_field_state, int target_tx, int target_ty) {
  if (figure->figure.flood_field_idx >= 0) {
    figure_release_reservation(map, figure);
    flood_field_array_release(flood_field_state, figure->figure.flood_field_idx);
    figure->figure.flood_field_idx = -1;
  }
  figure->figure.direct_walking = true;
  figure->figure.target_tx = target_tx;
  figure->figure.target_ty = target_ty;
  figure->figure.progress = 0.0f;
  figure->figure.prev_tile = -1;
  figure->figure.best_distance_to_target = -1;
  figure->figure.pacing_streak = 0;
  figure->figure.speed = FIGURE_SPEED_TILES_PER_SECOND;
  figure->figure.pending_action = FIGURE_ACTION_STAND;
  figure->figure.gather_tx = target_tx;
  figure->figure.gather_ty = target_ty;
}

static void start_sweep_leg(Object *figure, Map *map, FloodFieldArray *flood_field_state) {
  PlowRoute *route = &figure->figure.plow_route;
  if (route->row_along_tx) {
    figure_walk_to_direct(figure, map, flood_field_state, route->sweep_positive ? route->max_tx : route->min_tx, route->step_coord);
  } else {
    figure_walk_to_direct(figure, map, flood_field_state, route->step_coord, route->sweep_positive ? route->max_ty : route->min_ty);
  }
}

static void advance_plow_route(Object *figure, Map *map, FloodFieldArray *flood_field_state, GameState *game_state) {
  PlowRoute *route = &figure->figure.plow_route;
  switch (route->phase) {
    case PLOW_PHASE_NONE:
      return;
    case PLOW_PHASE_APPROACH:
      figure->figure.plowing = true;
      route->phase = PLOW_PHASE_SWEEP;
      start_sweep_leg(figure, map, flood_field_state);
      return;
    case PLOW_PHASE_SWEEP: {
      int next_row = route->step_coord + route->step_dir;
      int row_min = route->row_along_tx ? route->min_ty : route->min_tx;
      int row_max = route->row_along_tx ? route->max_ty : route->max_tx;
      if (next_row < row_min || next_row > row_max) {
        figure->figure.plowing = false;
        route->phase = PLOW_PHASE_NONE;
        release_field_lock(game_state, figure->id);
        return;
      }
      route->step_coord = next_row;
      route->phase = PLOW_PHASE_STEP;
      start_sweep_leg(figure, map, flood_field_state);
      return;
    }
    case PLOW_PHASE_STEP:
      route->sweep_positive = !route->sweep_positive;
      route->phase = PLOW_PHASE_SWEEP;
      start_sweep_leg(figure, map, flood_field_state);
      return;
  }
}


void harvest_route_field_tile(const HarvestRoute *route, int *out_tx, int *out_ty) {
  *out_tx = route->row_along_tx ? route->cursor : route->row;
  *out_ty = route->row_along_tx ? route->row : route->cursor;
}

void harvest_route_stand_tile(const HarvestRoute *route, int *out_tx, int *out_ty) {
  int stand_row = route->row - route->step_dir;
  *out_tx = route->row_along_tx ? route->cursor : stand_row;
  *out_ty = route->row_along_tx ? stand_row : route->cursor;
}

static void harvest_tile(Map *map, ObjectArray *objects, int tx, int ty) {
  Tile *t = map_tile(map, tx, ty);
  if (!t) return;
  for (int i = 0; i < t->wheat_tuft_count; i++) {
    int idx = object_array_find_by_id(objects, t->wheat_tuft_ids[i]);
    if (idx < 0) continue;
    Object *o = &objects->data[idx];

    WheatStage stage = o->wheat.wheat_stage;
    if (stage != WHEAT_STAGE_RIPE && stage != WHEAT_STAGE_OVERRIPE) continue;

    o->wheat.wheat_stage = WHEAT_STAGE_HARVESTED;
    o->wheat.wheat_stage_age = 0.0f;
    o->wheat.wheat_progress_timer = 0.0f;
  }
}

static bool tile_has_harvestable_tuft(Map *map, const ObjectArray *objects, int tx, int ty) {
  const Tile *t = map_tile(map, tx, ty);
  if (!t) return false;
  for (int i = 0; i < t->wheat_tuft_count; i++) {
    int idx = object_array_find_by_id(objects, t->wheat_tuft_ids[i]);
    if (idx < 0) continue;
    WheatStage stage = objects->data[idx].wheat.wheat_stage;
    if (stage == WHEAT_STAGE_RIPE || stage == WHEAT_STAGE_OVERRIPE) return true;
  }
  return false;
}

static bool find_nearest_sheaf(Map *map, const ObjectArray *objects, int min_tx, int max_tx, int min_ty, int max_ty,
                               int from_tx, int from_ty, int *out_tx, int *out_ty) {
  bool found = false;
  int best_dist2 = 0;
  for (int ty = min_ty; ty <= max_ty; ty++) {
    for (int tx = min_tx; tx <= max_tx; tx++) {
      const Tile *t = map_tile(map, tx, ty);
      if (!t || t->wheat_tuft_count == 0) continue;

      bool tile_has_harvested = false;
      for (int i = 0; i < t->wheat_tuft_count; i++) {
        int idx = object_array_find_by_id(objects, t->wheat_tuft_ids[i]);
        if (idx >= 0 && objects->data[idx].wheat.wheat_stage == WHEAT_STAGE_HARVESTED) {
          tile_has_harvested = true;
          break;
        }
      }
      if (!tile_has_harvested) continue;

      int dx = tx - from_tx;
      int dy = ty - from_ty;
      int dist2 = dx * dx + dy * dy;
      if (!found || dist2 < best_dist2) {
        found = true;
        best_dist2 = dist2;
        *out_tx = tx;
        *out_ty = ty;
      }
    }
  }
  return found;
}

static Mansus *find_own_mansus(GameState *game_state, unsigned int figure_id) {
  for (int i = 0; i < game_state->mansen.count; i++) {
    if (game_state->mansen.data[i].farmer_object_id == figure_id) return &game_state->mansen.data[i];
    if (game_state->mansen.data[i].farmhand_object_id == figure_id) return &game_state->mansen.data[i];
  }
  return NULL;
}

static bool mansus_has_barn(GameState *game_state, unsigned int figure_id) {
  const Mansus *mansus = find_own_mansus(game_state, figure_id);
  return mansus && mansus->barn != 0;
}

// A Mansus can have two fields, so route completions look up the one matching
// their own bounds instead of assuming fields.data[0].
static Field *find_mansus_field_by_bounds(Mansus *mansus, int min_tx, int max_tx, int min_ty, int max_ty) {
  for (int i = 0; i < mansus->fields.count; i++) {
    Field *f = &mansus->fields.data[i];
    if (f->corners_field[0][0] == min_tx && f->corners_field[1][0] == max_tx &&
        f->corners_field[0][1] == min_ty && f->corners_field[1][1] == max_ty) {
      return f;
    }
  }
  return NULL;
}

static int remove_sheaf_at(Map *map, ObjectArray *objects, int tx, int ty) {
  Tile *t = map_tile(map, tx, ty);
  if (!t) return 0;

  int removed_count = 0;
  for (int i = t->wheat_tuft_count - 1; i >= 0; i--) {
    int idx = object_array_find_by_id(objects, t->wheat_tuft_ids[i]);
    if (idx < 0 || objects->data[idx].wheat.wheat_stage != WHEAT_STAGE_HARVESTED) continue;

    object_array_remove_swap(objects, idx);
    t->wheat_tuft_ids[i] = t->wheat_tuft_ids[t->wheat_tuft_count - 1];
    t->wheat_tuft_count--;
    removed_count++;
  }
  return removed_count;
}

static void walk_to_next_mow_tile(Object *figure, Map *map, FloodFieldArray *flood_field_state) {
  HarvestRoute *route = &figure->figure.harvest_route;
  int field_tx, field_ty, stand_tx, stand_ty;
  harvest_route_field_tile(route, &field_tx, &field_ty);
  harvest_route_stand_tile(route, &stand_tx, &stand_ty);

  route->phase = HARVEST_PHASE_WALKING;
  figure->figure.action = FIGURE_ACTION_WALK;
  figure_walk_to_direct(figure, map, flood_field_state, stand_tx, stand_ty);
  figure->figure.gather_tx = field_tx;
  figure->figure.gather_ty = field_ty;
}

static void start_next_sheaf_leg(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                 GameState *game_state) {
  HarvestRoute *route = &figure->figure.harvest_route;
  int sheaf_tx, sheaf_ty;
  if (!find_nearest_sheaf(map, objects, route->min_tx, route->max_tx, route->min_ty, route->max_ty,
                          figure->tx, figure->ty, &sheaf_tx, &sheaf_ty)) {
    if (route->mowing_done) {
      route->phase = HARVEST_PHASE_NONE;
      figure->figure.action = FIGURE_ACTION_STAND;
      release_field_lock(game_state, figure->id);
    } else {
      walk_to_next_mow_tile(figure, map, flood_field_state);
    }
    return;
  }
  route->phase = HARVEST_PHASE_TO_SHEAF;
  figure->figure.action = route->wearing_basket ? FIGURE_ACTION_CARRY_WALK : FIGURE_ACTION_WALK;
  figure_walk_to_direct(figure, map, flood_field_state, sheaf_tx, sheaf_ty);
}

static void start_walk_to_barn(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                               GameState *game_state) {
  HarvestRoute *route = &figure->figure.harvest_route;
  const Mansus *mansus = find_own_mansus(game_state, figure->id);
  int barn_idx = mansus ? object_array_find_by_id(objects, mansus->barn) : -1;
  int approach_tx, approach_ty;
  if (barn_idx < 0 || map_free_tiles_near(map, objects->data[barn_idx].tx, objects->data[barn_idx].ty, 1, &approach_tx, &approach_ty) == 0) {
    route->phase = HARVEST_PHASE_NONE;
    figure->figure.action = FIGURE_ACTION_STAND;
    release_field_lock(game_state, figure->id);
    return;
  }
  route->phase = HARVEST_PHASE_TO_BARN;
  figure->figure.action = FIGURE_ACTION_CARRY_WALK;
  figure_walk_to(figure, map, flood_field_state, approach_tx, approach_ty);
}

static void advance_harvest_route(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                  GameState *game_state) {
  switch (figure->figure.harvest_route.phase) {
    case HARVEST_PHASE_WALKING:
      figure->figure.harvest_route.phase = HARVEST_PHASE_MOWING;
      figure->figure.harvest_route.mow_timer = 0.0f;
      figure->figure.action = FIGURE_ACTION_MOW;
      figure->figure.action_timer = 0.0f;
      return;
    case HARVEST_PHASE_TO_SHEAF:
      figure->figure.harvest_route.phase = HARVEST_PHASE_PICKING;
      figure->figure.harvest_route.pick_timer = 0.0f;
      figure->figure.action = FIGURE_ACTION_CARRY_PICK;
      figure->figure.action_timer = 0.0f;
      return;
    case HARVEST_PHASE_TO_BARN: {
      Mansus *mansus = find_own_mansus(game_state, figure->id);
      if (mansus) {
        mansus->goods.grains += figure->figure.harvest_route.carried_sheaves;
        mansus->goods.straw += figure->figure.harvest_route.carried_sheaves;
      }
      figure->figure.harvest_route.carried_sheaves = 0;
      start_next_sheaf_leg(figure, map, objects, flood_field_state, game_state);
      return;
    }
    default:
      return;
  }
}

static void start_wood_walk_to_barn(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                    GameState *game_state) {
  WoodRoute *route = &figure->figure.wood_route;
  const Mansus *mansus = find_own_mansus(game_state, figure->id);
  int barn_idx = mansus ? object_array_find_by_id(objects, mansus->barn) : -1;
  int approach_tx, approach_ty;
  if (barn_idx < 0 || map_free_tiles_near(map, objects->data[barn_idx].tx, objects->data[barn_idx].ty, 1, &approach_tx, &approach_ty) == 0) {
    route->phase = WOOD_PHASE_NONE;
    figure->figure.action = FIGURE_ACTION_STAND;
    return;
  }
  route->phase = WOOD_PHASE_TO_BARN;
  figure->figure.action = FIGURE_ACTION_CARRY_WALK;
  figure_walk_to(figure, map, flood_field_state, approach_tx, approach_ty);
}

static void advance_wood_route(Object *figure, GameState *game_state) {
  if (figure->figure.wood_route.phase != WOOD_PHASE_TO_BARN) return;

  Mansus *mansus = find_own_mansus(game_state, figure->id);
  if (mansus) mansus->goods.wood += 1;

  figure->figure.wood_route.phase = WOOD_PHASE_NONE;
  figure->figure.action = FIGURE_ACTION_STAND;
}

static bool advance_harvest_cursor(HarvestRoute *route, int *out_tx, int *out_ty) {
  int row_axis_min = route->row_along_tx ? route->min_tx : route->min_ty;
  int row_axis_max = route->row_along_tx ? route->max_tx : route->max_ty;
  int next_cursor = route->cursor + route->sweep_dir;

  if (next_cursor < row_axis_min || next_cursor > row_axis_max) {
    int step_axis_min = route->row_along_tx ? route->min_ty : route->min_tx;
    int step_axis_max = route->row_along_tx ? route->max_ty : route->max_tx;
    int next_row = route->row + route->step_dir;
    if (next_row < step_axis_min || next_row > step_axis_max) return false;
    route->row = next_row;
    route->sweep_dir = -route->sweep_dir;
  } else {
    route->cursor = next_cursor;
  }

  *out_tx = route->row_along_tx ? route->cursor : route->row;
  *out_ty = route->row_along_tx ? route->row : route->cursor;
  return true;
}

static void update_harvest_mowing(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                  GameState *game_state) {
  const float harvest_mow_seconds = 1.0f;
  HarvestRoute *route = &figure->figure.harvest_route;

  int field_tx, field_ty;
  harvest_route_field_tile(route, &field_tx, &field_ty);

  bool just_arrived = route->mow_timer <= 0.0f;
  if (!just_arrived || tile_has_harvestable_tuft(map, objects, field_tx, field_ty)) {
    route->mow_timer += game_delta_time();
    if (route->mow_timer < harvest_mow_seconds) return;
    harvest_tile(map, objects, field_tx, field_ty);
  }

  int row_before = route->row;
  int next_field_tx, next_field_ty;
  bool has_next_tile = advance_harvest_cursor(route, &next_field_tx, &next_field_ty);
  bool row_finished = !has_next_tile || route->row != row_before;

  if (row_finished) {
    route->mowing_done = !has_next_tile;
    route->carried_sheaves = 0;
    route->wearing_basket = false;
    if (mansus_has_barn(game_state, figure->id)) {
      start_next_sheaf_leg(figure, map, objects, flood_field_state, game_state);
    } else {
      route->phase = HARVEST_PHASE_NONE;
      figure->figure.action = FIGURE_ACTION_STAND;
      release_field_lock(game_state, figure->id);
    }
    return;
  }

  walk_to_next_mow_tile(figure, map, flood_field_state);
}

static void update_harvest_picking(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                   GameState *game_state) {
  const float harvest_pick_seconds = 2.0f;
  const int basket_capacity = 20;
  HarvestRoute *route = &figure->figure.harvest_route;
  route->pick_timer += game_delta_time();
  if (route->pick_timer < harvest_pick_seconds) return;

  route->carried_sheaves += remove_sheaf_at(map, objects, figure->tx, figure->ty);
  route->wearing_basket = true;

  int next_tx, next_ty;
  bool more_sheaves = find_nearest_sheaf(map, objects, route->min_tx, route->max_tx, route->min_ty, route->max_ty,
                                        figure->tx, figure->ty, &next_tx, &next_ty);

  if (route->carried_sheaves >= basket_capacity || !more_sheaves) {
    start_walk_to_barn(figure, map, objects, flood_field_state, game_state);
  } else {
    route->phase = HARVEST_PHASE_TO_SHEAF;
    figure->figure.action = FIGURE_ACTION_CARRY_WALK;
    figure_walk_to_direct(figure, map, flood_field_state, next_tx, next_ty);
  }
}

static void update_wood_picking(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                GameState *game_state) {
  const float wood_pick_seconds = 2.0f;
  WoodRoute *route = &figure->figure.wood_route;
  route->pick_timer += game_delta_time();
  if (route->pick_timer < wood_pick_seconds) return;

  int tree_idx = object_array_find_by_id(objects, route->tree_id);
  if (tree_idx >= 0) {
    map_clear_object(map, &objects->data[tree_idx]);
    object_array_remove_swap(objects, tree_idx);
  }
  start_wood_walk_to_barn(figure, map, objects, flood_field_state, game_state);
}

static bool advance_sow_cursor(SowRoute *route, int *out_tx, int *out_ty) {
  int row_axis_min = route->row_along_tx ? route->min_tx : route->min_ty;
  int row_axis_max = route->row_along_tx ? route->max_tx : route->max_ty;
  int next_cursor = route->cursor + route->sweep_dir;
  if (next_cursor < row_axis_min || next_cursor > row_axis_max) {
    int step_axis_min = route->row_along_tx ? route->min_ty : route->min_tx;
    int step_axis_max = route->row_along_tx ? route->max_ty : route->max_tx;
    int next_row = route->row + route->step_dir;
    if (next_row < step_axis_min || next_row > step_axis_max) return false;
    route->row = next_row;
    route->sweep_dir = -route->sweep_dir;
  } else {
    route->cursor = next_cursor;
  }
  *out_tx = route->row_along_tx ? route->cursor : route->row;
  *out_ty = route->row_along_tx ? route->row : route->cursor;
  return true;
}

// Sowing has no per-tile pause: the walk-with-seed-basket animation IS the sowing
// motion, so the figure just keeps walking straight through every tile of the field.
static void advance_sow_route(Object *figure, Map *map, FloodFieldArray *flood_field_state, GameState *game_state) {
  SowRoute *route = &figure->figure.sow_route;
  if (route->phase != SOW_PHASE_WALKING) return;
  Tile *sown_tile = map_tile(map, figure->tx, figure->ty);
  if (sown_tile) sown_tile->fallow_grass_blend = 0.0f;
  int next_tx, next_ty;
  if (!advance_sow_cursor(route, &next_tx, &next_ty)) {
    Mansus *mansus = find_own_mansus(game_state, figure->id);
    Field *sown_field = mansus ? find_mansus_field_by_bounds(mansus, route->min_tx, route->max_tx, route->min_ty, route->max_ty) : NULL;
    if (sown_field) sown_field->field_condition = SOWED;
    release_field_lock(game_state, figure->id);
    route->phase = SOW_PHASE_NONE;
    figure->figure.action = FIGURE_ACTION_STAND;
    return;
  }
  figure->figure.action = FIGURE_ACTION_SOW;
  figure_walk_to_direct(figure, map, flood_field_state, next_tx, next_ty);
}

static void dig_route_field_tile(const DigRoute *route, int *out_tx, int *out_ty) {
  *out_tx = route->row_along_tx ? route->cursor : route->row;
  *out_ty = route->row_along_tx ? route->row : route->cursor;
}

static bool advance_dig_cursor(DigRoute *route, int *out_tx, int *out_ty) {
  int row_axis_min = route->row_along_tx ? route->min_tx : route->min_ty;
  int row_axis_max = route->row_along_tx ? route->max_tx : route->max_ty;
  int next_cursor = route->cursor + route->sweep_dir;

  if (next_cursor < row_axis_min || next_cursor > row_axis_max) {
    int step_axis_min = route->row_along_tx ? route->min_ty : route->min_tx;
    int step_axis_max = route->row_along_tx ? route->max_ty : route->max_tx;
    int next_row = route->row + route->step_dir;
    if (next_row < step_axis_min || next_row > step_axis_max) return false;
    route->row = next_row;
    route->sweep_dir = -route->sweep_dir;
  } else {
    route->cursor = next_cursor;
  }

  *out_tx = route->row_along_tx ? route->cursor : route->row;
  *out_ty = route->row_along_tx ? route->row : route->cursor;
  return true;
}

static void advance_dig_route(Object *figure) {
  if (figure->figure.dig_route.phase != DIG_PHASE_WALKING) return;
  figure->figure.dig_route.phase = DIG_PHASE_DIGGING;
  figure->figure.dig_route.dig_timer = 0.0f;
  figure->figure.action = FIGURE_ACTION_DIG;
  figure->figure.action_timer = 0.0f;
}

// Digging clears whatever was growing wild (or died off unharvested) on the
// tile before it becomes arable soil again.
static void clear_field_tile_decor(ObjectArray *objects, Tile *tile, int tx, int ty) {
  for (int i = objects->count - 1; i >= 0; i--) {
    Object *o = &objects->data[i];
    if (o->tx != tx || o->ty != ty) continue;
    if (o->kind == OBJECT_GRASS_TUFT || o->kind == OBJECT_PUDDLE || o->kind == OBJECT_WHEAT_TUFT) {
      object_array_remove_swap(objects, i);
    }
  }
  tile->has_puddle = false;
  tile->wheat_tuft_count = 0;
}

static void update_dig_digging(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                               GameState *game_state) {
  const float dig_seconds = 1.0f;
  DigRoute *route = &figure->figure.dig_route;

  int field_tx, field_ty;
  dig_route_field_tile(route, &field_tx, &field_ty);
  Tile *dug_tile = map_tile(map, field_tx, field_ty);

  bool just_arrived = route->dig_timer <= 0.0f;
  if (just_arrived && dug_tile) {
    // Turn to soil right away but start it fully grass-tinted, then fade that
    // tint out over the dig, instead of an instant grass-to-dirt cut.
    dug_tile->type = TILE_SOIL;
    dug_tile->fallow_grass_blend = 1.0f;
    clear_field_tile_decor(objects, dug_tile, field_tx, field_ty);
  }

  route->dig_timer += game_delta_time();
  if (dug_tile) {
    float progress = route->dig_timer / dig_seconds;
    if (progress > 1.0f) progress = 1.0f;
    dug_tile->fallow_grass_blend = 1.0f - progress;
  }
  if (route->dig_timer < dig_seconds) return;

  int next_tx, next_ty;
  if (!advance_dig_cursor(route, &next_tx, &next_ty)) {
    Mansus *mansus = find_own_mansus(game_state, figure->id);
    Field *dug_field = mansus ? find_mansus_field_by_bounds(mansus, route->min_tx, route->max_tx, route->min_ty, route->max_ty) : NULL;
    if (dug_field) dug_field->field_condition = PLOWED;
    release_field_lock(game_state, figure->id);
    route->phase = DIG_PHASE_NONE;
    figure->figure.action = FIGURE_ACTION_STAND;
    return;
  }

  route->phase = DIG_PHASE_WALKING;
  figure->figure.action = FIGURE_ACTION_WALK;
  figure_walk_to_direct(figure, map, flood_field_state, next_tx, next_ty);
}

static void stop_walking(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state, int flood_field_index,
                         const Texture_State *texture_state, GameState *game_state) {
  flood_field_array_release(flood_field_state, flood_field_index);
  figure->figure.flood_field_idx = -1;
  figure->figure.action = figure->figure.pending_action;
  figure->figure.pending_action = FIGURE_ACTION_STAND;
  figure->figure.progress = 0.0f;
  figure->figure.prev_tile = -1;
  figure->figure.best_distance_to_target = -1;
  figure->figure.pacing_streak = 0;
  if (figure->figure.species == FIGURE_SPECIES_OX) {
    advance_plow_route(figure, map, flood_field_state, game_state);
  }
  advance_harvest_route(figure, map, objects, flood_field_state, game_state);
  advance_sow_route(figure, map, flood_field_state, game_state);
  advance_dig_route(figure);
  advance_wood_route(figure, game_state);
  if (!figure->figure.direct_walking && figure->figure.flood_field_idx < 0) {
    apply_stationary_sprite(figure, texture_state);
  }
}

static bool direct_step_toward(Map *map, int from_x, int from_y, int target_x, int target_y, int *out_step_x, int *out_step_y) {
  int step_x = from_x + (target_x > from_x ? 1 : (target_x < from_x ? -1 : 0));
  int step_y = from_y + (target_y > from_y ? 1 : (target_y < from_y ? -1 : 0));
  const Tile *tile = map_tile(map, step_x, step_y);
  if (!tile || !map_tile_walkable(tile)) return false;
  *out_step_x = step_x;
  *out_step_y = step_y;
  return true;
}

static int best_local_step_toward(Map *map, int current_tile, int target_tile) {
  int x, y, target_x, target_y;
  node_to_xy(current_tile, map->w, &x, &y);
  node_to_xy(target_tile, map->w, &target_x, &target_y);

  const int dx8[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
  const int dy8[8] = {0, 0, -1, 1, -1, 1, -1, 1};

  int best = -1;
  int best_distance = (x - target_x) * (x - target_x) + (y - target_y) * (y - target_y);
  for (int k = 0; k < 8; k++) {
    int nx = x + dx8[k];
    int ny = y + dy8[k];
    const Tile *t = map_tile(map, nx, ny);
    if (!t || !map_tile_walkable(t)) continue;

    bool diagonal = dx8[k] != 0 && dy8[k] != 0;
    if (diagonal) {
      const Tile *flank_a = map_tile(map, x, ny);
      const Tile *flank_b = map_tile(map, nx, y);
      if (!flank_a || !flank_b || !map_tile_walkable(flank_a) || !map_tile_walkable(flank_b)) continue;
    }

    int distance = (nx - target_x) * (nx - target_x) + (ny - target_y) * (ny - target_y);
    if (distance < best_distance) {
      best_distance = distance;
      best = node_index(nx, ny, map->w);
    }
  }
  return best;
}

static int step_toward_own_target_directly(Map *map, int current_tile, int target_tile) {
  int current_x, current_y, target_x, target_y;
  node_to_xy(current_tile, map->w, &current_x, &current_y);
  node_to_xy(target_tile, map->w, &target_x, &target_y);

  int step_x, step_y;
  if (direct_step_toward(map, current_x, current_y, target_x, target_y, &step_x, &step_y)) {
    return node_index(step_x, step_y, map->w);
  }

  int fallback = best_local_step_toward(map, current_tile, target_tile);
  return fallback < 0 ? current_tile : fallback;
}

static int choose_next_tile(Map *map, FloodField *field, int current_tile, int target_tile, bool allow_diagonal, int previous_tile) {
  if (current_tile == target_tile) return current_tile;
  if (field->cost[current_tile] == 0) {
    return step_toward_own_target_directly(map, current_tile, target_tile);
  }
  int best = flood_field_best_neighbor(map, field, current_tile, target_tile, allow_diagonal, previous_tile);
  if (best == -1 && previous_tile != -1) {
    best = flood_field_best_neighbor(map, field, current_tile, target_tile, allow_diagonal, -1);
  }
  return best == -1 ? current_tile : best;
}

static const int PACING_ESCAPE_STREAK = 5;

static bool figure_is_pacing(Object *figure) {
  int delta_x = figure->tx - figure->figure.target_tx;
  int delta_y = figure->ty - figure->figure.target_ty;
  int distance_to_target = delta_x * delta_x + delta_y * delta_y;

  if (figure->figure.best_distance_to_target < 0 ||
      distance_to_target < figure->figure.best_distance_to_target ||
      distance_to_target > 100
  ) {
    figure->figure.best_distance_to_target = distance_to_target;
    figure->figure.pacing_streak = 0;
  } else {
    figure->figure.pacing_streak++;
  }
  return figure->figure.pacing_streak >= PACING_ESCAPE_STREAK;
}

static int choose_and_reserve_next_tile(Object *figure, Map *map, FloodField *field, int current_tile, int target_tile) {
  const int severe_stuck_streak = 200;
  int next_tile = -1;
  bool pacing = figure_is_pacing(figure);
  if (pacing) {
    int escape_tile = step_toward_own_target_directly(map, current_tile, target_tile);
    if (escape_tile != current_tile) next_tile = escape_tile;
  }
  if (next_tile < 0 && figure->figure.pacing_streak > severe_stuck_streak) {
    int best = flood_field_best_neighbor_ignoring_density(map, field, current_tile, target_tile, true, figure->figure.prev_tile);
    if (best == -1 && figure->figure.prev_tile != -1) {
      best = flood_field_best_neighbor_ignoring_density(map, field, current_tile, target_tile, true, -1);
    }
    if (best != -1) next_tile = best;
  }
  if (next_tile < 0) {
    next_tile = choose_next_tile(map, field, current_tile, target_tile, true, figure->figure.prev_tile);
  }
  if (next_tile == current_tile) return -1;
  node_to_xy(next_tile, map->w, &figure->figure.step_tx, &figure->figure.step_ty);
  Tile *step_tile = map_tile(map, figure->figure.step_tx, figure->figure.step_ty);
  if (step_tile) step_tile->figure_occupied = 1;
  return next_tile;
}

static void update_flow_field_figure(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                     const Texture_State *texture_state, GameState *game_state) {
  const float figure_sow_speed_factor = 1.0f;
  int flood_field_index = figure->figure.flood_field_idx;
  FloodField *field = &flood_field_state->data[flood_field_index];
  int current_tile = node_index(figure->tx, figure->ty, map->w);
  int target_tile = node_index(figure->figure.target_tx, figure->figure.target_ty, map->w);

  if (current_tile == target_tile) {
    stop_walking(figure, map, objects, flood_field_state, flood_field_index, texture_state, game_state);
    return;
  }

  int next_tile;
  if (figure->figure.progress <= 0.0f) {
    next_tile = choose_and_reserve_next_tile(figure, map, field, current_tile, target_tile);
    if (next_tile < 0) return;
  } else {
    next_tile = node_index(figure->figure.step_tx, figure->figure.step_ty, map->w);
  }

  if (figure->figure.action != FIGURE_ACTION_SOW && figure->figure.action != FIGURE_ACTION_CARRY_WALK) {
    figure->figure.action = FIGURE_ACTION_WALK;
  }
  figure->figure.action_timer = 0.0f;
  float speed = figure->figure.speed;
  if (figure->figure.action == FIGURE_ACTION_SOW) speed *= figure_sow_speed_factor;

  int segment_start_x, segment_start_y, segment_end_x, segment_end_y;
  bool arrived = advance_along_segment(figure, map, current_tile, next_tile, speed,
                                       &segment_start_x, &segment_start_y, &segment_end_x, &segment_end_y);
  if (arrived && node_index(figure->tx, figure->ty, map->w) == target_tile) {
    stop_walking(figure, map, objects, flood_field_state, flood_field_index, texture_state, game_state);
    return;
  }

  apply_walk_sprite(figure, texture_state, segment_start_x, segment_start_y, segment_end_x, segment_end_y);
}

static void stop_direct_walk(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                             const Texture_State *texture_state, GameState *game_state) {
  figure->figure.direct_walking = false;
  figure->figure.action = figure->figure.pending_action;
  figure->figure.pending_action = FIGURE_ACTION_STAND;
  figure->figure.progress = 0.0f;
  figure->figure.prev_tile = -1;
  figure->figure.best_distance_to_target = -1;
  figure->figure.pacing_streak = 0;
  if (figure->figure.species == FIGURE_SPECIES_OX) {
    advance_plow_route(figure, map, flood_field_state, game_state);
  }
  advance_harvest_route(figure, map, objects, flood_field_state, game_state);
  advance_sow_route(figure, map, flood_field_state, game_state);
  advance_dig_route(figure);
  if (!figure->figure.direct_walking && figure->figure.flood_field_idx < 0) {
    apply_stationary_sprite(figure, texture_state);
  }
}

static void update_direct_walk_figure(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state,
                                      const Texture_State *texture_state, GameState *game_state) {
  int current_tile = node_index(figure->tx, figure->ty, map->w);
  int target_tile = node_index(figure->figure.target_tx, figure->figure.target_ty, map->w);

  if (current_tile == target_tile) {
    stop_direct_walk(figure, map, objects, flood_field_state, texture_state, game_state);
    return;
  }

  int next_tile;
  if (figure->figure.progress <= 0.0f) {
    next_tile = step_toward_own_target_directly(map, current_tile, target_tile);
    if (next_tile == current_tile) {
      figure_walk_to(figure, map, flood_field_state, figure->figure.target_tx, figure->figure.target_ty);
      return;
    }
    node_to_xy(next_tile, map->w, &figure->figure.step_tx, &figure->figure.step_ty);
    Tile *step_tile = map_tile(map, figure->figure.step_tx, figure->figure.step_ty);
    if (step_tile) step_tile->figure_occupied = 1;
  } else {
    next_tile = node_index(figure->figure.step_tx, figure->figure.step_ty, map->w);
  }

  if (figure->figure.action != FIGURE_ACTION_SOW && figure->figure.action != FIGURE_ACTION_CARRY_WALK) {
    figure->figure.action = FIGURE_ACTION_WALK;
  }
  figure->figure.action_timer = 0.0f;

  int segment_start_x, segment_start_y, segment_end_x, segment_end_y;
  bool arrived = advance_along_segment(figure, map, current_tile, next_tile, figure->figure.speed,
                                       &segment_start_x, &segment_start_y, &segment_end_x, &segment_end_y);
  if (arrived && node_index(figure->tx, figure->ty, map->w) == target_tile) {
    stop_direct_walk(figure, map, objects, flood_field_state, texture_state, game_state);
    return;
  }

  apply_walk_sprite(figure, texture_state, segment_start_x, segment_start_y, segment_end_x, segment_end_y);
}

static void update_tree_chopping(Object *figure, ObjectArray *objects) {
  const float TREE_CHOP_SECONDS_TO_FELL = 30.0f;
  const float TREE_CHOP_SECONDS_TO_BEAM = 30.0f;
  if (figure->figure.action != FIGURE_ACTION_CHOP) return;

  int tree_idx = find_object_at_tile(objects, figure->figure.gather_tx, figure->figure.gather_ty);
  if (tree_idx < 0 || objects->data[tree_idx].kind != OBJECT_TREE) return;
  Object *tree = &objects->data[tree_idx];

  if (tree->tree.state == TREE_STATE_BEAM) {
    figure->figure.action = FIGURE_ACTION_STAND;
    return;
  }

  tree->tree.chop_seconds += game_delta_time();

  if (tree->tree.state == TREE_STATE_STANDING) {
    if (tree->tree.chop_seconds < TREE_CHOP_SECONDS_TO_FELL) return;
    tree->tree.state = TREE_STATE_FELLED;
    tree->facing = (FigureDirection)GetRandomValue(0, FIGURE_DIR_COUNT - 1);
    tree->tree.chop_seconds = 0.0f;
    return;
  }

  if (tree->tree.chop_seconds < TREE_CHOP_SECONDS_TO_BEAM) return;
  tree->tree.state = TREE_STATE_BEAM;

  figure->figure.wood_route.phase = WOOD_PHASE_PICKING;
  figure->figure.wood_route.pick_timer = 0.0f;
  figure->figure.wood_route.tree_id = tree->id;
  figure->figure.action = FIGURE_ACTION_CARRY_PICK;
  figure->figure.action_timer = 0.0f;
}

void update_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, FloodFieldArray *flood_field_state,
                    GameState *game_state) {
  for (int i = 0; i < objects->count; i++) {
    Object *figure = &objects->data[i];
    if (figure->kind != OBJECT_FIGURE) continue;

    if (figure->figure.direct_walking) {
      update_direct_walk_figure(figure, map, objects, flood_field_state, texture_state, game_state);
      continue;
    }

    bool is_walking = figure->figure.flood_field_idx >= 0;
    if (!is_walking) {
      if (figure->figure.harvest_route.phase == HARVEST_PHASE_MOWING) {
        update_harvest_mowing(figure, map, objects, flood_field_state, game_state);
      } else if (figure->figure.harvest_route.phase == HARVEST_PHASE_PICKING) {
        update_harvest_picking(figure, map, objects, flood_field_state, game_state);
      } else if (figure->figure.dig_route.phase == DIG_PHASE_DIGGING) {
        update_dig_digging(figure, map, objects, flood_field_state, game_state);
      } else if (figure->figure.wood_route.phase == WOOD_PHASE_PICKING) {
        update_wood_picking(figure, map, objects, flood_field_state, game_state);
      }
      update_tree_chopping(figure, objects);
      if (figure->figure.flood_field_idx < 0 && !figure->figure.direct_walking) {
        handle_idle(figure, texture_state);
      }
      continue;
    }

    update_flow_field_figure(figure, map, objects, flood_field_state, texture_state, game_state);
  }
}
