#include "containers/arrays.h"
#include "flood_fill/flood_fill.h"
#include "drawing/update_figure.h"
#include "textures/textures.h"
#include "utils/game_time.h"
#include <math.h>

static const float FIGURE_ACTION_FPS = 8.0f;
static const float FIGURE_SOW_SPEED_FACTOR = 1.0f;

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
  if (figure->figure.action == FIGURE_ACTION_CHOP) {
    FigureDirection direction = detect_figure_direction(figure->tx, gather_x, figure->ty, gather_y);
    if (direction != FIGURE_DIR_COUNT) figure->facing = direction;
  }
  const float lean = 0.3f;
  figure->draw_x = (float)figure->tx + (delta_x / distance) * lean;
  figure->draw_y = (float)figure->ty + (delta_y / distance) * lean;
}

static void apply_stationary_sprite(Object *figure, const Texture_State *texture_state) {
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
    frame = (int)(figure->figure.action_timer * FIGURE_ACTION_FPS) % frame_count;
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
  }
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

static void start_sweep_leg(Object *figure, Map *map, FloodFieldArray *flood_field_state) {
  PlowRoute *route = &figure->figure.plow_route;
  if (route->row_along_tx) {
    figure_walk_to(figure, map, flood_field_state, route->sweep_positive ? route->max_tx : route->min_tx, route->step_coord);
  } else {
    figure_walk_to(figure, map, flood_field_state, route->step_coord, route->sweep_positive ? route->max_ty : route->min_ty);
  }
}

static void advance_plow_route(Object *figure, Map *map, FloodFieldArray *flood_field_state) {
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


static void advance_harvest_route(Object *figure) {
  if (figure->figure.harvest_route.phase != HARVEST_PHASE_WALKING) return;
  figure->figure.harvest_route.phase = HARVEST_PHASE_MOWING;
  figure->figure.harvest_route.mow_timer = 0.0f;
  figure->figure.action = FIGURE_ACTION_MOW;
  figure->figure.action_timer = 0.0f;
}

static void harvest_tile(ObjectArray *objects, int tx, int ty) {
  for (int i = 0; i < objects->count; i++) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT || o->tx != tx || o->ty != ty) continue;

    WheatStage stage = o->wheat.wheat_stage;
    if (stage == WHEAT_STAGE_HARVESTED || stage == WHEAT_STAGE_DESTROYED) continue;

    bool harvestable = stage == WHEAT_STAGE_RIPE || stage == WHEAT_STAGE_OVERRIPE;
    o->wheat.wheat_stage = harvestable ? WHEAT_STAGE_HARVESTED : WHEAT_STAGE_DESTROYED;
    o->wheat.wheat_stage_age = 0.0f;
    o->wheat.wheat_progress_timer = 0.0f;
  }
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

static void update_harvest_mowing(Object *figure, Map *map, ObjectArray *objects, FloodFieldArray *flood_field_state) {
  const float harvest_mow_seconds = 5.0f;
  figure->figure.harvest_route.mow_timer += game_delta_time();
  if (figure->figure.harvest_route.mow_timer < harvest_mow_seconds) return;

  harvest_tile(objects, figure->tx, figure->ty);

  int next_tx, next_ty;
  if (!advance_harvest_cursor(&figure->figure.harvest_route, &next_tx, &next_ty)) {
    figure->figure.harvest_route.phase = HARVEST_PHASE_NONE;
    figure->figure.action = FIGURE_ACTION_STAND;
    return;
  }

  figure->figure.harvest_route.phase = HARVEST_PHASE_WALKING;
  figure_walk_to(figure, map, flood_field_state, next_tx, next_ty);
}

static void stop_walking(Object *figure, Map *map, FloodFieldArray *flood_field_state, int flood_field_index, const Texture_State *texture_state) {
  flood_field_array_release(flood_field_state, flood_field_index);
  figure->figure.flood_field_idx = -1;
  figure->figure.action = figure->figure.pending_action;
  figure->figure.pending_action = FIGURE_ACTION_STAND;
  figure->figure.progress = 0.0f;
  figure->figure.prev_tile = -1;
  figure->figure.best_distance_to_target = -1;
  figure->figure.pacing_streak = 0;
  if (figure->figure.species == FIGURE_SPECIES_OX) {
    advance_plow_route(figure, map, flood_field_state);
  }
  advance_harvest_route(figure);
  if (figure->figure.flood_field_idx < 0) {
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

static const int SEVERE_STUCK_STREAK = 200;

static int choose_and_reserve_next_tile(Object *figure, Map *map, FloodField *field, int current_tile, int target_tile) {
  int next_tile = -1;
  bool pacing = figure_is_pacing(figure);

  if (pacing) {
    int escape_tile = step_toward_own_target_directly(map, current_tile, target_tile);
    if (escape_tile != current_tile) next_tile = escape_tile;
  }

  if (next_tile < 0 && figure->figure.pacing_streak > SEVERE_STUCK_STREAK) {
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

static void update_flow_field_figure(Object *figure, Map *map, FloodFieldArray *flood_field_state, const Texture_State *texture_state) {
  int flood_field_index = figure->figure.flood_field_idx;
  FloodField *field = &flood_field_state->data[flood_field_index];
  int current_tile = node_index(figure->tx, figure->ty, map->w);
  int target_tile = node_index(figure->figure.target_tx, figure->figure.target_ty, map->w);

  if (current_tile == target_tile) {
    stop_walking(figure, map, flood_field_state, flood_field_index, texture_state);
    return;
  }

  int next_tile;
  if (figure->figure.progress <= 0.0f) {
    next_tile = choose_and_reserve_next_tile(figure, map, field, current_tile, target_tile);
    if (next_tile < 0) return;
  } else {
    next_tile = node_index(figure->figure.step_tx, figure->figure.step_ty, map->w);
  }

  if (figure->figure.action != FIGURE_ACTION_SOW) {
    figure->figure.action = FIGURE_ACTION_WALK;
  }
  figure->figure.action_timer = 0.0f;
  float speed = figure->figure.speed;
  if (figure->figure.action == FIGURE_ACTION_SOW) speed *= FIGURE_SOW_SPEED_FACTOR;

  int segment_start_x, segment_start_y, segment_end_x, segment_end_y;
  bool arrived = advance_along_segment(figure, map, current_tile, next_tile, speed,
                                       &segment_start_x, &segment_start_y, &segment_end_x, &segment_end_y);
  if (arrived && node_index(figure->tx, figure->ty, map->w) == target_tile) {
    stop_walking(figure, map, flood_field_state, flood_field_index, texture_state);
    return;
  }

  apply_walk_sprite(figure, texture_state, segment_start_x, segment_start_y, segment_end_x, segment_end_y);
}

void update_figures(ObjectArray *objects, Map *map, const Texture_State *texture_state, FloodFieldArray *flood_field_state) {
  for (int i = 0; i < objects->count; i++) {
    Object *figure = &objects->data[i];
    if (figure->kind != OBJECT_FIGURE) continue;

    bool is_walking = figure->figure.flood_field_idx >= 0;
    if (!is_walking) {
      if (figure->figure.harvest_route.phase == HARVEST_PHASE_MOWING) {
        update_harvest_mowing(figure, map, objects, flood_field_state);
      }
      if (figure->figure.flood_field_idx < 0) {
        handle_idle(figure, texture_state);
      }
      continue;
    }

    update_flow_field_figure(figure, map, flood_field_state, texture_state);
  }
}
