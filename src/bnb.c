#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "bnb.h"
#include "vec.h"
#include "minheap.h"

/* One search node: robots 0..depth-1 are assigned. mask = tasks already used. */
typedef struct { int parent, depth, task, mask, cost, bound; } BNode;

typedef struct {
    BNode *pool;
    int size, cap;
    Vec v;               /* queue (FIFO) or stack (LIFO) of node indexes */
    int head;            /* read position, FIFO only */
    MinHeap h;           /* least-cost frontier, keyed by bound */
    BnbMode mode;
} BB;

/* Optimistic estimate: cost so far + cheapest free task for every unassigned robot. */
static int lower_bound(const int *c, int n, int depth, int mask, int base) {
    int lb = base;
    for (int r = depth; r < n; r++) {
        int m = INT_MAX;
        for (int j = 0; j < n; j++)
            if (!((mask >> j) & 1) && c[r * n + j] < m) m = c[r * n + j];
        lb += m;
    }
    return lb;
}

static int pool_add(BB *b, BNode nd) {                 /* index >= 0, or -3 / -4 */
    if (b->size >= BNB_NODE_LIMIT) return -4;
    if (b->size == b->cap) {
        int nc = b->cap ? b->cap * 2 : 64;
        if (nc > BNB_NODE_LIMIT) nc = BNB_NODE_LIMIT;
        BNode *p = realloc(b->pool, (size_t)nc * sizeof(BNode));
        if (p == NULL) return -3;
        b->pool = p;
        b->cap = nc;
    }
    b->pool[b->size] = nd;
    return b->size++;
}

/* The ONLY difference between the three strategies lives in these two functions. */
static int fr_push(BB *b, int idx, int key) {
    if (b->mode == BNB_LC) return heap_push(&b->h, key, idx) == 0 ? 0 : -3;
    return vec_push(&b->v, idx) == 0 ? 0 : -3;
}

static int fr_pop(BB *b, int *idx) {                   /* 0 ok, -1 empty */
    if (b->mode == BNB_LC) {
        HeapItem it;
        if (heap_pop(&b->h, &it) != 0) return -1;
        *idx = it.value;
        return 0;
    }
    if (b->mode == BNB_FIFO) {
        if (b->head >= b->v.size) return -1;
        *idx = b->v.data[b->head++];                   /* oldest first */
        return 0;
    }
    if (b->v.size == 0) return -1;
    *idx = b->v.data[--b->v.size];                     /* newest first */
    return 0;
}

const char *bnb_name(BnbMode mode) {
    return mode == BNB_FIFO ? "FIFO" : (mode == BNB_LIFO ? "LIFO" : "LC");
}

int assign_bnb(const int *c, int n, BnbMode mode,
               int *assign, long long *total, BnbStats *st) {
    BnbStats local;
    if (st == NULL) st = &local;
    st->generated = st->expanded = st->pruned = 0;
    if (total) *total = 0;
    if (c == NULL || assign == NULL || n < 1 || n > BNB_MAX_N ||
        (int)mode < BNB_FIFO || (int)mode > BNB_LC) return -1;
    for (int i = 0; i < n * n; i++)
        if (c[i] < 0 || c[i] > BNB_MAX_COST) return -1;

    BB b;
    memset(&b, 0, sizeof b);
    b.mode = mode;
    vec_init(&b.v);
    if (heap_init(&b.h, 16) != 0) return -3;

    int rc = 0, idx = 0, best_leaf = -1;
    long long best = LLONG_MAX;                        /* no complete solution yet */

    BNode root = {-1, 0, -1, 0, 0, lower_bound(c, n, 0, 0, 0)};
    int ri = pool_add(&b, root);
    if (ri < 0) { rc = ri; goto done; }
    rc = fr_push(&b, ri, root.bound);
    if (rc != 0) goto done;

    while (fr_pop(&b, &idx) == 0) {
        BNode u = b.pool[idx];                         /* copy: the pool may move */
        if (u.bound >= best) { st->pruned++; continue; }   /* cannot beat the best */
        st->expanded++;
        for (int j = 0; j < n; j++) {
            if ((u.mask >> j) & 1) continue;           /* task already taken */
            BNode ch;
            ch.parent = idx; ch.depth = u.depth + 1; ch.task = j;
            ch.mask = u.mask | (1 << j);
            ch.cost = u.cost + c[u.depth * n + j];
            st->generated++;
            if (ch.depth == n) {                       /* complete assignment */
                if (ch.cost < best) {
                    ch.bound = ch.cost;
                    int li = pool_add(&b, ch);
                    if (li < 0) { rc = li; goto done; }
                    best = ch.cost;
                    best_leaf = li;
                } else st->pruned++;
            } else {
                ch.bound = lower_bound(c, n, ch.depth, ch.mask, ch.cost);
                if (ch.bound >= best) { st->pruned++; continue; }
                int ci = pool_add(&b, ch);
                if (ci < 0) { rc = ci; goto done; }
                rc = fr_push(&b, ci, ch.bound);
                if (rc != 0) goto done;
            }
        }
    }

    for (int k = best_leaf; b.pool[k].parent != -1; k = b.pool[k].parent)
        assign[b.pool[k].depth - 1] = b.pool[k].task;  /* walk back to the root */
    if (total) *total = best;

done:
    free(b.pool);
    vec_free(&b.v);
    heap_free(&b.h);
    return rc;
}