#include "generic_array.h"
#include <stdlib.h>
#include <string.h>

Array array_create(size_t elem_size) {
  return (Array){.data = NULL, .elem_size = elem_size, .count = 0, .capacity = 0};
}

void array_push(Array *arr, const void *elem) {
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * arr->elem_size);
  }
  memcpy((char *)arr->data + arr->count * arr->elem_size, elem, arr->elem_size);
  arr->count++;
}

void *array_get(Array *arr, int index) {
  return (char *)arr->data + index * arr->elem_size;
}

void array_free(Array *arr) {
  free(arr->data);
  arr->data = NULL;
  arr->count = arr->capacity = 0;
}
