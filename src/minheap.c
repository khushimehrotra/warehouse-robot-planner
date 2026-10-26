/* src/minheap.c */
#include <stdlib.h>
#include "minheap.h"

static void swap_items(HeapItem *a, HeapItem *b) {
    HeapItem t = *a;
    *a = *b;
    *b = t;
}

int heap_init(MinHeap *h, int capacity) {
    if (capacity <= 0) capacity = 16;
    h->items = malloc(capacity * sizeof(HeapItem));
    if (h->items == NULL) return -1;
    h->size = 0;
    h->capacity = capacity;
    return 0;
}

int heap_push(MinHeap *h, int key, int value) {
    if (h->size == h->capacity) {
        int new_cap = h->capacity * 2;
        HeapItem *p = realloc(h->items, new_cap * sizeof(HeapItem));
        if (p == NULL) return -1;
        h->items = p;
        h->capacity = new_cap;
    }
    int i = h->size++;
    h->items[i].key = key;
    h->items[i].value = value;

    /* sift up: move the new item up while it is smaller than its parent */
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->items[parent].key <= h->items[i].key) break;
        swap_items(&h->items[parent], &h->items[i]);
        i = parent;
    }
    return 0;
}

int heap_pop(MinHeap *h, HeapItem *out) {
    if (h->size == 0) return -1;
    *out = h->items[0];                    /* the smallest is at the top */
    h->size--;
    h->items[0] = h->items[h->size];       /* move last item to the top */

    /* sift down: push it down while a child is smaller */
    int i = 0;
    for (;;) {
        int left = 2 * i + 1, right = left + 1, smallest = i;
        if (left  < h->size && h->items[left].key  < h->items[smallest].key) smallest = left;
        if (right < h->size && h->items[right].key < h->items[smallest].key) smallest = right;
        if (smallest == i) break;
        swap_items(&h->items[i], &h->items[smallest]);
        i = smallest;
    }
    return 0;
}

int heap_empty(const MinHeap *h) {
    return h->size == 0;
}

void heap_free(MinHeap *h) {
    free(h->items);
    h->items = NULL;
    h->size = h->capacity = 0;
}