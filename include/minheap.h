/* include/minheap.h */
#ifndef MINHEAP_H
#define MINHEAP_H

typedef struct { int key; int value; } HeapItem;

typedef struct {
    HeapItem *items;
    int size;
    int capacity;
} MinHeap;

int  heap_init(MinHeap *h, int capacity);        /* 0 ok, -1 fail */
int  heap_push(MinHeap *h, int key, int value);  /* 0 ok, -1 fail */
int  heap_pop(MinHeap *h, HeapItem *out);        /* 0 ok, -1 if empty */
int  heap_empty(const MinHeap *h);
void heap_free(MinHeap *h);

#endif