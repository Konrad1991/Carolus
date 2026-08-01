#ifndef CONTAINERS_ARRAYS_H
#define CONTAINERS_ARRAYS_H

#include "types.h"

unsigned int allocate_object_id(void);
void object_array_push(ObjectArray *arr, Object obj);
void object_array_remove_swap(ObjectArray *arr, int index);
int object_array_find_by_id(const ObjectArray *arr, unsigned int id);
void object_array_free(ObjectArray *arr);

void mansus_array_push(MansusArray *arr, Mansus mansus);
void mansus_array_remove_swap(MansusArray *arr, int index);
void mansus_array_free(MansusArray *arr);

void field_array_push(FieldArray *arr, Field field);
void field_array_remove_swap(FieldArray *arr, int index);
void field_array_free(FieldArray *arr);

int flood_field_array_push(FloodFieldArray *arr, FloodField flood_field);
void flood_field_array_release(FloodFieldArray *ffa, int index);
void flood_field_array_free(FloodFieldArray *arr);

#endif
