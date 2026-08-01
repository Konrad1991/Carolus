#ifndef SELECTION_TYPES_H
#define SELECTION_TYPES_H

// Selection
// ----------------------
typedef struct {
  unsigned int *ids; // Object.id of the currently selected figures
  int count;
  int capacity;
  bool dragging;
  Vector2 drag_start;
  int mansus_idx; // index into GameState.mansen, or -1 if no mansus selected
  bool barn_selected; // true if the selected mansus's barn (not just the yard) was clicked
  int field_mansus_idx; // index into GameState.mansen whose field was clicked, or -1 if none selected
  int field_idx; // index into that Mansus's fields array
} Selection;

#endif
