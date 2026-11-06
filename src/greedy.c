#include <stdlib.h>
#include <string.h>
#include "greedy.h"
#include "minheap.h"

static int validate(const Interval *iv, int n) {
    if (n < 0 || (n > 0 && iv == NULL)) return -1;
    for (int i = 0; i < n; i++)
        if (iv[i].finish <= iv[i].start) return -2;
    return 0;
}

/* qsort comparator: earlier finish first, ties by earlier start */
static int cmp_finish(const void *a, const void *b) {
    const Interval *x = a, *y = b;
    if (x->finish != y->finish) return (x->finish > y->finish) - (x->finish < y->finish);
    return (x->start > y->start) - (x->start < y->start);
}

/* Sorting O(n log n) + one pass O(n) = O(n log n) */
int activity_select(const Interval *iv, int n, int *chosen, int *count) {
    *count = 0;
    int rc = validate(iv, n);
    if (rc != 0) return rc;
    if (n == 0) return 0;

    Interval *s = malloc((size_t)n * sizeof(Interval));
    if (s == NULL) return -3;
    memcpy(s, iv, (size_t)n * sizeof(Interval));
    qsort(s, (size_t)n, sizeof(Interval), cmp_finish);

    int last_finish = 0;
    for (int i = 0; i < n; i++) {
        if (*count == 0 || s[i].start >= last_finish) {   /* fits after the last pick */
            chosen[(*count)++] = s[i].id;
            last_finish = s[i].finish;
        }
    }
    free(s);
    return 0;
}

/* An interval together with its original position */
typedef struct { Interval iv; int idx; } Tagged;

static int cmp_start(const void *a, const void *b) {
    const Tagged *x = a, *y = b;
    return (x->iv.start > y->iv.start) - (x->iv.start < y->iv.start);
}

/* Heap holds (finish time, room number). The top room frees up first. O(n log n) */
int interval_partition(const Interval *iv, int n, int *room_of, int *rooms) {
    *rooms = 0;
    int rc = validate(iv, n);
    if (rc != 0) return rc;
    if (n == 0) return 0;

    Tagged *s = malloc((size_t)n * sizeof(Tagged));
    if (s == NULL) return -3;
    for (int i = 0; i < n; i++) { s[i].iv = iv[i]; s[i].idx = i; }
    qsort(s, (size_t)n, sizeof(Tagged), cmp_start);

    MinHeap h;
    if (heap_init(&h, n) != 0) { free(s); return -3; }

    for (int i = 0; i < n; i++) {
        int room;
        if (!heap_empty(&h) && h.items[0].key <= s[i].iv.start) {
            HeapItem top;
            heap_pop(&h, &top);          /* reuse the room that is already free */
            room = top.value;
        } else {
            room = (*rooms)++;           /* open a new room */
        }
        room_of[s[i].idx] = room;
        if (heap_push(&h, s[i].iv.finish, room) != 0) { rc = -3; break; }
    }
    heap_free(&h);
    free(s);
    return rc;
}