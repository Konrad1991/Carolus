#include "drawing/drawing.h"
#include "drawing/drawing_helper.h"
#include "selection/selection.h"
#include "textures/textures.h"
#include "soil/soil_overlay.h"
#include "units/units.h"
#include "seasons/seasons.h"
#include <math.h>

// Draw scene
// -----------------------------------------------------
int compare_object_order(const void *a, const void *b) {
  const ObjectOrder *oa = a;
  const ObjectOrder *ob = b;
  // correct order as a is fully behind b
  if (oa->frontmost_point < ob->backmost_point) return -1;
  // sorting required as b is fully in front of a
  if (ob->frontmost_point < oa->backmost_point) return 1;

  // oa->frontmost_point >= ob->backmost_point && ob->frontmost_point >= oa->backmost_point
  // if oa_depth != ob_depth than the objects differ in their object depth
  // object depth --> the depth along the combined x and y axis
  // if a has a larger object depth than b then sorting is required otherwise the order is correct.
  const float oa_depth = oa->frontmost_point + oa->backmost_point;
  const float ob_depth = ob->frontmost_point + ob->backmost_point;
  if (oa_depth != ob_depth) return (oa_depth > ob_depth) ? 1 : -1;

  if (oa->z != ob->z) return oa->z - ob->z;

  // the tests above proved --> a and b have the same combined depth position and the same z

  if (oa->is_puddle != ob->is_puddle) return (int)ob->is_puddle - (int)oa->is_puddle;
  const bool same_tile = oa->tile.x == ob->tile.x && oa->tile.y == ob->tile.y;
  if (same_tile && oa->is_tall_decor != ob->is_tall_decor) return (int)ob->is_tall_decor - (int)oa->is_tall_decor;
  if (oa->is_ground_decor != ob->is_ground_decor) return (int)oa->is_ground_decor - (int)ob->is_ground_decor;
  if (oa->is_figure != ob->is_figure) return (int)oa->is_figure - (int)ob->is_figure;
  return 0;
}

static int bridge_at(const ObjectArray *objects, const int x, const int y) {
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].kind == OBJECT_BRIDGE && objects->data[i].tx == x && objects->data[i].ty == y) {
      return i;
    }
  }
  return -1;
}

static bool tile_is_edge(Map *map, const Tile *t, int x, int y, float water_level_z) {
  if (x == 0 || y == 0 || x == map->w - 1 || y == map->h - 1) return true;
  const Tile *n_right = map_tile(map, x + 1, y);
  const Tile *n_down = map_tile(map, x, y + 1);
  float z_right = n_right ? (n_right->type == TILE_WATER ? water_level_z : n_right->z) : 0;
  float z_down = n_down ? (n_down->type == TILE_WATER ? water_level_z : n_down->z) : 0;
  return (n_right && t->z > z_right) || (n_down && t->z > z_down);
}

static Color darken_color(Color c, float factor) {
  return (Color){
    (unsigned char)(c.r * factor),
    (unsigned char)(c.g * factor),
    (unsigned char)(c.b * factor),
    c.a,
  };
}

static void draw_overlay_edge_walls(TileState tile_state, Map *map, int x, int y,
                                    float effective_z, int sx, int sy,
                                    float water_level_z, Color tint) {
  const Tile *n_right = map_tile(map, x + 1, y);
  const Tile *n_down = map_tile(map, x, y + 1);
  float z_right = n_right ? (n_right->type == TILE_WATER ? water_level_z : n_right->z) : effective_z;
  float z_down = n_down ? (n_down->type == TILE_WATER ? water_level_z : n_down->z) : effective_z;
  Vector2 top = {(float)sx, (float)sy};

  if (n_right && effective_z > z_right) {
    float delta = (effective_z - z_right) * tile_state.TILE_H;
    Vector2 right = {sx + tile_state.TILE_W / 2.0f, sy + tile_state.TILE_H / 2.0f};
    Vector2 right_ext = {right.x, right.y + delta};
    Vector2 top_ext = {top.x, top.y + delta};
    DrawTriangle(top, right, right_ext, tint);
    DrawTriangle(top, right_ext, top_ext, tint);
  }

  if (n_down && effective_z > z_down) {
    float delta = (effective_z - z_down) * tile_state.TILE_H;
    Vector2 left = {sx - tile_state.TILE_W / 2.0f, sy + tile_state.TILE_H / 2.0f};
    Vector2 left_ext = {left.x, left.y + delta};
    Vector2 top_ext = {top.x, top.y + delta};
    DrawTriangle(top, left_ext, left, tint);
    DrawTriangle(top, top_ext, left_ext, tint);
  }
}

void visible_tile_bounds(TileState tile_state, const Map *map,
                         int *min_tx, int *max_tx, int *min_ty, int *max_ty) {
  int screen_w = GetScreenWidth();
  int screen_h = GetScreenHeight();
  int corner_sx[4] = {0, screen_w, 0, screen_w};
  int corner_sy[4] = {0, 0, screen_h, screen_h};

  *min_tx = screen_to_tile_x(tile_state, (float)corner_sx[0], (float)corner_sy[0]);
  *max_tx = *min_tx;
  *min_ty = screen_to_tile_y(tile_state, (float)corner_sx[0], (float)corner_sy[0]);
  *max_ty = *min_ty;
  for (int i = 1; i < 4; i++) {
    int tx = screen_to_tile_x(tile_state, (float)corner_sx[i], (float)corner_sy[i]);
    int ty = screen_to_tile_y(tile_state, (float)corner_sx[i], (float)corner_sy[i]);
    if (tx < *min_tx) *min_tx = tx;
    if (tx > *max_tx) *max_tx = tx;
    if (ty < *min_ty) *min_ty = ty;
    if (ty > *max_ty) *max_ty = ty;
  }

  const int margin = 12;
  *min_tx -= margin; *max_tx += margin;
  *min_ty -= margin; *max_ty += margin;
  if (*min_tx < 0) *min_tx = 0;
  if (*min_ty < 0) *min_ty = 0;
  if (*max_tx >= map->w) *max_tx = map->w - 1;
  if (*max_ty >= map->h) *max_ty = map->h - 1;
}

static Scales scales;
static void update_scales(Scales* scales, const TileState tile_state, const Texture_State* texture_state, SeasonBlend season_blend) {
  for (int i = 0; i < TILE_VARIANT_COUNT; i++) {
    // tiles are sometimes a bit too small and therefore the scale is adapted to cover the entire tile
    scales->grass_flat_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->grass_flat[season_blend.base][i].width,
      (float)tile_state.TILE_H / (float)texture_state->grass_flat[season_blend.base][i].height
    );
    scales->grass_edge_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->grass_edge[season_blend.base][i].width,
      (float)tile_state.TILE_H / (float)texture_state->grass_edge[season_blend.base][i].height
    );
    scales->road_flat_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->road_flat[i].width,
      (float)tile_state.TILE_H / (float)texture_state->road_flat[i].height
    );
    scales->road_high_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->road_high[i].width,
      (float)tile_state.TILE_H / (float)texture_state->road_high[i].height
    );
    scales->water_flat_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->water_flat[i].width,
      (float)tile_state.TILE_H / (float)texture_state->water_flat[i].height
    );
    scales->swamp_flat_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->swamp_flat[i].width,
      (float)tile_state.TILE_H / (float)texture_state->swamp_flat[i].height
    );
    scales->swamp_edge_scale[i] = fmaxf(
      (float)tile_state.TILE_W / (float)texture_state->swamp_edge[i].width,
      (float)tile_state.TILE_H / (float)texture_state->swamp_edge[i].height
    );
  }
  for (int i = 0; i < FIGURE_MAX_FRAMES; i++) {
    scales->water_flow_scale[i] = (float)tile_state.TILE_H / (float)texture_state->water_flow[i].height;
  }
}

// Draw tiles
// ----------------------
static void draw_grass(const Tile* t,
                       TileState tile_state, Texture_State* texture_state,
                       SeasonBlend season_blend, Map* map,
                       const int sx, const int sy, const int x, const int y,
                       float water_level_z) {
  Color GRASS_COL = {78, 120, 44, 255};
  switch (season_blend.base) {
    case WINTER: {
      GRASS_COL = DARKGREEN;
      break;
    }
    case SPRING: {}
    case SUMMER: {}
    case AUTUMN: {}
    case SEASON_COUNT: {}
  }
  draw_diamond(tile_state, sx, sy, GRASS_COL);
  Color fade_tint = (Color){255, 255, 255, (unsigned char)(season_blend.alpha * 255)};

  const int n_variants = 5; // TILE_VARIANT_COUNT;

  if (tile_is_edge(map, t, x, y, water_level_z)) {
    int variant = tile_variant(x, y, n_variants);
    draw_relief_tile(sx, sy, &texture_state->grass_edge[season_blend.base][variant], scales.grass_edge_scale[variant]);
    if (season_blend.alpha > 0.0f) {
      draw_relief_tile_tinted(sx, sy, &texture_state->grass_edge[season_blend.fade_in][variant], scales.grass_edge_scale[variant], fade_tint);
    }
  } else {
    int variant = tile_variant(x, y, n_variants);
    draw_ground_tile(tile_state, sx, sy, &texture_state->grass_flat[season_blend.base][variant], scales.grass_flat_scale[variant]);
    if (season_blend.alpha > 0.0f) {
      draw_ground_tile_tinted(tile_state, sx, sy, &texture_state->grass_flat[season_blend.fade_in][variant], scales.grass_flat_scale[variant], fade_tint);
    }
  }
}

static void draw_road(const Tile* t,
                      TileState tile_state, Texture_State* texture_state, Map* map,
                      const ObjectArray *objects, const int highlight_index,
                      const int sx, const int sy, const int x, const int y,
                      float water_level_z) {
  const Color ROAD_COL = {130, 105, 75, 255};
  const Color WATER_COL = {30, 100, 200, 255};
  int bridge_idx = bridge_at(objects, x, y);
  if (bridge_idx >= 0) {
    int sy_water = (x + y) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - water_level_z * tile_state.TILE_H;
    draw_diamond(tile_state, sx, sy_water, WATER_COL);
    int variant = tile_variant(x, y, TILE_VARIANT_COUNT);
    draw_ground_tile(tile_state, sx, sy_water, &texture_state->water_flat[variant], scales.water_flat_scale[variant]);
    Color bridge_tint = (bridge_idx == highlight_index) ? RED : WHITE;
    draw_object(tile_state, &objects->data[bridge_idx], bridge_tint);
  } else {
    draw_diamond(tile_state, sx, sy, ROAD_COL);
    int variant = tile_variant(x, y, TILE_VARIANT_COUNT);
    if (tile_is_edge(map, t, x, y, water_level_z)) {
      draw_relief_tile(sx, sy, &texture_state->road_high[variant], scales.road_high_scale[variant]);
    } else {
      draw_ground_tile(tile_state, sx, sy, &texture_state->road_flat[variant], scales.road_flat_scale[variant]);
      draw_diamond_outline(tile_state, sx, sy, BLACK);
    }
  }
}

static void draw_water_flow(TileState tile_state, Texture_State* texture_state,
                            const int sx, const int sy, const int x, const int y) {
  const float water_flow_fps = 4.0f;
  if (tile_variant(x, y, 10) >= 8) return;
  float phase = (float)tile_variant(x, y, 10000) / 10000.0f * FIGURE_MAX_FRAMES;
  int frame = ((int)(GetTime() * water_flow_fps + phase)) % FIGURE_MAX_FRAMES;
  Texture2D *tex = &texture_state->water_flow[frame];
  float scale = scales.water_flow_scale[frame] * 1.1f;
  float w = tex->width * scale;
  float h = tex->height * scale;
  Vector2 center = {(float)sx, sy + tile_state.TILE_H / 2.0f};
  Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
  Rectangle dst = {center.x, center.y, w, h};
  Vector2 origin = {w / 2.0f, h / 2.0f};
  DrawTexturePro(*tex, src, dst, origin, 45.0f, WHITE);
}

static void draw_water(TileState tile_state, Texture_State* texture_state,
                       const int sx, const int sy, const int x, const int y) {
  int variant = tile_variant(x, y, 1);
  draw_ground_tile(tile_state, sx, sy, &texture_state->water_flat[variant], scales.water_flat_scale[variant]);
  draw_water_flow(tile_state, texture_state, sx, sy, x, y);
}

static void draw_mansus_tiles(const Tile* t,
                              TileState tile_state, Texture_State* texture_state,
                              SeasonBlend season_blend, Map* map,
                              const int sx, const int sy, const int x, const int y,
                              float water_level_z) {
  draw_grass(t, tile_state, texture_state, season_blend, map, sx, sy, x, y, water_level_z);
}

static void draw_dirt(const Tile* t, TileState tile_state, Texture_State* texture_state,
                      const int sx, const int sy, const int x, const int y) {
  const Color SOIL_COL = {110, 82, 48, 255};
  draw_diamond(tile_state, sx, sy, SOIL_COL);
  float scale = (float)tile_state.TILE_W / (float)texture_state->arable_land[0].width;
  draw_ground_tile(tile_state, sx, sy, &texture_state->arable_land[0], scale);

  // A fallow field cross-fades toward a grass look the same way seasons cross-fade.
  // Always fades toward SUMMER grass specifically (not the current season) so
  // overgrowth always reads as "green and wild" instead of e.g. winter's pale look.
  if (t->fallow_grass_blend > 0.0f) {
    const int n_variants = 5;
    int variant = tile_variant(x, y, n_variants);
    const Texture2D *overgrown_tex = &texture_state->grass_flat[SUMMER][variant];
    float grass_scale = (float)tile_state.TILE_W / (float)overgrown_tex->width;
    Color grass_tint = (Color){255, 255, 255, (unsigned char)(t->fallow_grass_blend * 255)};
    draw_ground_tile_tinted(tile_state, sx, sy, overgrown_tex, grass_scale, grass_tint);
  }

  draw_diamond_outline(tile_state, sx, sy, BLACK);
}

static void draw_swamp(const Tile* t, TileState tile_state, Texture_State* texture_state, Map* map,
                       const int sx, const int sy, const int x, const int y, float water_level_z) {
  const Color SWAMP_COL = {66, 74, 48, 255};
  draw_diamond(tile_state, sx, sy, SWAMP_COL);
  int variant = tile_variant(x, y, TILE_VARIANT_COUNT);
  if (tile_is_edge(map, t, x, y, water_level_z)) {
    draw_relief_tile(sx, sy, &texture_state->swamp_edge[variant], scales.swamp_edge_scale[variant]);
  } else {
    draw_ground_tile(tile_state, sx, sy, &texture_state->swamp_flat[variant], scales.swamp_flat_scale[variant]);
    draw_diamond_outline(tile_state, sx, sy, BLACK);
  }
}

static void draw_tiles(TileState tile_state, Texture_State* texture_state,
                       SeasonBlend season_blend, Map* map,
                       const ObjectArray *objects, const int highlight_index,
                       int min_tx, int max_tx, int min_ty, int max_ty,
                       float water_level_z, SoilOverlayState soil_overlay_state) {
  for (int y = min_ty; y <= max_ty; y++) {
    for (int x = min_tx; x <= max_tx; x++) {

      const Tile *t = &map->tiles[y * map->w + x];
      float tile_z = (t->type == TILE_WATER) ? water_level_z : t->z;
      int sx = (x - y) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
      int sy = (x + y) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - tile_z * tile_state.TILE_H;

      if (soil_overlay_state.current != SOIL_OVERLAY_NONE) {
        Color tint = soil_overlay_tint(soil_overlay_state, t);
        draw_overlay_edge_walls(tile_state, map, x, y, tile_z, sx, sy, water_level_z, darken_color(tint, 0.65f));
        draw_diamond(tile_state, sx, sy, tint);
        draw_diamond_outline(tile_state, sx, sy, (Color){0, 0, 0, 60});
        continue;
      }

      switch (t->type) {
        case TILE_GRASS: {
          draw_grass(t, tile_state, texture_state, season_blend, map, sx, sy, x, y, water_level_z);
          break;
        }
        case TILE_ROAD: {
          draw_road(t, tile_state, texture_state, map, objects, highlight_index, sx, sy, x, y, water_level_z);
          break;
        }
        case TILE_WATER: {
          draw_water(tile_state, texture_state, sx, sy, x, y);
          break;
        }
        case TILE_MANSUSYARD: {
          draw_mansus_tiles(t, tile_state, texture_state, season_blend, map, sx, sy, x, y, water_level_z);
          break;
        }
        case TILE_SOIL: {
          draw_dirt(t, tile_state, texture_state, sx, sy, x, y);
          break;
        }
        case TILE_SWAMP: {
          draw_swamp(t, tile_state, texture_state, map, sx, sy, x, y, water_level_z);
          break;
        }
      }
    }
  }
}

static bool object_in_bounds(const Object *o, int min_tx, int max_tx, int min_ty, int max_ty) {
  bool use_draw_pos = (o->kind == OBJECT_FIGURE && o->id != 0) || o->kind == OBJECT_WHEAT_TUFT;
  float fx = use_draw_pos ? o->draw.x : (float)o->tx;
  float fy = use_draw_pos ? o->draw.y : (float)o->ty;
  int front_x = (int)floorf(fx);
  int front_y = (int)floorf(fy);
  int back_x = front_x - o->footprint_w + 1;
  int back_y = front_y - o->footprint_h + 1;
  return back_x <= max_tx && front_x >= min_tx && back_y <= max_ty && front_y >= min_ty;
}

// A felled trunk's sprite reaches out from its tile in the direction it fell,
// past a 1x1 footprint - the sort position needs to follow that reach, or a
// figure standing where the trunk visually extends to sorts as if it were
// behind the stump instead of behind the part of the trunk actually drawn there.
// Trunk art spans 3 tiles total (base + 2), confirmed visually 2026-08-02.
static const float TREE_TRUNK_SORT_REACH = 2.0f;

static ObjectOrder create_ObjectOrder_from_object(const Object *o, int idx) {
  bool use_draw_pos = (o->kind == OBJECT_FIGURE && o->id != 0) || o->kind == OBJECT_WHEAT_TUFT;
  float x = use_draw_pos ? o->draw.x : (float)o->tx;
  float y = use_draw_pos ? o->draw.y : (float)o->ty;
  float base_point = x + y;
  float frontmost_point = base_point;
  // A felled trunk spans from its base tile to the tip in the fall direction,
  // not a single point - give it real spread in the sort instead of shifting
  // the whole object to the tip, or figures near the stump end sort as if the
  // entire trunk (reported far ahead of them) were in front of them too.
  if (o->kind == OBJECT_TREE && o->tree.state != TREE_STATE_STANDING) {
    frontmost_point += ((float)TREE_TRUNK_DIR_TX[o->facing] + (float)TREE_TRUNK_DIR_TY[o->facing]) * TREE_TRUNK_SORT_REACH;
  }
  return (ObjectOrder){
    .idx = idx,
    .frontmost_point = frontmost_point,
    .backmost_point = base_point - (o->footprint_w - 1) - (o->footprint_h - 1),
    .z = o->z,
    .tile = {o->tx, o->ty},
    .sprite_slice_from = 0.0f,
    .sprite_slice_to = 1.0f,
    .is_figure = o->kind == OBJECT_FIGURE,
    .is_ground_decor = o->kind == OBJECT_GRASS_TUFT,
    .is_tall_decor = o->kind == OBJECT_WHEAT_TUFT,
    .is_puddle = o->kind == OBJECT_PUDDLE,
  };
}

// A felled trunk's sprite spans TREE_TRUNK_TILE_SPAN tiles but is one texture -
// sliced left-to-right into one segment per tile so each segment can be sorted
// at its own tile's depth instead of the whole sprite sorting as one object
// (which gets the depth order wrong wherever a neighboring figure's sprite is
// tall enough to visually overlap into a tile the trunk merely reaches into).
static ObjectOrder create_tree_segment_order(const Object *o, int idx, int seg, float slice_from, float slice_to) {
  int seg_tx = o->tx + TREE_TRUNK_DIR_TX[o->facing] * seg;
  int seg_ty = o->ty + TREE_TRUNK_DIR_TY[o->facing] * seg;
  float depth = (float)(seg_tx + seg_ty);
  return (ObjectOrder){
    .idx = idx,
    .frontmost_point = depth,
    .backmost_point = depth,
    .z = o->z,
    .tile = {seg_tx, seg_ty},
    .sprite_slice_from = slice_from,
    .sprite_slice_to = slice_to,
    .slice_horizontal = true,
  };
}

void draw_scene(const ObjectArray *objects,
                TileState tile_state, Texture_State *texture_state,
                SeasonBlend season_blend, Map *map,
                int highlight_index, const Selection *selection, const GameState *game_state,
                const Object *preview, bool preview_active, Color preview_tint,
                float water_level_z, SoilOverlayState soil_overlay_state) {
  update_scales(&scales, tile_state, texture_state, season_blend);

  int min_tx, max_tx, min_ty, max_ty;
  visible_tile_bounds(tile_state, map, &min_tx, &max_tx, &min_ty, &max_ty);
  draw_tiles(tile_state, texture_state, season_blend, map, objects, highlight_index, min_tx, max_tx, min_ty, max_ty, water_level_z, soil_overlay_state);

  int n_objects = objects->count;
  // Grass/wheat tufts push 2 ObjectOrder entries each, felled south-east trees
  // push TREE_TRUNK_TILE_SPAN - sizing for the worst case per object so the
  // buffer can't overflow regardless of how many of which kind are on screen.
  int total = n_objects * TREE_TRUNK_TILE_SPAN + (preview_active ? 1 : 0);
  if (total > 0) {
    ObjectOrder *order = malloc(total * sizeof(ObjectOrder));
    int visible_count = 0;

    const float sprite_slice_to = 1.0; // 0.5f;

    for (int i = 0; i < n_objects; i++) {
      if (!object_in_bounds(&objects->data[i], min_tx, max_tx, min_ty, max_ty)) continue;
      ObjectKind k = objects->data[i].kind;
      if (soil_overlay_state.current != SOIL_OVERLAY_NONE && (k == OBJECT_GRASS_TUFT || k == OBJECT_WHEAT_TUFT || k == OBJECT_PUDDLE)) continue;

      if (k == OBJECT_GRASS_TUFT || k == OBJECT_WHEAT_TUFT) {
        ObjectOrder foot = create_ObjectOrder_from_object(&objects->data[i], i);
        foot.sprite_slice_from = sprite_slice_to;
        foot.sprite_slice_to = 1.0f;
        order[visible_count++] = foot;

        ObjectOrder tip = create_ObjectOrder_from_object(&objects->data[i], i);
        tip.sprite_slice_from = 0.0f;
        tip.sprite_slice_to = sprite_slice_to;
        order[visible_count++] = tip;
        continue;
      }

      if (k == OBJECT_TREE && objects->data[i].tree.state != TREE_STATE_STANDING && objects->data[i].facing == FIGURE_DIR_RIGHT) {
        for (int seg = 0; seg < TREE_TRUNK_TILE_SPAN; seg++) {
          float slice_from = (float)seg / TREE_TRUNK_TILE_SPAN;
          float slice_to = (float)(seg + 1) / TREE_TRUNK_TILE_SPAN;
          order[visible_count++] = create_tree_segment_order(&objects->data[i], i, seg, slice_from, slice_to);
        }
        continue;
      }

      order[visible_count++] = create_ObjectOrder_from_object(&objects->data[i], i);
    }

    if (preview_active) {
      order[visible_count] = create_ObjectOrder_from_object(preview, -1);
      order[visible_count++].is_preview = true;
    }
    total = visible_count;
    qsort(order, total, sizeof(ObjectOrder), compare_object_order);

    unsigned int selected_mansus_farmer_id = 0;
    unsigned int selected_mansus_farmhand_id = 0;
    if (selection->mansus_idx >= 0 && selection->mansus_idx < game_state->mansen.count) {
      const Mansus *m = &game_state->mansen.data[selection->mansus_idx];
      selected_mansus_farmer_id = m->farmer_object_id;
      selected_mansus_farmhand_id = m->farmhand_object_id;
    }

    for (int i = 0; i < total; i++) {
      if (order[i].is_preview) {
        draw_object(tile_state, preview, preview_tint);
        continue;
      }

      if (objects->data[order[i].idx].kind == OBJECT_BRIDGE ||
        objects->data[order[i].idx].kind == OBJECT_MANSUS ||
        objects->data[order[i].idx].kind == OBJECT_ROAD) continue;

      const Object *o = &objects->data[order[i].idx];
      Color tint = WHITE;
      bool is_selected = o->kind == OBJECT_FIGURE && selection_contains(selection, o->id);
      bool is_selected_mansus_worker = o->kind == OBJECT_FIGURE && o->id != 0 &&
        (o->id == selected_mansus_farmer_id || o->id == selected_mansus_farmhand_id);
      if (order[i].idx == highlight_index) {
        tint = RED;
      } else if (is_selected) {
        tint = PURPLE;
      } else if (is_selected_mansus_worker) {
        tint = GOLD;
      } else if (o->kind == OBJECT_BOUNDARY_STONE && o->boundary_stone.is_field_boundary) {
        tint = RED;
      }
      if (order[i].slice_horizontal) {
        draw_object_slice_x(tile_state, o, tint, order[i].sprite_slice_from, order[i].sprite_slice_to);
      } else {
        draw_object_slice(tile_state, o, tint, order[i].sprite_slice_from, order[i].sprite_slice_to);
      }
    }
    free(order);
  }
}


static bool selection_has_ox(const ObjectArray *objects, const Selection *sel) {
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (o->kind == OBJECT_FIGURE && o->figure.species == FIGURE_SPECIES_OX &&
        selection_contains(sel, o->id)) {
      return true;
    }
  }
  return false;
}

void draw_cursor(const Texture_State *texture_state, const TileState* tile_state,
                 const ObjectArray* objects, const Selection* sel, Map *map) {
  Vector2 mouse = GetMousePosition();
  const SpriteAsset *sprite = &texture_state->cursor;
  if (sel->count > 0) {
    const bool is_tree_target = find_tree_at_screen_pos(objects, *tile_state, mouse) >= 0;
    if (is_tree_target) {
      sprite = &texture_state->axe_cursor;
    } else if (selection_has_ox(objects, sel)) {
      int tx = screen_to_tile_x(*tile_state, mouse.x, mouse.y);
      int ty = screen_to_tile_y(*tile_state, mouse.x, mouse.y);
      const Tile *t = map_tile(map, tx, ty);
      if (t && t->type == TILE_SOIL) {
        sprite = &texture_state->plow_cursor;
      }
    }
  }
  Vector2 pos = {
    mouse.x - sprite->tex.width  * sprite->scale * sprite->anchor.x,
    mouse.y - sprite->tex.height * sprite->scale * sprite->anchor.y,
  };
  DrawTextureEx(sprite->tex, pos, 0.0f, sprite->scale, WHITE);
}

void draw_mansus_assign_flash(TileState tile_state, Map *map, const GameState *game_state) {
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    if (m->assign_flash_timer <= 0.0f) continue;

    float alpha = m->assign_flash_timer / MANSUS_ASSIGN_FLASH_SECONDS;
    Color tint = (Color){80, 220, 90, (unsigned char)(140 * alpha)};

    for (int fy = m->corners_floor_area[0][1]; fy <= m->corners_floor_area[1][1]; fy++) {
      for (int fx = m->corners_floor_area[0][0]; fx <= m->corners_floor_area[1][0]; fx++) {
        const Tile *t = map_tile(map, fx, fy);
        if (!t) continue;
        int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
        int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - t->z * tile_state.TILE_H;
        draw_diamond(tile_state, sx, sy, tint);
      }
    }
  }
}

void draw_mansus_selection_highlight(TileState tile_state, Map *map, const GameState *game_state, const Selection *selection) {
  if (selection->mansus_idx < 0 || selection->mansus_idx >= game_state->mansen.count) return;
  const Mansus *m = &game_state->mansen.data[selection->mansus_idx];

  const Color yard_tint = (Color){255, 215, 0, 70};
  for (int fy = m->corners_floor_area[0][1]; fy <= m->corners_floor_area[1][1]; fy++) {
    for (int fx = m->corners_floor_area[0][0]; fx <= m->corners_floor_area[1][0]; fx++) {
      const Tile *t = map_tile(map, fx, fy);
      if (!t) continue;
      int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
      int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - t->z * tile_state.TILE_H;
      draw_diamond_outline(tile_state, sx, sy, GOLD);
      draw_diamond(tile_state, sx, sy, yard_tint);
    }
  }

  const Color field_tint = (Color){255, 215, 0, 90};
  for (int fi = 0; fi < m->fields.count; fi++) {
    const Field *f = &m->fields.data[fi];
    for (int fy = f->corners_field[0][1]; fy <= f->corners_field[1][1]; fy++) {
      for (int fx = f->corners_field[0][0]; fx <= f->corners_field[1][0]; fx++) {
        const Tile *t = map_tile(map, fx, fy);
        if (!t) continue;
        int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
        int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - t->z * tile_state.TILE_H;
        draw_diamond(tile_state, sx, sy, field_tint);
        draw_diamond_outline(tile_state, sx, sy, GOLD);
      }
    }
  }
}

// Placeholder status badge for the selected field, until real pixellab icons exist.
void draw_field_status_icon(TileState tile_state, Map *map, const GameState *game_state, const Selection *selection) {
  if (selection->field_mansus_idx < 0 || selection->field_mansus_idx >= game_state->mansen.count) return;
  const Mansus *m = &game_state->mansen.data[selection->field_mansus_idx];
  if (selection->field_idx < 0 || selection->field_idx >= m->fields.count) return;
  const Field *f = &m->fields.data[selection->field_idx];

  int center_tx = (f->corners_field[0][0] + f->corners_field[1][0]) / 2;
  int center_ty = (f->corners_field[0][1] + f->corners_field[1][1]) / 2;
  const Tile *t = map_tile(map, center_tx, center_ty);
  float z = t ? t->z : 0.0f;
  int sx = (center_tx - center_ty) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
  int sy = (center_tx + center_ty) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - (int)(z * tile_state.TILE_H);

  const char *label;
  Color color;
  if (f->field_condition == GRASS || f->field_condition == FALLOW) {
    label = "Has to be plowed";
    color = (Color){90, 130, 60, 255};
  } else if (f->field_condition == PLOWED) {
    label = "Fallow";
    color = (Color){139, 90, 43, 255};
  } else if (f->field_condition == SOWED) {
    label = "Sowed";
    color = (Color){212, 175, 55, 255};
  } else {
    label = "Growing";
    color = (Color){60, 160, 60, 255};
  }

  int icon_y = sy - tile_state.TILE_H * 2;
  int radius = tile_state.TILE_W / 6;
  DrawCircle(sx, icon_y, (float)radius, color);
  DrawCircleLines(sx, icon_y, (float)radius, BLACK);

  int font_size = 14;
  int text_width = MeasureText(label, font_size);
  DrawText(label, sx - text_width / 2, icon_y + radius + 4, font_size, BLACK);
}

static void draw_goods_row(int x, int y, int font_size, const char *label, int amount) {
  DrawText(TextFormat("%s: %d", label, amount), x, y, font_size, WHITE);
}

void draw_mansus_goods_panel(const GameState *game_state, const Selection *selection) {
  if (!selection->barn_selected) return;
  if (selection->mansus_idx < 0 || selection->mansus_idx >= game_state->mansen.count) return;
  const Mansus *m = &game_state->mansen.data[selection->mansus_idx];
  const Goods *g = &m->goods;

  float scale = topbar_scale();
  float vscale = topbar_vertical_scale();
  int font_size = (int)(20 * vscale);
  int line_height = (int)(26 * vscale);
  int padding = (int)(15 * scale);
  int panel_w = (int)(220 * scale);
  int panel_h = padding * 2 + line_height * 7;
  Rectangle r = {0, topbar_panel_rect().height + 10 * vscale, (float)panel_w, (float)panel_h};

  DrawRectangleRounded(r, 0.1f, 8, BEIGE);
  DrawRectangleRoundedLinesEx(r, 0.1f, 8, 2.0f, BLACK);

  int x = (int)r.x + padding;
  int y = (int)r.y + padding;
  draw_goods_row(x, y, font_size, "Grains", g->grains); y += line_height;
  draw_goods_row(x, y, font_size, "Straw", g->straw); y += line_height;
  draw_goods_row(x, y, font_size, "Wood", g->wood); y += line_height;
  draw_goods_row(x, y, font_size, "Clay", g->clay); y += line_height;
  draw_goods_row(x, y, font_size, "Limestone", g->limestone); y += line_height;
  draw_goods_row(x, y, font_size, "Slaked lime", g->slaked_lime); y += line_height;
  draw_goods_row(x, y, font_size, "Willow branches", g->willow_branches);
}
