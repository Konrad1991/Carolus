#ifndef GENERIC_ARRAY_H
#define GENERIC_ARRAY_H

#include <stddef.h>

// One implementation for every element type. Trade-off: elem_size is a
// runtime value, so nothing stops you from pushing the wrong struct into
// the wrong array or misreading array_get's return type.
typedef struct {
  void *data;
  size_t elem_size;
  int count;
  int capacity;
} Array;

Array array_create(size_t elem_size);
void array_push(Array *arr, const void *elem);
void *array_get(Array *arr, int index);
void array_free(Array *arr);

#endif
