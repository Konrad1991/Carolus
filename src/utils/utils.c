#include "utils.h"

int node_index(int x, int y, int w) {
  return y * w + x;
}

void node_to_xy(int node, int w, int *x, int *y) {
  *x = node % w;
  *y = node / w;
}

BinaryHeap *heap_create(int num_nodes) {
  BinaryHeap *h = malloc(sizeof(BinaryHeap));
  h->arr = malloc(sizeof(HeapEntry) * num_nodes);
  h->size = 0;
  h->pos = malloc(sizeof(int) * num_nodes);
  for (int i = 0; i < num_nodes; i++) h->pos[i] = -1;
  return h;
}

void heap_free(BinaryHeap *h) {
  free(h->arr);
  free(h->pos);
  free(h);
}

bool heap_contains(BinaryHeap *h, int node) {
  return h->pos[node] != -1;
}

void heap_swap(BinaryHeap *h, int i, int j) {
  HeapEntry tmp = h->arr[i];
  h->arr[i] = h->arr[j];
  h->arr[j] = tmp;
  h->pos[h->arr[i].node] = i;
  h->pos[h->arr[j].node] = j;
}

/* array is 0-indexed here (unlike the 1-indexed R version), so the
 * parent/child index arithmetic shifts: parent(i)=(i-1)/2, children=2i+1, 2i+2 */
void heap_sift_up(BinaryHeap *h, int i) {
  while (i > 0) {
    int parent = (i - 1) / 2;
    if (h->arr[i].priority >= h->arr[parent].priority) break;
    heap_swap(h, i, parent);
    i = parent;
  }
}

void heap_insert(BinaryHeap *h, int node, int priority) {
  int i = h->size++;
  h->arr[i].node = node;
  h->arr[i].priority = priority;
  h->pos[node] = i;
  heap_sift_up(h, i);
}

void heap_decrease_key(BinaryHeap *h, int node, int new_priority) {
  int i = h->pos[node];
  h->arr[i].priority = new_priority;
  heap_sift_up(h, i);
}

HeapEntry heap_extract_min(BinaryHeap *h) {
  HeapEntry res = h->arr[0];
  h->pos[res.node] = -1;
  h->size--;
  h->arr[0] = h->arr[h->size];
  if (h->size > 0) h->pos[h->arr[0].node] = 0;

  int i = 0;
  while (true) {
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int smallest = i;
    if (left < h->size && h->arr[left].priority < h->arr[smallest].priority) smallest = left;
    if (right < h->size && h->arr[right].priority < h->arr[smallest].priority) smallest = right;
    if (smallest == i) break;
    heap_swap(h, i, smallest);
    i = smallest;
  }
  return res;
}
