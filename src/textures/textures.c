#include "textures/textures.h"
#include <stdio.h>

static const int FIGURE_ACTION_FRAME_COUNT[FIGURE_ACTION_COUNT] = {
  [FIGURE_ACTION_STAND]  = 1,
  [FIGURE_ACTION_WALK]   = 6,
  [FIGURE_ACTION_SOW]    = 9,
  [FIGURE_ACTION_CHOP]   = 9,
  [FIGURE_ACTION_DIG]    = 9,
  [FIGURE_ACTION_HAMMER] = 9,
  [FIGURE_ACTION_MOW]    = 9,
  [FIGURE_ACTION_CARRY_STAND] = 1,
  [FIGURE_ACTION_CARRY_PICK]  = 9,
  [FIGURE_ACTION_CARRY_WALK]  = 8,
  [FIGURE_ACTION_CARRY_TRUNK_STAND] = 1,
  [FIGURE_ACTION_CARRY_TRUNK_WALK]  = 8,
};

int figure_action_frame_count(FigureAction action) {
  return FIGURE_ACTION_FRAME_COUNT[action];
}

static const char *FIGURE_ACTION_FOLDER[FIGURE_ACTION_COUNT] = {
  [FIGURE_ACTION_STAND]  = "standing",
  [FIGURE_ACTION_WALK]   = "walking",
  [FIGURE_ACTION_SOW]    = "sowing",
  [FIGURE_ACTION_CHOP]   = "chopping",
  [FIGURE_ACTION_DIG]    = "digging",
  [FIGURE_ACTION_HAMMER] = "hammering",
  [FIGURE_ACTION_MOW]    = "mowing",
  [FIGURE_ACTION_CARRY_STAND] = "wearing_basket",
  [FIGURE_ACTION_CARRY_PICK]  = "wearing_basket",
  [FIGURE_ACTION_CARRY_WALK]  = "wearing_basket",
  [FIGURE_ACTION_CARRY_TRUNK_STAND] = "trunk_carrying",
  [FIGURE_ACTION_CARRY_TRUNK_WALK]  = "trunk_carrying",
};

// animation subfolder name, only used when FIGURE_ACTION_FRAME_COUNT > 1;
// matches FIGURE_ACTION_FOLDER except where several actions share one pose folder (wearing_basket)
static const char *FIGURE_ACTION_ANIM_SUBDIR[FIGURE_ACTION_COUNT] = {
  [FIGURE_ACTION_WALK]   = "walking",
  [FIGURE_ACTION_SOW]    = "sowing",
  [FIGURE_ACTION_CHOP]   = "chopping",
  [FIGURE_ACTION_DIG]    = "digging",
  [FIGURE_ACTION_HAMMER] = "hammering",
  [FIGURE_ACTION_MOW]    = "mowing",
  [FIGURE_ACTION_CARRY_PICK] = "pick",
  [FIGURE_ACTION_CARRY_WALK] = "walking",
  [FIGURE_ACTION_CARRY_TRUNK_WALK] = "walking",
};

static const float FIGURE_ACTION_SCALE_CORRECTION[FIGURE_ACTION_COUNT] = {
  [FIGURE_ACTION_STAND]  = 0.9f,  // 25x60 px
  [FIGURE_ACTION_WALK]   = 0.9f,  // 26x61 px
  [FIGURE_ACTION_SOW]    = 0.93f, // 42x58 px
  [FIGURE_ACTION_CHOP]   = 0.9f,  // 52x60 px
  [FIGURE_ACTION_DIG]    = 0.9f,  // 54x60 px
  [FIGURE_ACTION_HAMMER] = 0.77f, // 41x72 px
  [FIGURE_ACTION_MOW]    = 0.9f,  // 43x60 px
  [FIGURE_ACTION_CARRY_STAND] = 0.9f, // 29x60 px
  [FIGURE_ACTION_CARRY_PICK]  = 0.9f, // 29x60 px
  [FIGURE_ACTION_CARRY_WALK]  = 0.9f, // 29x60 px
  [FIGURE_ACTION_CARRY_TRUNK_STAND] = 0.9f, // 49x66 px
  [FIGURE_ACTION_CARRY_TRUNK_WALK]  = 0.9f, // 49x66 px
};

static const char *FIGURE_DIR_COMPASS[FIGURE_DIR_COUNT] = {
  [FIGURE_DIR_FRONT]       = "south-west",
  [FIGURE_DIR_FRONT_RIGHT] = "south",
  [FIGURE_DIR_RIGHT]       = "south-east",
  [FIGURE_DIR_BACK_RIGHT]  = "east",
  [FIGURE_DIR_BACK]        = "north-east",
  [FIGURE_DIR_BACK_LEFT]   = "north",
  [FIGURE_DIR_LEFT]        = "north-west",
  [FIGURE_DIR_FRONT_LEFT]  = "west",
};

static void load_convention_figure_action(Texture_State *texture_state, const char *base_dir,
                                          int farmer_type, FigureAction action, float scale) {
  int frame_count = FIGURE_ACTION_FRAME_COUNT[action];
  bool animated = frame_count > 1;
  float action_scale = scale * FIGURE_ACTION_SCALE_CORRECTION[action];
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    for (int f = 0; f < frame_count; f++) {
      if (animated) {
        snprintf(path, sizeof(path), "%s%s/animations/%s/%s/frame_%03d.png",
                 base_dir, FIGURE_ACTION_FOLDER[action], FIGURE_ACTION_ANIM_SUBDIR[action], dir_name, f);
      } else {
        snprintf(path, sizeof(path), "%s%s/rotations/%s.png",
                 base_dir, FIGURE_ACTION_FOLDER[action], dir_name);
      }
      SpriteAsset *sprite = &texture_state->farmers[farmer_type][action][d][f];
      sprite->tex = LoadTexture(path);
      sprite->anchor = (Vector2){0.5f, 1.0f + 16.0f / (sprite->tex.height * action_scale)};
      sprite->scale = action_scale;
      SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_ox(Texture_State *texture_state, const char *ox_dir, float scale) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];

    snprintf(path, sizeof(path), "%sstanding/rotations/%s.png", ox_dir, dir_name);
    SpriteAsset *stand = &texture_state->ox_stand[d];
    stand->tex = LoadTexture(path);
    stand->anchor = (Vector2){0.5f, 1.0f + 16.0f / (stand->tex.height * scale)};
    stand->scale = scale;
    SetTextureFilter(stand->tex, TEXTURE_FILTER_POINT);

    for (int f = 0; f < OX_WALK_FRAMES; f++) {
      snprintf(path, sizeof(path), "%swalking/animations/walking/%s/frame_%03d.png", ox_dir, dir_name, f);
      SpriteAsset *walk = &texture_state->ox_walk[d][f];
      walk->tex = LoadTexture(path);
      walk->anchor = (Vector2){0.5f, 1.0f + 16.0f / (walk->tex.height * scale)};
      walk->scale = scale;
      SetTextureFilter(walk->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_plow(Texture_State *texture_state, const char *plow_dir, float scale) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];

    snprintf(path, sizeof(path), "%sstanding/rotations/%s.png", plow_dir, dir_name);
    SpriteAsset *stand = &texture_state->plow_stand[d];
    stand->tex = LoadTexture(path);
    stand->anchor = (Vector2){0.5f, 1.0f + 16.0f / (stand->tex.height * scale)};
    stand->scale = scale;
    SetTextureFilter(stand->tex, TEXTURE_FILTER_POINT);

    for (int f = 0; f < PLOW_WALK_FRAMES; f++) {
      snprintf(path, sizeof(path), "%swalking/animations/walking/%s/frame_%03d.png", plow_dir, dir_name, f);
      SpriteAsset *walk = &texture_state->plow_walk[d][f];
      walk->tex = LoadTexture(path);
      walk->anchor = (Vector2){0.5f, 1.0f + 16.0f / (walk->tex.height * scale)};
      walk->scale = scale;
      SetTextureFilter(walk->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_tile_variants(Texture2D dest[TILE_VARIANT_COUNT], const char *dir) {
  char path[512];
  for (int i = 0; i < TILE_VARIANT_COUNT; i++) {
    snprintf(path, sizeof(path), "%stile_%02d.png", dir, i);
    dest[i] = LoadTexture(path);
    SetTextureFilter(dest[i], TEXTURE_FILTER_POINT);
  }
}

static void load_water_flow(Texture2D dest[FIGURE_MAX_FRAMES], const char *dir) {
  char path[512];
  for (int i = 0; i < FIGURE_MAX_FRAMES; i++) {
    snprintf(path, sizeof(path), "%sframe_%03d.png", dir, i);
    dest[i] = LoadTexture(path);
    SetTextureFilter(dest[i], TEXTURE_FILTER_POINT);
  }
}

static const char *BUILDING_DIR_SUFFIX[BUILDING_DIR_COUNT] = {
  [BUILDING_DIR_SE] = "se",
  [BUILDING_DIR_SW] = "sw",
  [BUILDING_DIR_NW] = "nw",
  [BUILDING_DIR_NE] = "ne",
};

static const Vector2 HOUSE1_ANCHORS[BUILDING_DIR_COUNT] = {
  [BUILDING_DIR_SE] = {0.25f, 1.05f},
  [BUILDING_DIR_SW] = {0.75f, 1.05f},
  [BUILDING_DIR_NW] = {0.75f, 1.05f},
  [BUILDING_DIR_NE] = {0.25f, 1.05f},
};
static const float HOUSE1_SCALES[BUILDING_DIR_COUNT] = {1.3f, 1.3f, 1.3f, 1.3f};

static const Vector2 HOUSE2_ANCHORS[BUILDING_DIR_COUNT] = {
  {0.5f, 1.05f}, {0.5f, 1.05f}, {0.5f, 1.05f}, {0.5f, 1.05f},
};
static const float HOUSE2_SCALES[BUILDING_DIR_COUNT] = {0.85f, 0.85f, 0.85f, 0.85f};

static const Vector2 OAK_ANCHORS[BUILDING_DIR_COUNT] = {
  {0.5f, 1.05f}, {0.5f, 1.05f}, {0.5f, 1.05f}, {0.5f, 1.05f},
};
static const float OAK_SCALES[BUILDING_DIR_COUNT] = {1.8f, 1.8f, 1.8f, 1.8f};

static const Vector2 GRASS_TUFT_ANCHOR = {0.5f, 1.05f};
static const float GRASS_TUFT_SCALE = 0.38f;

static const Vector2 WHEAT_TUFT_ANCHOR = {0.5f, 1.05f};
static const float WHEAT_TUFT_SCALE = 0.4f;

static const Vector2 PUDDLE_ANCHOR = {0.5f, 1.0f};
static const float PUDDLE_SCALE = 1.0f;

static const Vector2 OAK_TRUNK_ANCHOR = {0.5f, 1.0f};
// AoE-style: a felled trunk fits on the single tile it stands on. The art
// itself (Images/oak_trunk/rotations/) is now trimmed to trunk-only size,
// not the old multi-tile-spanning sprite - starting guess, tune visually against TILE_W.
static const float OAK_TRUNK_SCALE = 1.0f;

static void load_grass_tuft(Texture_State *texture_state, const char *grass_tuft_dir, int season) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    snprintf(path, sizeof(path), "%srotations/%s.png", grass_tuft_dir, dir_name);
    SpriteAsset *idle = &texture_state->grass_tuft_idle[season][d];
    idle->tex = LoadTexture(path);
    idle->anchor = GRASS_TUFT_ANCHOR;
    idle->scale = GRASS_TUFT_SCALE;
    SetTextureFilter(idle->tex, TEXTURE_FILTER_POINT);

    int lean_dir = (d + FIGURE_DIR_COUNT / 2) % FIGURE_DIR_COUNT;
    for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
      snprintf(path, sizeof(path), "%sanimations/grass/%s/frame_%03d.png", grass_tuft_dir, FIGURE_DIR_COMPASS[lean_dir], f);
      SpriteAsset *sway = &texture_state->grass_tuft_sway[season][d][f];
      sway->tex = LoadTexture(path);
      sway->anchor = GRASS_TUFT_ANCHOR;
      sway->scale = GRASS_TUFT_SCALE;
      SetTextureFilter(sway->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_wheat_tuft_stage(Texture_State *texture_state, const char *wheat_dir, WheatStage stage) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];

    snprintf(path, sizeof(path), "%srotations/%s.png", wheat_dir, dir_name);
    SpriteAsset *idle = &texture_state->wheat_tuft_idle[stage][d];
    idle->tex = LoadTexture(path);
    idle->anchor = WHEAT_TUFT_ANCHOR;
    idle->scale = WHEAT_TUFT_SCALE;
    SetTextureFilter(idle->tex, TEXTURE_FILTER_POINT);

    int lean_dir = (d + FIGURE_DIR_COUNT / 2) % FIGURE_DIR_COUNT;
    if (d == FIGURE_DIR_BACK_RIGHT || d == FIGURE_DIR_FRONT_LEFT) lean_dir = d;
    for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
      snprintf(path, sizeof(path), "%sanimations/wheat/%s/frame_%03d.png", wheat_dir, FIGURE_DIR_COMPASS[lean_dir], f);
      SpriteAsset *sway = &texture_state->wheat_tuft_sway[stage][d][f];
      sway->tex = LoadTexture(path);
      sway->anchor = WHEAT_TUFT_ANCHOR;
      sway->scale = WHEAT_TUFT_SCALE;
      SetTextureFilter(sway->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_wheat_tuft_static_stage(Texture_State *texture_state, const char *wheat_dir, WheatStage stage) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    snprintf(path, sizeof(path), "%srotations/%s.png", wheat_dir, dir_name);
    SpriteAsset *idle = &texture_state->wheat_tuft_idle[stage][d];
    idle->tex = LoadTexture(path);
    idle->anchor = WHEAT_TUFT_ANCHOR;
    idle->scale = WHEAT_TUFT_SCALE;
    SetTextureFilter(idle->tex, TEXTURE_FILTER_POINT);
  }
}

static void load_wheat_tuft(Texture_State *texture_state, const char *wheat_young_dir, const char *wheat_middle_dir,
                            const char *wheat_large_green_dir, const char *wheat_ripe_dir,
                            const char *wheat_overripe_dir, const char *wheat_harvested_dir, const char *wheat_destroyed_dir) {
  load_wheat_tuft_stage(texture_state, wheat_young_dir, WHEAT_STAGE_YOUNG);
  load_wheat_tuft_stage(texture_state, wheat_middle_dir, WHEAT_STAGE_MIDDLE);
  load_wheat_tuft_stage(texture_state, wheat_large_green_dir, WHEAT_STAGE_LARGE_GREEN);
  load_wheat_tuft_stage(texture_state, wheat_ripe_dir, WHEAT_STAGE_RIPE);
  load_wheat_tuft_stage(texture_state, wheat_overripe_dir, WHEAT_STAGE_OVERRIPE);
  load_wheat_tuft_static_stage(texture_state, wheat_harvested_dir, WHEAT_STAGE_HARVESTED);
  load_wheat_tuft_static_stage(texture_state, wheat_destroyed_dir, WHEAT_STAGE_DESTROYED);
}

static const Vector2 BOUNDARY_STONE_ANCHOR = {0.5f, 1.05f};
static const float BOUNDARY_STONE_SCALE = 0.25f;

static void load_boundary_stone(Texture_State *texture_state, const char *boundary_stone_dir) {
  char path[512];
  snprintf(path, sizeof(path), "%sstone.png", boundary_stone_dir);
  SpriteAsset *sprite = &texture_state->boundary_stone;
  sprite->tex = LoadTexture(path);
  sprite->anchor = BOUNDARY_STONE_ANCHOR;
  sprite->scale = BOUNDARY_STONE_SCALE;
  SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
}

static void load_sprite_set(SpriteAsset *dest, int count, const char *const *filenames, const char *dir, Vector2 anchor, float scale) {
  char path[512];
  for (int i = 0; i < count; i++) {
    snprintf(path, sizeof(path), "%s%s", dir, filenames[i]);
    SpriteAsset *sprite = &dest[i];
    sprite->tex = LoadTexture(path);
    sprite->anchor = anchor;
    sprite->scale = scale;
    SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
  }
}

static const char *ROCK_FILENAMES[ROCK_VARIANT_COUNT] = {
  "limestone1.png", "limestone2.png", "limestone3.png", "limestone4.png",
  "sandstone1.png", "sandstone2.png", "sandstone3.png",
};
static const Vector2 ROCK_ANCHOR = {0.5f, 1.0f};
static const float ROCK_SCALE = 0.4f; // starting guess, tune visually against TILE_W

static const char *MOSS_FERNS_FILENAMES[MOSS_FERNS_VARIANT_COUNT] = {
  "moss_ferns1.png", "moss_ferns2.png", "moss_ferns3.png", "moss_ferns4.png",
};
static const Vector2 MOSS_FERNS_ANCHOR = {0.5f, 1.0f};
static const float MOSS_FERNS_SCALE = 0.5f; // starting guess, tune visually against TILE_W

static const char *MUSHROOMS_FILENAMES[MUSHROOMS_VARIANT_COUNT] = {
  "chanterelle1.png", "chanterelle2.png",
};
static const Vector2 MUSHROOMS_ANCHOR = {0.5f, 1.0f};
static const float MUSHROOMS_SCALE = 0.5f; // starting guess, tune visually against TILE_W

static const char *STRAWBERRY_FILENAMES[STRAWBERRY_VARIANT_COUNT] = {
  "strawberry1.png", "strawberry2.png", "strawberry3.png", "strawberry4.png",
};
static const Vector2 STRAWBERRY_ANCHOR = {0.5f, 1.0f};
static const float STRAWBERRY_SCALE = 0.5f; // starting guess, tune visually against TILE_W

static const char *CAIRN_FILENAMES[CAIRN_VARIANT_COUNT] = {
  "cairn1.png", "cairn2.png",
};
static const Vector2 CAIRN_ANCHOR = {0.5f, 1.0f};
static const float CAIRN_SCALE = 0.5f; // starting guess, tune visually against TILE_W

static const char *WOODY_DEBRIS_FILENAMES[WOODY_DEBRIS_VARIANT_COUNT] = {
  "debris1.png", "debris2.png", "debris3.png", "debris4.png",
};
static const Vector2 WOODY_DEBRIS_ANCHOR = {0.5f, 1.0f};
static const float WOODY_DEBRIS_SCALE = 0.4f; // starting guess, tune visually against TILE_W

static const Vector2 HIVE_ANCHOR = {0.5f, 1.0f};
static const float HIVE_SCALE = 0.6f; // starting guess, tune visually against TILE_W

static void load_hive(Texture_State *texture_state, const char *hive_dir) {
  char path[512];
  snprintf(path, sizeof(path), "%srotations/unknown.png", hive_dir);
  SpriteAsset *sprite = &texture_state->hive;
  sprite->tex = LoadTexture(path);
  sprite->anchor = HIVE_ANCHOR;
  sprite->scale = HIVE_SCALE;
  SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
}

static const Vector2 WELL_ANCHOR = {0.5f, 1.0f};
static const float WELL_SCALE = 1.0f; // starting guess, tune visually against TILE_W

static void load_well(Texture_State *texture_state, const char *well_dir) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    snprintf(path, sizeof(path), "%srotations/%s.png", well_dir, dir_name);
    SpriteAsset *sprite = &texture_state->well[d];
    sprite->tex = LoadTexture(path);
    sprite->anchor = WELL_ANCHOR;
    sprite->scale = WELL_SCALE;
    SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
  }
}

static const Vector2 WHEAT_SHEAF_ANCHOR = {0.5f, 1.05f};
static const float WHEAT_SHEAF_SCALE = 0.25f;

static void load_wheat_sheaf(Texture_State *texture_state, const char *wheat_sheaf_dir) {
  char path[512];
  snprintf(path, sizeof(path), "%ssheaf.png", wheat_sheaf_dir);
  SpriteAsset *sprite = &texture_state->wheat_sheaf;
  sprite->tex = LoadTexture(path);
  sprite->anchor = WHEAT_SHEAF_ANCHOR;
  sprite->scale = WHEAT_SHEAF_SCALE;
  SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
}

static const Vector2 CURSOR_ANCHOR = {4.5f / 56.0f, 0.0f};
static const float CURSOR_SCALE = 0.5f;

static void load_signs(Texture_State *texture_state, const char *signs_dir) {
  char path[512];

  snprintf(path, sizeof(path), "%scursor.png", signs_dir);
  SpriteAsset *cursor = &texture_state->cursor;
  cursor->tex = LoadTexture(path);
  cursor->anchor = CURSOR_ANCHOR;
  cursor->scale = CURSOR_SCALE;
  SetTextureFilter(cursor->tex, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%smansus_area.png", signs_dir);
  texture_state->mansus_area_icon = LoadTexture(path);
  SetTextureFilter(texture_state->mansus_area_icon, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%sremove.png", signs_dir);
  texture_state->remove_icon = LoadTexture(path);
  SetTextureFilter(texture_state->remove_icon, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%saxe.png", signs_dir);
  SpriteAsset *axe_cursor = &texture_state->axe_cursor;
  axe_cursor->tex = LoadTexture(path);
  axe_cursor->anchor = CURSOR_ANCHOR;
  axe_cursor->scale = CURSOR_SCALE;
  SetTextureFilter(axe_cursor->tex, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%splow.png", signs_dir);
  SpriteAsset *plow_cursor = &texture_state->plow_cursor;
  plow_cursor->tex = LoadTexture(path);
  plow_cursor->anchor = CURSOR_ANCHOR;
  plow_cursor->scale = CURSOR_SCALE;
  SetTextureFilter(plow_cursor->tex, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%sscythe.png", signs_dir);
  SpriteAsset *scythe_cursor = &texture_state->scythe_cursor;
  scythe_cursor->tex = LoadTexture(path);
  scythe_cursor->anchor = CURSOR_ANCHOR;
  scythe_cursor->scale = CURSOR_SCALE;
  SetTextureFilter(scythe_cursor->tex, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%sspade.png", signs_dir);
  SpriteAsset *dig_cursor = &texture_state->dig_cursor;
  dig_cursor->tex = LoadTexture(path);
  dig_cursor->anchor = CURSOR_ANCHOR;
  dig_cursor->scale = CURSOR_SCALE;
  SetTextureFilter(dig_cursor->tex, TEXTURE_FILTER_POINT);

  snprintf(path, sizeof(path), "%sbasket.png", signs_dir);
  SpriteAsset *sow_cursor = &texture_state->sow_cursor;
  sow_cursor->tex = LoadTexture(path);
  sow_cursor->anchor = CURSOR_ANCHOR;
  sow_cursor->scale = CURSOR_SCALE;
  SetTextureFilter(sow_cursor->tex, TEXTURE_FILTER_POINT);
}

static void load_puddle(Texture_State *texture_state, const char *puddle_dir) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    snprintf(path, sizeof(path), "%srotations/%s.png", puddle_dir, dir_name);
    SpriteAsset *sprite = &texture_state->puddle[d];
    sprite->tex = LoadTexture(path);
    sprite->anchor = PUDDLE_ANCHOR;
    sprite->scale = PUDDLE_SCALE;
    SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
  }
}

static void load_oak_trunk(Texture_State *texture_state, const char *oak_trunk_dir) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    snprintf(path, sizeof(path), "%srotations/%s.png", oak_trunk_dir, dir_name);
    SpriteAsset *sprite = &texture_state->oak_trunk[d];
    sprite->tex = LoadTexture(path);
    sprite->anchor = OAK_TRUNK_ANCHOR;
    sprite->scale = OAK_TRUNK_SCALE;
    SetTextureFilter(sprite->tex, TEXTURE_FILTER_POINT);
  }
}

static void load_cloud(Texture_State *texture_state, const char *cloud_dir) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
      snprintf(path, sizeof(path), "%sanimations/cloud/%s/frame_%03d.png", cloud_dir, dir_name, f);
      SpriteAsset *drift = &texture_state->cloud_drift[d][f];
      drift->tex = LoadTexture(path);
      drift->anchor = (Vector2){0.5f, 0.5f};
      drift->scale = 3.0f;
      SetTextureFilter(drift->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_oak_sway(Texture_State *texture_state, const char *oak_dir, int season) {
  char path[512];
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    const char *dir_name = FIGURE_DIR_COMPASS[d];
    for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
      snprintf(path, sizeof(path), "%sanimations/oak/%s/frame_%03d.png", oak_dir, dir_name, f);
      SpriteAsset *sway = &texture_state->oak_sway[season][d][f];
      sway->tex = LoadTexture(path);
      sway->anchor = OAK_ANCHORS[0];
      sway->scale = OAK_SCALES[0];
      SetTextureFilter(sway->tex, TEXTURE_FILTER_POINT);
    }
  }
}

static void load_building_variants(SpriteAsset dest[BUILDING_DIR_COUNT], const char *dir,
                                   const char *suffixes[BUILDING_DIR_COUNT],
                                   const Vector2 anchors[BUILDING_DIR_COUNT],
                                   const float scales[BUILDING_DIR_COUNT]) {
  char path[512];
  for (int d = 0; d < BUILDING_DIR_COUNT; d++) {
    snprintf(path, sizeof(path), "%s%s.png", dir, suffixes[d]);
    dest[d].tex = LoadTexture(path);
    dest[d].anchor = anchors[d];
    dest[d].scale = scales[d];
    SetTextureFilter(dest[d].tex, TEXTURE_FILTER_POINT);
  }
}

#define IMG_PATH(var, suffix) \
  char var[300]; \
  snprintf(var, sizeof(var), "%s/" suffix, images_dir)

void init_texture_state(Texture_State* texture_state, const char* images_dir) {
  IMG_PATH(grass_flat_winter_path, "meadow_winter/flat/");
  IMG_PATH(grass_edge_winter_path, "meadow_winter/edge/");
  IMG_PATH(grass_flat_spring_path, "meadow_spring/flat/");
  IMG_PATH(grass_edge_spring_path, "meadow_spring/edge/");
  IMG_PATH(grass_flat_summer_path, "meadow_summer/flat/");
  IMG_PATH(grass_edge_summer_path, "meadow_spring/edge/");
  IMG_PATH(grass_flat_autumn_path, "meadow_autumn/flat/");
  IMG_PATH(grass_edge_autumn_path, "meadow_autumn/edge/");

  IMG_PATH(road_flat_path, "road/flat/");
  IMG_PATH(road_high_path, "road/edge/");
  IMG_PATH(water_flat_path, "water/");
  IMG_PATH(water_flow_path, "water_flow/");
  IMG_PATH(house1_dir, "living_house/");
  IMG_PATH(house2_dir, "barn/");
  IMG_PATH(bridge_path, "bridge/");
  IMG_PATH(swamp_flat_path, "swamp/flat/");
  IMG_PATH(swamp_edge_path, "swamp/edge/");
  IMG_PATH(farmer_actions_dir1, "farmer1/");
  IMG_PATH(farmer_actions_dir2, "farmer2/");
  IMG_PATH(ox_dir, "ox/");
  IMG_PATH(plow_dir, "plow/");

  IMG_PATH(oak_winter, "oak_winter/");
  IMG_PATH(oak_spring, "oak_spring/");
  IMG_PATH(oak_summer, "oak_summer/");
  IMG_PATH(oak_autumn, "oak_autumn/");
  IMG_PATH(oak_trunk_dir, "oak_trunk/");

  IMG_PATH(wheat_young_dir, "wheat_young/");
  IMG_PATH(wheat_middle_dir, "wheat_middle/");
  IMG_PATH(wheat_large_green_dir, "wheat_large_green/");
  IMG_PATH(wheat_ripe_dir, "wheat_ripe/");
  IMG_PATH(wheat_overripe_dir, "wheat_overripe/");
  IMG_PATH(wheat_harvested_dir, "wheat_harvested/");
  IMG_PATH(wheat_destroyed_dir, "wheat_destroyed/");

  IMG_PATH(grass_tuft_winter_dir, "grass_winter/");
  IMG_PATH(grass_tuft_spring_dir, "grass_spring/");
  IMG_PATH(grass_tuft_summer_dir, "grass_summer/");
  IMG_PATH(grass_tuft_autumn_dir, "grass_autumn/");

  IMG_PATH(earth_path, "earth/");
  IMG_PATH(arable_land_path, "arable_land/");
  IMG_PATH(cloud_dir, "cloud/");
  IMG_PATH(puddle_dir, "puddle/");
  IMG_PATH(boundary_stone_dir, "boundary_stone/");
  IMG_PATH(rocks_dir, "rocks/");
  IMG_PATH(moss_ferns_dir, "moss_and_ferns/");
  IMG_PATH(mushrooms_dir, "mushrooms_chanterelle/");
  IMG_PATH(strawberry_dir, "wild_strawberry/");
  IMG_PATH(cairns_dir, "cairns/");
  IMG_PATH(woody_debris_dir, "coarse_woody_debris/");
  IMG_PATH(hive_dir, "hive/");
  IMG_PATH(well_dir, "well/");
  IMG_PATH(wheat_sheaf_dir, "wheat_sheaf/");
  IMG_PATH(signs_dir, "signs/");

  const float scale_figure[FARMER_TYPE_COUNT] = {0.7, 0.9};

  const char *grass_flat_per_season[SEASON_COUNT] = {
    grass_flat_winter_path,
    grass_flat_spring_path,
    grass_flat_summer_path,
    grass_flat_autumn_path
  };
  const char *grass_edge_per_season[SEASON_COUNT] = {
    grass_edge_winter_path,
    grass_edge_spring_path,
    grass_edge_summer_path,
    grass_edge_autumn_path
  };
  const char *oak_per_season[SEASON_COUNT] = {
    oak_winter,
    oak_spring,
    oak_summer,
    oak_autumn
  };
  const char* grass_tuft_per_season[SEASON_COUNT] = {
    grass_tuft_winter_dir,
    grass_tuft_spring_dir,
    grass_tuft_summer_dir,
    grass_tuft_autumn_dir
  };
  for (int i = 0; i < SEASON_COUNT; i++) {
    load_tile_variants(texture_state->grass_flat[i], grass_flat_per_season[i]);
    load_tile_variants(texture_state->grass_edge[i], grass_edge_per_season[i]);
    load_building_variants(texture_state->oak[i], oak_per_season[i], BUILDING_DIR_SUFFIX, OAK_ANCHORS, OAK_SCALES);
    load_oak_sway(texture_state, oak_per_season[i], i);
    load_grass_tuft(texture_state, grass_tuft_per_season[i], i);
  }
  load_tile_variants(texture_state->road_flat, road_flat_path);
  load_tile_variants(texture_state->road_high, road_high_path);
  load_tile_variants(texture_state->water_flat, water_flat_path);
  load_water_flow(texture_state->water_flow, water_flow_path);
  load_tile_variants(texture_state->bridge, bridge_path);
  load_tile_variants(texture_state->swamp_flat, swamp_flat_path);
  load_tile_variants(texture_state->swamp_edge, swamp_edge_path);
  load_tile_variants(texture_state->earth, earth_path);
  load_tile_variants(texture_state->arable_land, arable_land_path);

  load_building_variants(texture_state->house1, house1_dir, BUILDING_DIR_SUFFIX, HOUSE1_ANCHORS, HOUSE1_SCALES);
  load_building_variants(texture_state->house2, house2_dir, BUILDING_DIR_SUFFIX, HOUSE2_ANCHORS, HOUSE2_SCALES);

  load_wheat_tuft(texture_state, wheat_young_dir, wheat_middle_dir, wheat_large_green_dir, wheat_ripe_dir,
                  wheat_overripe_dir, wheat_harvested_dir, wheat_destroyed_dir);
  load_cloud(texture_state, cloud_dir);
  load_puddle(texture_state, puddle_dir);
  load_oak_trunk(texture_state, oak_trunk_dir);
  load_boundary_stone(texture_state, boundary_stone_dir);
  load_sprite_set(texture_state->rock, ROCK_VARIANT_COUNT, ROCK_FILENAMES, rocks_dir, ROCK_ANCHOR, ROCK_SCALE);
  load_sprite_set(texture_state->moss_ferns, MOSS_FERNS_VARIANT_COUNT, MOSS_FERNS_FILENAMES, moss_ferns_dir, MOSS_FERNS_ANCHOR, MOSS_FERNS_SCALE);
  load_sprite_set(texture_state->mushrooms, MUSHROOMS_VARIANT_COUNT, MUSHROOMS_FILENAMES, mushrooms_dir, MUSHROOMS_ANCHOR, MUSHROOMS_SCALE);
  load_sprite_set(texture_state->strawberry, STRAWBERRY_VARIANT_COUNT, STRAWBERRY_FILENAMES, strawberry_dir, STRAWBERRY_ANCHOR, STRAWBERRY_SCALE);
  load_sprite_set(texture_state->cairn, CAIRN_VARIANT_COUNT, CAIRN_FILENAMES, cairns_dir, CAIRN_ANCHOR, CAIRN_SCALE);
  load_sprite_set(texture_state->woody_debris, WOODY_DEBRIS_VARIANT_COUNT, WOODY_DEBRIS_FILENAMES, woody_debris_dir, WOODY_DEBRIS_ANCHOR, WOODY_DEBRIS_SCALE);
  load_hive(texture_state, hive_dir);
  load_well(texture_state, well_dir);
  load_wheat_sheaf(texture_state, wheat_sheaf_dir);
  load_signs(texture_state, signs_dir);

  const char *farmer_dirs[FARMER_TYPE_COUNT] = { farmer_actions_dir1,  farmer_actions_dir2};
  for (int t = 0; t < FARMER_TYPE_COUNT; t++) {
    for (int a = 0; a < FIGURE_ACTION_COUNT; a++) {
      load_convention_figure_action(texture_state, farmer_dirs[t], t, (FigureAction)a, scale_figure[t]);
    }
  }

  load_ox(texture_state, ox_dir, 1.0f);
  load_plow(texture_state, plow_dir, 1.0f);
}
#undef IMG_PATH

void free_texture_state(Texture_State* texture_state) {
  for (int s = 0; s < SEASON_COUNT; s++) {
    for (int i = 0; i < TILE_VARIANT_COUNT; i++) {
      UnloadTexture(texture_state->grass_flat[s][i]);
      UnloadTexture(texture_state->grass_edge[s][i]);
    }
    for (int d = 0; d < BUILDING_DIR_COUNT; d++) {
      UnloadTexture(texture_state->oak[s][d].tex);
      UnloadTexture(texture_state->grass_tuft_idle[s][d].tex);
    }
    for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
      for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
        UnloadTexture(texture_state->oak_sway[s][d][f].tex);
        UnloadTexture(texture_state->grass_tuft_sway[s][d][f].tex);
      }
    }
  }
  for (int i = 0; i < TILE_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->road_flat[i]);
    UnloadTexture(texture_state->road_high[i]);
    UnloadTexture(texture_state->water_flat[i]);
    UnloadTexture(texture_state->bridge[i]);
    UnloadTexture(texture_state->earth[i]);
    UnloadTexture(texture_state->arable_land[i]);
    UnloadTexture(texture_state->swamp_flat[i]);
    UnloadTexture(texture_state->swamp_edge[i]);
  }
  for (int i = 0; i < FIGURE_MAX_FRAMES; i++) {
    UnloadTexture(texture_state->water_flow[i]);
  }
  for (int d = 0; d < BUILDING_DIR_COUNT; d++) {
    UnloadTexture(texture_state->house1[d].tex);
    UnloadTexture(texture_state->house2[d].tex);
  }
  UnloadTexture(texture_state->boundary_stone.tex);
  for (int i = 0; i < ROCK_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->rock[i].tex);
  }
  for (int i = 0; i < MOSS_FERNS_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->moss_ferns[i].tex);
  }
  for (int i = 0; i < MUSHROOMS_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->mushrooms[i].tex);
  }
  for (int i = 0; i < STRAWBERRY_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->strawberry[i].tex);
  }
  for (int i = 0; i < CAIRN_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->cairn[i].tex);
  }
  for (int i = 0; i < WOODY_DEBRIS_VARIANT_COUNT; i++) {
    UnloadTexture(texture_state->woody_debris[i].tex);
  }
  UnloadTexture(texture_state->hive.tex);
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    UnloadTexture(texture_state->well[d].tex);
  }
  UnloadTexture(texture_state->wheat_sheaf.tex);
  UnloadTexture(texture_state->cursor.tex);
  UnloadTexture(texture_state->mansus_area_icon);
  UnloadTexture(texture_state->remove_icon);
  UnloadTexture(texture_state->axe_cursor.tex);
  UnloadTexture(texture_state->plow_cursor.tex);
  UnloadTexture(texture_state->scythe_cursor.tex);
  UnloadTexture(texture_state->dig_cursor.tex);
  UnloadTexture(texture_state->sow_cursor.tex);
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    UnloadTexture(texture_state->puddle[d].tex);
    UnloadTexture(texture_state->oak_trunk[d].tex);
    for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
      UnloadTexture(texture_state->cloud_drift[d][f].tex);
    }
  }
  for (int s = 0; s < WHEAT_STAGE_COUNT; s++) {
    for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
      UnloadTexture(texture_state->wheat_tuft_idle[s][d].tex);
      for (int f = 0; f < FIGURE_MAX_FRAMES; f++) {
        UnloadTexture(texture_state->wheat_tuft_sway[s][d][f].tex);
      }
    }
  }
  for (int t = 0; t < FARMER_TYPE_COUNT; t++) {
    for (int a = 0; a < FIGURE_ACTION_COUNT; a++) {
      for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
        int frame_count = FIGURE_ACTION_FRAME_COUNT[a];
        for (int f = 0; f < frame_count; f++) {
          UnloadTexture(texture_state->farmers[t][a][d][f].tex);
        }
      }
    }
  }
  for (int d = 0; d < FIGURE_DIR_COUNT; d++) {
    UnloadTexture(texture_state->ox_stand[d].tex);
    for (int f = 0; f < OX_WALK_FRAMES; f++) UnloadTexture(texture_state->ox_walk[d][f].tex);
    UnloadTexture(texture_state->plow_stand[d].tex);
    for (int f = 0; f < PLOW_WALK_FRAMES; f++) UnloadTexture(texture_state->plow_walk[d][f].tex);
  }
}
