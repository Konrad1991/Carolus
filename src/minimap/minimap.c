#include "minimap/minimap.h"
#include "drawing/drawing.h"
#include "utils/game_time.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>

static const Color MINIMAP_FIELD_COLOR = {225, 190, 80, 255};
static const Color MINIMAP_TREE_COLOR = {25, 70, 20, 255};
static const Color MINIMAP_FIGURE_COLOR = {225, 140, 40, 255};

static Rectangle minimap_rect(void) {
  const float minimap_size = 200.0f;
  const float minimap_margin = 20.0f;
  return (Rectangle){
    minimap_margin,
    GetScreenHeight() - minimap_size - minimap_margin,
    minimap_size,
    minimap_size
  };
}

static Color minimap_tile_color(TileType type) {
  switch (type) {
    case TILE_GRASS: return (Color){78, 120, 44, 255};
    case TILE_MANSUSYARD: return (Color){78, 120, 44, 255};
    case TILE_ROAD: return (Color){130, 105, 75, 255};
    case TILE_WATER: return (Color){30, 100, 200, 255};
    case TILE_SOIL: return (Color){110, 82, 48, 255};
  }
  return BLACK;
}

static void paint_fields(Image *img, const Map *map, const GameState *game_state) {
  for (int m = 0; m < game_state->mansen.count; m++) {
    const FieldArray *fields = &game_state->mansen.data[m].fields;
    for (int f = 0; f < fields->count; f++) {
      const Field *field = &fields->data[f];
      for (int y = field->corners_field[0][1]; y <= field->corners_field[1][1]; y++) {
        for (int x = field->corners_field[0][0]; x <= field->corners_field[1][0]; x++) {
          if (x < 0 || x >= map->w || y < 0 || y >= map->h) continue;
          ImageDrawPixel(img, x, y, MINIMAP_FIELD_COLOR);
        }
      }
    }
  }
}

static void paint_object_kind_density(Image *img, const Map *map, const ObjectArray *objects,
                                      ObjectKind kind, Color tint) {
  const int density_radius = 2;
  const float max_object_density = 3.0f;
  float *density = calloc((size_t)map->w * (size_t)map->h, sizeof(float));
  for (int i = 0; i < objects->count; i++) {
    const Object *o = &objects->data[i];
    if (o->kind != kind) continue;
    for (int dy = - density_radius; dy <= density_radius; dy++) {
      for (int dx = - density_radius; dx <= density_radius; dx++) {
        int tx = o->tx + dx;
        int ty = o->ty + dy;
        if (tx < 0 || tx >= map->w || ty < 0 || ty >= map->h) continue;
        float dist = sqrtf((float)(dx * dx + dy * dy));
        density[ty * map->w + tx] += 1.0f / (1.0f + dist);
      }
    }
  }

  for (int y = 0; y < map->h; y++) {
    for (int x = 0; x < map->w; x++) {
      float local_density = density[y * map->w + x];
      if (local_density <= 0.0f) continue;
      float alpha = fminf(local_density / max_object_density, 1.0f);
      Color base = GetImageColor(*img, x, y);
      ImageDrawPixel(img, x, y, ColorLerp(base, tint, alpha));
    }
  }
  free(density);
}

static Image build_terrain_image(const Map *map, const ObjectArray *objects, const GameState *game_state) {
  Image img = GenImageColor(map->w, map->h, BLANK);
  for (int y = 0; y < map->h; y++) {
    for (int x = 0; x < map->w; x++) {
      ImageDrawPixel(&img, x, y, minimap_tile_color(map->tiles[y * map->w + x].type));
    }
  }
  paint_fields(&img, map, game_state);
  paint_object_kind_density(&img, map, objects, OBJECT_TREE, MINIMAP_TREE_COLOR);
  paint_object_kind_density(&img, map, objects, OBJECT_FIGURE, MINIMAP_FIGURE_COLOR);
  return img;
}

void init_minimap_state(MinimapState *state, const Map *map, const ObjectArray *objects, const GameState *game_state) {
  Image img = build_terrain_image(map, objects, game_state);
  state->terrain_texture = LoadTextureFromImage(img);
  UnloadImage(img);
  state->terrain_texture_loaded = true;
  state->refresh_timer = 0.0f;
  state->hovered = false;
}

void free_minimap_state(MinimapState *state) {
  if (!state->terrain_texture_loaded) return;
  UnloadTexture(state->terrain_texture);
  state->terrain_texture_loaded = false;
}

void draw_minimap(MinimapState *state, const Map *map, const TileState *tile_state,
                  const ObjectArray *objects, const GameState *game_state) {
  const float minimap_refresh_interval = 1.5f;
  state->refresh_timer += game_delta_time();
  if (state->refresh_timer >= minimap_refresh_interval) {
    state->refresh_timer = 0.0f;
    Image img = build_terrain_image(map, objects, game_state);
    UpdateTexture(state->terrain_texture, img.data);
    UnloadImage(img);
  }

  Rectangle rect = minimap_rect();
  Rectangle panel = {rect.x - 4, rect.y - 4, rect.width + 8, rect.height + 8};
  DrawRectangleRounded(panel, 0.05f, 6, BEIGE);
  DrawRectangleRoundedLinesEx(panel, 0.05f, 6, 2.0f, BLACK);

  Rectangle src = {0, 0, (float)map->w, (float)map->h};
  DrawTexturePro(state->terrain_texture, src, rect, (Vector2){0, 0}, 0.0f, WHITE);

  int min_tx, max_tx, min_ty, max_ty;
  visible_tile_bounds(*tile_state, map, &min_tx, &max_tx, &min_ty, &max_ty);
  Rectangle viewport = {
    rect.x + (float)min_tx / map->w * rect.width,
    rect.y + (float)min_ty / map->h * rect.height,
    (float)(max_tx - min_tx) / map->w * rect.width,
    (float)(max_ty - min_ty) / map->h * rect.height
  };
  DrawRectangleLinesEx(viewport, 2.0f, PURPLE);

  state->hovered = CheckCollisionPointRec(GetMousePosition(), rect);
}

void update_minimap_state(MinimapState *state, TileState *tile_state, const Map *map) {
  if (!state->hovered || !IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;

  Rectangle rect = minimap_rect();
  Vector2 mouse = GetMousePosition();
  int target_tx = (int)((mouse.x - rect.x) / rect.width * map->w);
  int target_ty = (int)((mouse.y - rect.y) / rect.height * map->h);

  tile_state->OFFSET_X = GetScreenWidth() / 2 - (target_tx - target_ty) * tile_state->TILE_W / 2;
  tile_state->OFFSET_Y = GetScreenHeight() / 2 - (target_tx + target_ty) * tile_state->TILE_H / 2;
}
