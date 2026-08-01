#ifndef GAMESPEED_TYPES_H
#define GAMESPEED_TYPES_H

// Game speed
// ----------------------
#define GAME_SPEED_COUNT 5
typedef struct {
  int current_index;
  bool hovered[GAME_SPEED_COUNT];
} GameSpeedState;

#endif // !GAMESPEED_TYPES_H
