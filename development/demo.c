#include <stdio.h>
#include "generic_array.h"
#include "typed_array.h"

typedef struct {
  int x, y;
} Point;

DEFINE_ARRAY(Point)  // generates PointArray, PointArray_push, PointArray_free

int main(void) {
  // --- Approach A: void* generic array ---
  Array points_generic = array_create(sizeof(Point));
  for (int i = 0; i < 5; i++) {
    Point p = {i, i * 2};
    array_push(&points_generic, &p);
  }
  for (int i = 0; i < points_generic.count; i++) {
    // no cast needed (void* converts implicitly in C), but nothing checks
    // that this array actually holds Points -- get it wrong and it's a
    // silent type-punning bug instead of a compiler error
    Point *p = array_get(&points_generic, i);
    printf("generic: (%d, %d)\n", p->x, p->y);
  }
  array_free(&points_generic);

  // --- Approach B: macro-generated typed array ---
  PointArray points_typed = {0};
  for (int i = 0; i < 5; i++) {
    PointArray_push(&points_typed, (Point){i, i * 2});
  }
  for (int i = 0; i < points_typed.count; i++) {
    // direct access, real Point*, compiler-checked
    Point p = points_typed.data[i];
    printf("typed:   (%d, %d)\n", p.x, p.y);
  }
  PointArray_free(&points_typed);

  return 0;
}
