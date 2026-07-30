#include "mode/mode.h"

void update_md(ModeState *ms) {
  for (Mode m = BUILD_LIVING_HOUSE; m < MODE_COUNT; m++) {
    if (ms->hovered[m] && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      ms->current = (ms->current == m) ? MOVEMENT : m;
      break;
    }
  }
  if (ms->current != MOVEMENT && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    ms->current = MOVEMENT;
  }
  bool rotatable = ms->current == BUILD_LIVING_HOUSE || ms->current == BUILD_BARN ||
    ms->current == BUILD_BRIDGE || ms->current == BUILD_MANSUS;
  if (rotatable && IsKeyPressed(KEY_R)) {
    ms->build_direction = (ms->build_direction + 1) % BUILDING_DIR_COUNT;
  }
}
