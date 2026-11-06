#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "minheap.h"
#include "unionfind.h"

/* ---------- graph storage ---------- */
int graph_init(Graph *g, int n) {
    g->n = 0; g->m = 0; g->cap = 0;
    g->head = g->next = g->to = g->w = NULL;
    if (n <= 0) return -1;
    g->head = malloc((size_t)n * sizeof(int));
    if (g->head == NULL) return -3;
    for (int i = 0; i < n; i++) g->head[i] = -1;
    g->n = n;
    return 0;
}

int graph_add_edge(Graph *g, int u, int v, int w) {
    if (u < 0 || u >= g->n || v < 0 || v >= g->n) return -1;
    if (g->m == g->cap) {                              /* grow all three arrays */
        int nc = g->cap ? g->cap * 2 : 16;
        int *a = realloc(g->next, (size_t)nc * sizeof(int));
        if (a == NULL) return -3;
        g->next = a;
        a = realloc(g->to, (size_t)nc * sizeof(int));
        if (a == NULL) return -3;
        g->to = a;
        a = realloc(g->w, (size_t)nc * sizeof(int));
        if (a == NULL) return -3;
        g->w = a;
        g->cap = nc;
    }
    g->to[g->m] = v;
    g->w[g->m] = w;
    g->next[g->m] = g->head[u];                        /* push in front of u's list */
    g->head[u] = g->m;
    g->m++;
    return 0;
}

int graph_add_undirected(Graph *g, int u, int v, int w) {
    int rc = graph_add_edge(g, u, v, w);
    if (rc != 0) return rc;
    return graph_add_edge(g, v, u, w);
}

void graph_free(Graph *g) {
    free(g->head); free(g->next); free(g->to); free(g->w);
    g->head = g->next = g->to = g->w = NULL;
    g->n = g->m = g->cap = 0;
}

/* ---------- Kruskal: O(m log m) ---------- */
static int cmp_wedge(const void *a, const void *b) {
    const WEdge *x = a, *y = b;
    return (x->w > y->w) - (x->w < y->w);
}

int kruskal(int n, const WEdge *e, int m, WEdge *mst, int *mst_count, long long *total) {
    *mst_count = 0;
    *total = 0;
    if (n <= 0 || m < 0 || (m > 0 && e == NULL)) return -1;
    for (int i = 0; i < m; i++)
        if (e[i].u < 0 || e[i].u >= n || e[i].v < 0 || e[i].v >= n) return -1;

    WEdge *s = malloc((size_t)(m > 0 ? m : 1) * sizeof(WEdge));
    if (s == NULL) return -3;
    if (m > 0) memcpy(s, e, (size_t)m * sizeof(WEdge));
    qsort(s, (size_t)m, sizeof(WEdge), cmp_wedge);     /* cheapest first */

    UF uf;
    if (uf_init(&uf, n) != 0) { free(s); return -3; }
    for (int i = 0; i < m && *mst_count < n - 1; i++) {
        if (uf_union(&uf, s[i].u, s[i].v)) {           /* joins two separate pieces */
            mst[(*mst_count)++] = s[i];
            *total += s[i].w;
        }
    }
    uf_free(&uf);
    free(s);
    return 0;
}

/* ---------- Prim: O((n+m) log n) ----------
   best[v] = cheapest known edge connecting v to the tree.
   Uses goto for cleanup: a common C habit so every exit frees everything. */
int prim(const Graph *g, int start, int *parent, long long *total, int *visited) {
    *total = 0;
    *visited = 0;
    if (g == NULL || start < 0 || start >= g->n) return -1;

    int n = g->n, rc = 0, heap_ok = 0;
    int *best = malloc((size_t)n * sizeof(int));
    char *done = calloc((size_t)n, 1);
    MinHeap h = {NULL, 0, 0};
    HeapItem it = {0, 0};

    if (best == NULL || done == NULL) { rc = -3; goto cleanup; }
    if (heap_init(&h, 16) != 0) { rc = -3; goto cleanup; }
    heap_ok = 1;

    for (int i = 0; i < n; i++) { best[i] = INT_MAX; parent[i] = -1; }
    best[start] = 0;
    if (heap_push(&h, 0, start) != 0) { rc = -3; goto cleanup; }

    while (heap_pop(&h, &it) == 0) {
        int u = it.value;
        if (done[u]) continue;                         /* stale heap entry */
        done[u] = 1;
        (*visited)++;
        *total += best[u];
        for (int a = g->head[u]; a != -1; a = g->next[a]) {
            int v = g->to[a];
            if (!done[v] && g->w[a] < best[v]) {
                best[v] = g->w[a];
                parent[v] = u;
                if (heap_push(&h, best[v], v) != 0) { rc = -3; goto cleanup; }
            }
        }
    }
cleanup:
    if (heap_ok) heap_free(&h);
    free(best);
    free(done);
    return rc;
}

/* ---------- Dijkstra: O((n+m) log n), weights must be >= 0 ---------- */
int dijkstra(const Graph *g, int src, int *dist, int *prev) {
    if (g == NULL || src < 0 || src >= g->n) return -1;
    for (int a = 0; a < g->m; a++)
        if (g->w[a] < 0) return -2;

    for (int i = 0; i < g->n; i++) { dist[i] = DIST_INF; prev[i] = -1; }

    MinHeap h;
    if (heap_init(&h, 16) != 0) return -3;
    dist[src] = 0;
    int rc = heap_push(&h, 0, src) == 0 ? 0 : -3;

    HeapItem it;
    while (rc == 0 && heap_pop(&h, &it) == 0) {
        int u = it.value;
        if (it.key > dist[u]) continue;                /* stale entry: a better one was used */
        for (int a = g->head[u]; a != -1 && rc == 0; a = g->next[a]) {
            int v = g->to[a];
            long long nd = (long long)dist[u] + g->w[a];
            if (nd < dist[v]) {                        /* found a shorter way to v */
                dist[v] = (int)nd;
                prev[v] = u;
                if (heap_push(&h, dist[v], v) != 0) rc = -3;
            }
        }
    }
    heap_free(&h);
    return rc;
}

int path_extract(const int *prev, int src, int dst, int *path, int cap) {
    int len = 1, x = dst;
    while (x != src) {                                 /* walk backwards to count */
        x = prev[x];
        if (x == -1) return -1;                        /* never reached the start */
        len++;
    }
    if (len > cap) return -2;
    x = dst;
    for (int i = len - 1; i >= 0; i--) { path[i] = x; x = prev[x]; }
    return len;
}