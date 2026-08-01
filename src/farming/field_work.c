#include "containers/arrays.h"
#include "units/units.h"
#include "farming/field_work.h"

static bool is_idle(const Object* fig) {
  const bool check = fig->figure.flood_field_idx < 0 &&
    !fig->figure.direct_walking &&
    fig->figure.harvest_route.phase == HARVEST_PHASE_NONE &&
    fig->figure.sow_route.phase == SOW_PHASE_NONE &&
    fig->figure.dig_route.phase == DIG_PHASE_NONE &&
    fig->figure.wood_route.phase == WOOD_PHASE_NONE;
  return check;
}

static bool requires_digging(const Field* field, SeasonState* season_state) {
  const bool needs_digging = field->field_condition == GRASS || field->field_condition == FALLOW;
  const bool correct_month = season_state->month == 2 || season_state->month == 3 ?
    true : false;
  return needs_digging && correct_month;
}
static bool requires_sowing(const Field* field, SeasonState* season_state) {
  const bool is_grass = field->field_condition == PLOWED;
  const bool correct_month = season_state->month == 2 || season_state->month == 3 ?
    true : false;
  return is_grass && correct_month;
}

static bool field_has_enough_ripe_tufts(const Field* field, const ObjectArray* objects) {
  int counter = 0;
  for (int i = 0; i < objects->count; i++) {
    const Object* o = &objects->data[i];
    if (o->kind != OBJECT_WHEAT_TUFT) continue;
    if (o->tx < field->corners_field[0][0] || o->tx > field->corners_field[1][0]) continue;
    if (o->ty < field->corners_field[0][1] || o->ty > field->corners_field[1][1]) continue;
    if (o->wheat.wheat_stage == WHEAT_STAGE_RIPE || o->wheat.wheat_stage == WHEAT_STAGE_OVERRIPE) counter++;
  }
  return counter > 5;
}

static int which_field(const Mansus* m, const SeasonState* season_state) {
  const int current_year = season_state->year;
  for (int i = 0; i < m->fields.count; i++) {
    const int last_cultivated_in = m->fields.data[i].cultivated_last_in_year;
    if (current_year - last_cultivated_in == 0) return i;
    if ((current_year - last_cultivated_in) >= 2) return i;
    if (last_cultivated_in == -1) return i; // was never cultivated
  }
  return -1;
}

void culitvate_fields(ObjectArray* objects, Map* map, FloodFieldArray* flood_field_state,
          GameState* game_state, SeasonState* season_state) {
  for (int i = 0; i < game_state->mansen.count; i++) {
    Mansus* m = &game_state->mansen.data[i];
    const unsigned int farmer_id = m->farmer_object_id;
    int idx = object_array_find_by_id(objects, farmer_id);
    if (idx == -1) continue;
    Object* fig = &objects->data[idx];
    if (m->fields.count == 0) continue;
    if (!is_idle(fig)) continue;
    const int wf = which_field(m, season_state);
    if (wf == -1) continue;
    Field *field = &m->fields.data[wf];
    if (requires_digging(field, season_state)) {
      start_dig_route(fig, map, flood_field_state, game_state, field);
      field->cultivated_last_in_year = season_state->year;
    }
    if (requires_sowing(field, season_state)) {
      start_sow_route(fig, map, flood_field_state, game_state, m, field);
    }
    if (field_has_enough_ripe_tufts(field, objects)) {
      start_harvest_route(fig, map, flood_field_state, game_state, field);
    }
  }
}
