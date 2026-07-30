#ifndef TYPED_ARRAY_H
#define TYPED_ARRAY_H

#include <stdlib.h>

// DEFINE_ARRAY(T) generates a T##Array struct plus push/free functions for
// that exact type. Call it once per type at file scope. In exchange for
// that one extra line, .data is a real T* -- direct indexing, no casts,
// and the compiler rejects pushing the wrong type.
#define DEFINE_ARRAY(T)                                                     \
  typedef struct {                                                          \
    T *data;                                                                \
    int count;                                                              \
    int capacity;                                                           \
  } T##Array;                                                               \
                                                                              \
  static inline void T##Array_push(T##Array *arr, T elem) {                 \
    if (arr->count == arr->capacity) {                                      \
      arr->capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;           \
      arr->data = realloc(arr->data, arr->capacity * sizeof(T));            \
    }                                                                       \
    arr->data[arr->count++] = elem;                                        \
  }                                                                         \
                                                                              \
  static inline void T##Array_free(T##Array *arr) {                        \
    free(arr->data);                                                       \
    arr->data = NULL;                                                      \
    arr->count = arr->capacity = 0;                                        \
  }

#endif
