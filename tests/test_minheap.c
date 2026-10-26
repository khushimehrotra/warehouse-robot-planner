/* tests/test_minheap.c */
#include <stdlib.h>
#include "test.h"
#include "minheap.h"

int main(void) {
    MinHeap h;
    HeapItem it;
    ASSERT_EQ(heap_init(&h, 2), 0);        /* small on purpose: forces growth */
    ASSERT_EQ(heap_pop(&h, &it), -1);      /* pop from empty fails */

    heap_push(&h, 5, 50);
    heap_push(&h, 1, 10);
    heap_push(&h, 3, 30);
    heap_push(&h, 4, 40);

    ASSERT_EQ(heap_pop(&h, &it), 0); ASSERT_EQ(it.key, 1); ASSERT_EQ(it.value, 10);
    ASSERT_EQ(heap_pop(&h, &it), 0); ASSERT_EQ(it.key, 3);
    ASSERT_EQ(heap_pop(&h, &it), 0); ASSERT_EQ(it.key, 4);
    ASSERT_EQ(heap_pop(&h, &it), 0); ASSERT_EQ(it.key, 5);
    ASSERT_EQ(heap_empty(&h), 1);

    /* random test: popping must always give sorted order */
    srand(42);
    for (int i = 0; i < 1000; i++) heap_push(&h, rand() % 500, i);
    int prev = -1;
    while (!heap_empty(&h)) {
        heap_pop(&h, &it);
        ASSERT_EQ(it.key >= prev, 1);
        prev = it.key;
    }

    heap_free(&h);
    TEST_SUMMARY();
}