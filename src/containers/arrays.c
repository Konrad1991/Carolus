#include "flood_fill/flood_fill.h"
#include "containers/arrays.h"
#include <stdlib.h>

void object_array_push(ObjectArray *arr, Object obj) {
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(Object));
  }
  arr->data[arr->count++] = obj;
}

void object_array_remove_swap(ObjectArray *arr, int index) {
  arr->data[index] = arr->data[arr->count - 1];
  arr->count--;
}

void object_array_free(ObjectArray *arr) {
  free(arr->data);
  arr->data = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void mansus_array_push(MansusArray *arr, Mansus mansus) {
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(Mansus));
  }
  arr->data[arr->count++] = mansus;
}

void mansus_array_remove_swap(MansusArray *arr, int index) {
  arr->data[index] = arr->data[arr->count - 1];
  arr->count--;
}

void mansus_array_free(MansusArray *arr) {
  free(arr->data);
  arr->data = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

void field_array_push(FieldArray *arr, Field field) {
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(Field));
  }
  arr->data[arr->count++] = field;
}

void field_array_remove_swap(FieldArray *arr, int index) {
  arr->data[index] = arr->data[arr->count - 1];
  arr->count--;
}

void field_array_free(FieldArray *arr) {
  free(arr->data);
  arr->data = NULL;
  arr->count = 0;
  arr->capacity = 0;
}

int flood_field_array_push(FloodFieldArray *arr, FloodField flood_field) {
  for (int i = 0; i < arr->count; i++) {
    if (!arr->data[i].active) {
      arr->data[i] = flood_field;
      return i;
    }
  }
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(FloodField));
  }
  arr->data[arr->count] = flood_field;
  return arr->count++;
}

void flood_field_array_release(FloodFieldArray *ffa, int index) {
  if (!ffa->data[index].active) return;
  if (--ffa->data[index].n_figures > 0) return;
  flood_fill_free(&ffa->data[index]);
  ffa->data[index].active = false;
}

void flood_field_array_free(FloodFieldArray *arr) {
  for (int i = 0; i < arr->count; i++) {
    if (arr->data[i].active) flood_fill_free(&arr->data[i]);
  }
  free(arr->data);
  arr->data = NULL;
  arr->count = 0;
  arr->capacity = 0;
}
