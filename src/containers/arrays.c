#include "flood_fill/flood_fill.h"
#include "containers/arrays.h"
#include <stdlib.h>

static unsigned int next_object_id = 1;

unsigned int allocate_object_id(void) {
  return next_object_id++;
}

static void id_to_index_set(ObjectArray *arr, unsigned int id, int index) {
  if (id == 0) return;
  if ((int)id >= arr->id_to_index_capacity) {
    int new_capacity = arr->id_to_index_capacity == 0 ? 64 : arr->id_to_index_capacity;
    while ((int)id >= new_capacity) new_capacity *= 2;
    arr->id_to_index = realloc(arr->id_to_index, new_capacity * sizeof(int));
    for (int i = arr->id_to_index_capacity; i < new_capacity; i++) arr->id_to_index[i] = -1;
    arr->id_to_index_capacity = new_capacity;
  }
  arr->id_to_index[id] = index;
}

int object_array_find_by_id(const ObjectArray *arr, unsigned int id) {
  if (id == 0 || (int)id >= arr->id_to_index_capacity) return -1;
  return arr->id_to_index[id];
}

void object_array_push(ObjectArray *arr, Object obj) {
  if (arr->count == arr->capacity) {
    arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
    arr->data = realloc(arr->data, arr->capacity * sizeof(Object));
  }
  int index = arr->count++;
  arr->data[index] = obj;
  id_to_index_set(arr, obj.id, index);
}

void object_array_remove_swap(ObjectArray *arr, int index) {
  id_to_index_set(arr, arr->data[index].id, -1);
  arr->data[index] = arr->data[arr->count - 1];
  arr->count--;
  if (index < arr->count) id_to_index_set(arr, arr->data[index].id, index);
}

void object_array_free(ObjectArray *arr) {
  free(arr->data);
  free(arr->id_to_index);
  arr->data = NULL;
  arr->id_to_index = NULL;
  arr->count = 0;
  arr->capacity = 0;
  arr->id_to_index_capacity = 0;
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
