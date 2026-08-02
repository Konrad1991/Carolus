#ifndef PATHFINDING_TYPES_H
#define PATHFINDING_TYPES_H

// Path finding result
// ----------------------
typedef struct {
  int node;
  int priority;
} HeapEntry;

typedef struct {
  HeapEntry *arr;
  int size;
  int *pos; /* pos[node] = current index in arr, or -1 if not in the heap */
} BinaryHeap;

typedef struct {
  int *solution;
  int solution_len;
  int cost; /* -1 if no path was found */
} PathResult;

typedef struct {
  bool dragging;
  int start_tx;
  int start_ty;
  PathResult path;
} RoadDragState;

typedef struct {
  bool dragging;
  Vector2 drag_start; // screen space, like Selection.drag_start - not tile coords
} ClearForestDragState;

typedef struct {
  int* cost;
  int* cost_no_density;
  int len;
  int n_figures;
  bool active;
} FloodField;

typedef struct {
  FloodField *data;
  int count;
  int capacity;
} FloodFieldArray;

#endif
