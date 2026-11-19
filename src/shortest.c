#include <stddef.h>
#include "shortest.h"

/* Bellman-Ford: O(n * m) */
int bellman_ford(const Graph *g, int src, int *dist, int *prev) {
    if (g == NULL || dist == NULL || prev == NULL || src < 0 || src >= g->n) return -1;
    int n = g->n;
    for (int i = 0; i < n; i++) { dist[i] = DIST_INF; prev[i] = -1; }
    dist[src] = 0;

    for (int pass = 1; pass < n; pass++) {              /* at most n-1 rounds */
        int changed = 0;
        for (int u = 0; u < n; u++) {
            if (dist[u] == DIST_INF) continue;          /* not reached yet */
            for (int a = g->head[u]; a != -1; a = g->next[a]) {
                int v = g->to[a];
                long long nd = (long long)dist[u] + g->w[a];
                if (nd < dist[v]) { dist[v] = (int)nd; prev[v] = u; changed = 1; }
            }
        }
        if (!changed) break;                            /* already settled */
    }
    for (int u = 0; u < n; u++) {                       /* one extra pass: any gain = cycle */
        if (dist[u] == DIST_INF) continue;
        for (int a = g->head[u]; a != -1; a = g->next[a])
            if ((long long)dist[u] + g->w[a] < dist[g->to[a]]) return 1;
    }
    return 0;
}

/* Floyd-Warshall: O(n^3) */
int floyd_warshall(const Graph *g, long long *d, int *next) {
    if (g == NULL || d == NULL) return -1;
    size_t n = (size_t)g->n;
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) {
            d[i * n + j] = (i == j) ? 0 : FW_INF;
            if (next) next[i * n + j] = (i == j) ? (int)i : -1;
        }
    for (size_t u = 0; u < n; u++)                      /* direct edges (keep the cheapest) */
        for (int a = g->head[u]; a != -1; a = g->next[a]) {
            size_t v = (size_t)g->to[a];
            if (g->w[a] < d[u * n + v]) {               /* a negative self-loop lands here too */
                d[u * n + v] = g->w[a];
                if (next) next[u * n + v] = (int)v;
            }
        }

    for (size_t k = 0; k < n; k++) {                    /* k MUST be the outer loop */
        for (size_t i = 0; i < n; i++) {
            if (d[i * n + k] >= FW_INF) continue;
            for (size_t j = 0; j < n; j++) {
                if (d[k * n + j] >= FW_INF) continue;
                long long t = d[i * n + k] + d[k * n + j];
                if (t < d[i * n + j]) {
                    d[i * n + j] = t;
                    if (next) next[i * n + j] = next[i * n + k];
                }
            }
        }
        for (size_t i = 0; i < n; i++)                  /* negative cycle? stop before overflow */
            if (d[i * n + i] < 0) return 1;
    }
    return 0;
}

int fw_path(const int *next, int n, int i, int j, int *path, int cap) {
    if (next[(size_t)i * n + j] == -1) return -1;
    int len = 0, x = i;
    for (;;) {
        if (len >= cap) return -2;
        path[len++] = x;
        if (x == j) break;
        x = next[(size_t)x * n + j];
    }
    return len;
}