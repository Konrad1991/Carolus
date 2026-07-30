#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include "types.h"

int node_index(int x, int y, int w);
void node_to_xy(int node, int w, int *x, int *y);

BinaryHeap *heap_create(int num_nodes);
void heap_free(BinaryHeap *h);
bool heap_contains(BinaryHeap *h, int node);
void heap_swap(BinaryHeap *h, int i, int j);
void heap_sift_up(BinaryHeap *h, int i);
void heap_insert(BinaryHeap *h, int node, int priority);
void heap_decrease_key(BinaryHeap *h, int node, int new_priority);
HeapEntry heap_extract_min(BinaryHeap *h);

#endif
