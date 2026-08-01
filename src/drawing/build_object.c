#include "drawing/build_object.h"
#include "drawing/drawing_helper.h"
#include "drawing/update_figure.h"
#include "a_star/a_star.h"
#include "textures/textures.h"
#include "containers/arrays.h"
#include "utils/game_time.h"
#include <stdlib.h>

void place_road_tile(ObjectArray *objects, Map *map, const Texture_State *texture_state, int tx, int ty) {
  if (!map_is_placeable(map, tx, ty, 1, 1)) return;

  int variant = tile_variant(tx, ty, BUILDING_DIR_COUNT);
  SpriteAsset sprite;
  sprite.tex = texture_state->road_flat[variant];
  sprite.scale = 64.0f / sprite.tex.width;
  sprite.anchor = (Vector2){0.5f, 1.0f};

  Object road = {
    .sprite = sprite,
    .tx = tx, .ty = ty, .z = 0,
    .footprint_w = 1, .footprint_h = 1,
    .kind = OBJECT_ROAD,
  };
  road.id = allocate_object_id();
  object_array_push(objects, road);
  map_place_object(map, tx, ty, 1, 1, true);
}

// Delete objects
// -----------------------------------------------------------------
int hovered_delete_target(const ObjectArray *objects, TileState tile_state, ModeState mode_state) {
  if (mode_state.current != BUILD_DELETE) return -1;
  Vector2 current = GetMousePosition();
  int tx = screen_to_tile_x(tile_state, current.x, current.y);
  int ty = screen_to_tile_y(tile_state, current.x, current.y);
  return find_object_at_tile(objects, tx, ty);
}

int hovered_delete_field(const GameState *game_state, TileState tile_state, ModeState mode_state, int *out_field_idx) {
  if (mode_state.current != BUILD_DELETE) return -1;
  Vector2 current = GetMousePosition();
  int tx = screen_to_tile_x(tile_state, current.x, current.y);
  int ty = screen_to_tile_y(tile_state, current.x, current.y);
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    const Mansus *m = &game_state->mansen.data[mi];
    for (int fi = 0; fi < m->fields.count; fi++) {
      const Field *f = &m->fields.data[fi];
      if (tx < f->corners_field[0][0] || tx > f->corners_field[1][0]) continue;
      if (ty < f->corners_field[0][1] || ty > f->corners_field[1][1]) continue;
      *out_field_idx = fi;
      return mi;
    }
  }
  return -1;
}

void delete_field(ObjectArray *objects, Map *map, GameState *game_state, int mansus_idx, int field_idx, bool icon_hovered) {
  if (mansus_idx < 0 || field_idx < 0 || icon_hovered || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
  Mansus *m = &game_state->mansen.data[mansus_idx];
  if (field_idx >= m->fields.count) return;
  Field *f = &m->fields.data[field_idx];

  for (int i = objects->count - 1; i >= 0; i--) {
    Object *o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;
    if (o->tx < f->corners_field[0][0] || o->tx > f->corners_field[1][0]) continue;
    if (o->ty < f->corners_field[0][1] || o->ty > f->corners_field[1][1]) continue;
    object_array_remove_swap(objects, i);
  }

  for (int y = f->corners_field[0][1]; y <= f->corners_field[1][1]; y++) {
    for (int x = f->corners_field[0][0]; x <= f->corners_field[1][0]; x++) {
      Tile *t = map_tile(map, x, y);
      if (t) t->type = TILE_GRASS;
    }
  }

  field_array_remove_swap(&m->fields, field_idx);
}

int find_mansus_at(GameState *game_state, int x, int y) {
  for (int i = 0; i < game_state->mansen.count; i++) {
    Mansus *m = &game_state->mansen.data[i];
    int min_x = m->corners_floor_area[0][0];
    int min_y = m->corners_floor_area[0][1];
    int max_x = m->corners_floor_area[1][0];
    int max_y = m->corners_floor_area[1][1];
    if (x >= min_x && x <= max_x && y >= min_y && y <= max_y) return i;
  }
  return -1;
}

void update_mansus_effects(GameState *game_state) {
  float dt = game_delta_time();
  for (int i = 0; i < game_state->mansen.count; i++) {
    Mansus *m = &game_state->mansen.data[i];
    if (m->assign_flash_timer <= 0.0f) continue;
    m->assign_flash_timer -= dt;
    if (m->assign_flash_timer < 0.0f) m->assign_flash_timer = 0.0f;
  }
}

void delete_object(ObjectArray *objects, Map *map,
                   GameState *game_state, int index, bool icon_hovered,
                   FloodFieldArray* flood_field_state) {
  if (index < 0 || icon_hovered || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

  Object *o = &objects->data[index];
  unsigned int deleted_id = o->id;

  if (o->kind == OBJECT_MANSUS) {
    int mansus_idx = find_mansus_at(game_state, o->tx, o->ty);
    if (mansus_idx >= 0) {
      Mansus *m = &game_state->mansen.data[mansus_idx];
      if (m->living_house != 0 || m->barn != 0 || m->fields.count > 0 || m->farmer_object_id != 0 || m->farmhand_object_id != 0) return;
      int back_x = m->corners_floor_area[0][0];
      int back_y = m->corners_floor_area[0][1];
      int front_x = m->corners_floor_area[1][0];
      int front_y = m->corners_floor_area[1][1];
      field_array_free(&m->fields);
      mansus_array_remove_swap(&game_state->mansen, mansus_idx);
      for (int i = objects->count - 1; i >= 0; i--) {
        Object *go = &objects->data[i];
        if (go->kind != OBJECT_BOUNDARY_STONE) continue;
        bool at_corner = (go->tx == back_x - 1 || go->tx == front_x) &&
                          (go->ty == back_y - 1 || go->ty == front_y);
        if (!at_corner) continue;
        object_array_remove_swap(objects, i);
      }
    }
  } else {
    for (int i = 0; i < game_state->mansen.count; i++) {
      Mansus *m = &game_state->mansen.data[i];
      if (m->living_house == o->id) m->living_house = 0;
      if (m->barn == o->id) m->barn = 0;
      if (m->farmer_object_id == o->id) {
        m->farmer_object_id = 0;
        m->familia = (Familia){0};
      }
    }
  }

  int final_index = -1;
  for (int i = 0; i < objects->count; i++) {
    if (objects->data[i].id == deleted_id) { final_index = i; break; }
  }
  if (final_index < 0) return;
  o = &objects->data[final_index];

  if (o->kind == OBJECT_FIGURE && o->figure.flood_field_idx >= 0) {
    figure_release_reservation(map, o);
    flood_field_array_release(flood_field_state, o->figure.flood_field_idx);
  }
  map_clear_object(map, o);
  object_array_remove_swap(objects, final_index);
}

static void scatter_boundary_stones(ObjectArray *objects, Map *map, const Texture_State *texture_state,
                                    int tx, int ty, int footprint_w, int footprint_h, bool is_field_boundary) {
  int back_x = tx - footprint_w + 1;
  int back_y = ty - footprint_h + 1;
  int corner_x[4] = {back_x - 1, tx, back_x - 1, tx};
  int corner_y[4] = {back_y - 1, back_y - 1, ty, ty};

  for (int i = 0; i < 4; i++) {
    Tile *t = map_tile(map, corner_x[i], corner_y[i]);
    if (!t) continue;
    Object stone = {
      .sprite = texture_state->boundary_stone,
      .tx = corner_x[i], .ty = corner_y[i], .z = t->z,
      .footprint_w = 1, .footprint_h = 1,
      .kind = OBJECT_BOUNDARY_STONE,
      .boundary_stone = {.is_field_boundary = is_field_boundary},
    };
    object_array_push(objects, stone);
  }
}

// Build a texture
// -----------------------------------------------------------------
BuildPreview build_object_preview(TileState tile_state, ModeState mode_state, const Texture_State *texture_state,
                                  Map *map, GameState *game_state, SeasonBlend season_blend) {
  BuildPreview preview = {0};

  Vector2 current = GetMousePosition();
  int tx = screen_to_tile_x(tile_state, current.x, current.y);
  int ty = screen_to_tile_y(tile_state, current.x, current.y);
  const int tz = 0;

  BuildingDirection dir = mode_state.build_direction;
  bool rotated_90 = (dir == BUILDING_DIR_NW || dir == BUILDING_DIR_SW);

  SpriteAsset sprite;
  int footprint_w;
  int footprint_h;
  if (mode_state.current == BUILD_BRIDGE) {
    sprite.tex = texture_state->bridge[0];
    sprite.scale = 64.0f / sprite.tex.width;
    sprite.anchor = (Vector2){0.5f, 1.0f};
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_LIVING_HOUSE) {
    sprite = texture_state->house1[dir];
    footprint_w = rotated_90 ? 4 : 2;
    footprint_h = rotated_90 ? 2 : 4;
  } else if (mode_state.current == BUILD_BARN) {
    sprite = texture_state->house2[dir];
    footprint_w = 2;
    footprint_h = 2;
  } else if (mode_state.current == BUILD_OAK) {
    int variant = tile_variant(tx, ty, BUILDING_DIR_COUNT);
    sprite = texture_state->oak[season_blend.base][variant];
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_GRASS) {
    sprite = texture_state->grass_tuft_idle[season_blend.base][tile_variant(tx, ty, FIGURE_DIR_COUNT)];
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_ROAD) {
    int variant = tile_variant(tx, ty, TILE_VARIANT_COUNT);
    sprite.tex = texture_state->road_flat[variant];
    sprite.scale = 64.0f / sprite.tex.width;
    sprite.anchor = (Vector2){0.5f, 1.0f};
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_FARMER1) {
    sprite.tex = texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex;
    sprite.scale = texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].scale;
    sprite.anchor = texture_state->farmers[0][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].anchor;
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_FARMER2) {
    sprite.tex = texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].tex;
    sprite.scale = texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].scale;
    sprite.anchor = texture_state->farmers[1][FIGURE_ACTION_STAND][FIGURE_DIR_FRONT][0].anchor;
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_OX) {
    sprite = texture_state->ox_stand[FIGURE_DIR_FRONT];
    footprint_w = 1;
    footprint_h = 1;
  } else if (mode_state.current == BUILD_MANSUS) {
    sprite.tex = texture_state->road_flat[13];
    sprite.scale = 64.0f / sprite.tex.width;
    sprite.anchor = (Vector2){0.5f, 1.0f};
    footprint_w = rotated_90 ? MANSUS_FOOTPRINT_H : MANSUS_FOOTPRINT_W;
    footprint_h = rotated_90 ? MANSUS_FOOTPRINT_W : MANSUS_FOOTPRINT_H;
  } else {
    return preview;
  }

  ObjectKind kind = OBJECT_BUILDING;

  if (mode_state.current == BUILD_FARMER1 || mode_state.current == BUILD_FARMER2 || mode_state.current == BUILD_OX) kind = OBJECT_FIGURE;
  else if (mode_state.current == BUILD_BRIDGE) kind = OBJECT_BRIDGE;
  else if (mode_state.current == BUILD_MANSUS) kind = OBJECT_MANSUS;
  else if (mode_state.current == BUILD_OAK) kind = OBJECT_TREE;
  else if (mode_state.current == BUILD_GRASS) kind = OBJECT_GRASS_TUFT;

  FigureSpecies species = FIGURE_SPECIES_FARMER1;
  if (mode_state.current == BUILD_FARMER2) species = FIGURE_SPECIES_FARMER2;
  else if (mode_state.current == BUILD_OX) species = FIGURE_SPECIES_OX;

  preview.obj = (Object){
    .sprite = sprite,
    .tx = tx, .ty = ty, .z = tz,
    .footprint_w = footprint_w, .footprint_h = footprint_h,
    .kind = kind,
    .facing = (FigureDirection)tile_variant(tx, ty, FIGURE_DIR_COUNT),
    .figure = {
      .gather_tx = tx, .gather_ty = ty,
      .species = species, .flood_field_idx = -1, .prev_tile = -1,
      .best_distance_to_target = -1
    },
  };

  bool placeable = false;
  if (mode_state.current == BUILD_BRIDGE) {
    placeable = map_bridge_is_placeable(map, tx, ty, footprint_w, footprint_h);
  } else if (mode_state.current == BUILD_FARMER1 || mode_state.current == BUILD_FARMER2 || mode_state.current == BUILD_OX) {
    placeable = map_figure_is_placeable(map, tx, ty, footprint_w, footprint_h);
  } else if (mode_state.current == BUILD_LIVING_HOUSE || mode_state.current == BUILD_BARN) {
    placeable = map_building_is_placeable(map, tx, ty, footprint_w, footprint_h);
    if (placeable) {
      int back_x = tx - footprint_w + 1;
      int back_y = ty - footprint_h + 1;
      int mansus_idx = find_mansus_at(game_state, back_x, back_y);
      if (mansus_idx < 0 || mansus_idx != find_mansus_at(game_state, tx, ty)) {
        placeable = false;
      } else {
        Mansus *m = &game_state->mansen.data[mansus_idx];
        unsigned int already_built = (mode_state.current == BUILD_LIVING_HOUSE) ? m->living_house : m->barn;
        if (already_built != 0) placeable = false;
      }
    }
  } else if (mode_state.current == BUILD_MANSUS) {
    placeable = map_mansus_is_placeable(map, tx, ty, footprint_w, footprint_h);
  } else if (mode_state.current == BUILD_GRASS) {
    const Tile *t = map_tile(map, tx, ty);
    placeable = t && t->type == TILE_GRASS;
  } else {
    placeable = map_is_placeable(map, tx, ty, footprint_w, footprint_h);
  }

  preview.active = true;
  preview.show_ghost = (mode_state.current != BUILD_MANSUS);
  preview.placeable = placeable;
  preview.tint = placeable ? DARKGREEN : RED;
  return preview;
}

void build_object(ObjectArray *objects, const BuildPreview *preview,
                  TileState tile_state, ModeState mode_state,
                  Map* map, GameState *game_state, const Texture_State *texture_state,
                  bool icon_hovered) {
  if (!preview->active) return;

  int tx = preview->obj.tx;
  int ty = preview->obj.ty;

  if (mode_state.current == BUILD_MANSUS) {
    Color area_tint = preview->placeable ? (Color){0, 200, 0, 110} : (Color){200, 0, 0, 110};
    for (int fy = ty - preview->obj.footprint_h + 1; fy <= ty; fy++) {
      for (int fx = tx - preview->obj.footprint_w + 1; fx <= tx; fx++) {
        const Tile *t = map_tile(map, fx, fy);
        if (!t) continue;
        int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
        int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - t->z * tile_state.TILE_H;
        draw_diamond(tile_state, sx, sy, area_tint);
      }
    }
  }

  if (mode_state.current == BUILD_LIVING_HOUSE || mode_state.current == BUILD_BARN) {
    Color Yellow = (Color){230, 200, 60, 140};
    Color Red = (Color){200, 0, 0, 110};
    for (int mi = 0; mi < game_state->mansen.count; mi++) {
      Mansus *m = &game_state->mansen.data[mi];
      unsigned int already_built = (mode_state.current == BUILD_LIVING_HOUSE) ? m->living_house : m->barn;
      Color tint = (already_built == 0) ? Yellow : Red;
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
  if (mode_state.current == BUILD_ROAD) return;

  if (!(preview->placeable && !icon_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
    return;
  }

  Object current_obj = preview->obj;
  current_obj.id = allocate_object_id();
  object_array_push(objects, current_obj);

  if (mode_state.current == BUILD_OAK) {
    map_place_object(map, tx, ty, current_obj.footprint_w, current_obj.footprint_h, false);
  }
  else if (mode_state.current == BUILD_LIVING_HOUSE || mode_state.current == BUILD_BARN) {
    map_place_object(map, tx, ty, current_obj.footprint_w, current_obj.footprint_h, false);
    int mansus_idx = find_mansus_at(game_state, tx, ty);
    if (mansus_idx >= 0) {
      if (mode_state.current == BUILD_LIVING_HOUSE) {
        game_state->mansen.data[mansus_idx].living_house = current_obj.id;
      } else {
        game_state->mansen.data[mansus_idx].barn = current_obj.id;
      }
    }
  }
  else if (mode_state.current == BUILD_MANSUS) {
    for (int fy = ty - current_obj.footprint_h + 1; fy <= ty; fy++) {
      for (int fx = tx - current_obj.footprint_w + 1; fx <= tx; fx++) {
        Tile *t = map_tile(map, fx, fy);
        if (t) {
          t->type = TILE_MANSUSYARD;
          t->occupied = 0;
        }
      }
    }

    for (int i = objects->count - 1; i >= 0; i--) {
      Object *go = &objects->data[i];
      if (go->kind != OBJECT_GRASS_TUFT) continue;
      if (go->tx < tx - current_obj.footprint_w + 1 || go->tx > tx) continue;
      if (go->ty < ty - current_obj.footprint_h + 1 || go->ty > ty) continue;
      object_array_remove_swap(objects, i);
    }

    Mansus new_mansus = {
      .mansus_level = HOUSLER,
      .corners_floor_area = {
        {tx - current_obj.footprint_w + 1, ty - current_obj.footprint_h + 1},
        {tx, ty},
      },
      .living_house = 0,
      .barn = 0,
      .fields = {0},
      .farmer_object_id = 0,
      .familia = {0},
      .goods = {.grains = FIELD_SOW_GRAIN_COST},
      .assign_flash_timer = 0.0f,
    };
    mansus_array_push(&game_state->mansen, new_mansus);
    scatter_boundary_stones(objects, map, texture_state, tx, ty, current_obj.footprint_w, current_obj.footprint_h, false);
  }
  else if (current_obj.kind == OBJECT_FIGURE) {
    map_place_figure(map, tx, ty);
  }

  if (mode_state.current == BUILD_BRIDGE) {
    Tile *t = map_tile(map, tx, ty);
    if (t) {
      t->type = TILE_ROAD;
      t->z = -1;
    }
  }
}

// Field placement:
// --------------------------------------------------------------------
void update_field_action(FieldAssignState *field_state, TileState tile_state, ModeState *mode_state,
                         Map *map, GameState *game_state, bool icon_hovered,
                         ObjectArray *objects, const Texture_State *texture_state) {
  Vector2 current = GetMousePosition();
  int tx = screen_to_tile_x(tile_state, current.x, current.y);
  int ty = screen_to_tile_y(tile_state, current.x, current.y);

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    field_state->assigning = false;
    return;
  }

  if (!field_state->assigning) {
    if (mode_state->current != BUILD_FIELD) return;

    bool placeable = map_is_placeable(map, tx, ty, QUARTER_HUFE_FOOTPRINT_W, QUARTER_HUFE_FOOTPRINT_H);
    Color area_tint = placeable ? (Color){230, 200, 60, 140} : (Color){200, 0, 0, 110};
    for (int fy = ty - QUARTER_HUFE_FOOTPRINT_H + 1; fy <= ty; fy++) {
      for (int fx = tx - QUARTER_HUFE_FOOTPRINT_W + 1; fx <= tx; fx++) {
        const Tile *ft = map_tile(map, fx, fy);
        if (!ft) continue;
        int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
        int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - ft->z * tile_state.TILE_H;
        draw_diamond(tile_state, sx, sy, area_tint);
      }
    }

    if (!(placeable && !icon_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) return;

    field_state->corners_field[0][0] = tx - QUARTER_HUFE_FOOTPRINT_W + 1;
    field_state->corners_field[0][1] = ty - QUARTER_HUFE_FOOTPRINT_H + 1;
    field_state->corners_field[1][0] = tx;
    field_state->corners_field[1][1] = ty;
    field_state->crop = SUMMER_WHEAT;
    field_state->assigning = true;
    mode_state->current = MOVEMENT;
    return;
  }

  // Assigning stage: highlight every Mansus
  for (int mi = 0; mi < game_state->mansen.count; mi++) {
    Mansus *m = &game_state->mansen.data[mi];
    bool eligible = m->fields.count < MANSUS_MAX_FIELDS;
    Color Yellow = (Color){230, 200, 60, 140};
    Color Red = (Color){200, 0, 0, 110};
    Color tint = eligible ? Yellow : Red;
    for (int fy = m->corners_floor_area[0][1]; fy <= m->corners_floor_area[1][1]; fy++) {
      for (int fx = m->corners_floor_area[0][0]; fx <= m->corners_floor_area[1][0]; fx++) {
        const Tile *ft = map_tile(map, fx, fy);
        if (!ft) continue;
        int sx = (fx - fy) * tile_state.TILE_W / 2 + tile_state.OFFSET_X;
        int sy = (fx + fy) * tile_state.TILE_H / 2 + tile_state.OFFSET_Y - ft->z * tile_state.TILE_H;
        draw_diamond(tile_state, sx, sy, tint);
      }
    }
  }

  if (mode_state->current != MOVEMENT || icon_hovered || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

  Tile *t = map_tile(map, tx, ty);
  if (!t || t->type != TILE_MANSUSYARD) return;
  int mansus_idx = find_mansus_at(game_state, tx, ty);
  if (mansus_idx < 0) return;

  Mansus *m = &game_state->mansen.data[mansus_idx];
  if (m->fields.count >= MANSUS_MAX_FIELDS) return;

  Field new_field = {
    .field_condition = GRASS,
    .crop = field_state->crop,
    .corners_field = {
      {field_state->corners_field[0][0], field_state->corners_field[0][1]},
      {field_state->corners_field[1][0], field_state->corners_field[1][1]},
    },
    .worked_by_figure_id = 0,
    .cultivated_last_in_year = -1
  };
  // Freshly assigned fields are just marked by boundary stones for now; the ground
  // itself only turns into arable soil tile by tile as it gets dug (see the dig route).
  scatter_boundary_stones(objects, map, texture_state, new_field.corners_field[1][0], new_field.corners_field[1][1],
                          QUARTER_HUFE_FOOTPRINT_W, QUARTER_HUFE_FOOTPRINT_H, true);
  field_array_push(&m->fields, new_field);
  field_state->assigning = false;
}
