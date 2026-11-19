#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "flow.h"

int flow_add_edge(Graph *g, int u, int v, int cap) {
    if (g == NULL || cap < 0) return -1;
    int rc = graph_add_edge(g, u, v, cap);             /* arc 2k   : forward  */
    if (rc != 0) return rc;
    return graph_add_edge(g, v, u, 0);                 /* arc 2k+1 : backward */
}

/* Edmonds-Karp: BFS finds the shortest augmenting path. O(V * E^2) */
int max_flow(Graph *g, int s, int t, long long *flow) {
    if (flow == NULL) return -1;
    *flow = 0;
    if (g == NULL || s < 0 || t < 0 || s >= g->n || t >= g->n || s == t) return -1;

    int n = g->n;
    int *pa = malloc((size_t)n * sizeof(int));         /* arc used to reach each node */
    int *q = malloc((size_t)n * sizeof(int));
    if (pa == NULL || q == NULL) { free(pa); free(q); return -3; }

    for (;;) {
        for (int i = 0; i < n; i++) pa[i] = -2;        /* -2 = not reached yet */
        pa[s] = -1;
        int h = 0, tl = 0;
        q[tl++] = s;
        while (h < tl && pa[t] == -2) {
            int u = q[h++];
            for (int a = g->head[u]; a != -1; a = g->next[a]) {
                int v = g->to[a];
                if (g->w[a] > 0 && pa[v] == -2) { pa[v] = a; q[tl++] = v; }
            }
        }
        if (pa[t] == -2) break;                        /* no augmenting path left */

        int bottleneck = INT_MAX;                      /* smallest spare capacity on the path */
        for (int v = t; v != s; v = g->to[pa[v] ^ 1])
            if (g->w[pa[v]] < bottleneck) bottleneck = g->w[pa[v]];
        for (int v = t; v != s; v = g->to[pa[v] ^ 1]) {
            g->w[pa[v]] -= bottleneck;                 /* use up forward capacity */
            g->w[pa[v] ^ 1] += bottleneck;             /* allow undoing it later */
        }
        *flow += bottleneck;
    }
    free(pa); free(q);
    return 0;
}

int flow_min_cut(const Graph *g, int s, char *side) {
    if (g == NULL || side == NULL || s < 0 || s >= g->n) return -1;
    int n = g->n;
    int *q = malloc((size_t)n * sizeof(int));
    if (q == NULL) return -3;
    memset(side, 0, (size_t)n);
    int h = 0, tl = 0;
    side[s] = 1;
    q[tl++] = s;
    while (h < tl) {                                   /* everything reachable in the residual graph */
        int u = q[h++];
        for (int a = g->head[u]; a != -1; a = g->next[a]) {
            int v = g->to[a];
            if (g->w[a] > 0 && !side[v]) { side[v] = 1; q[tl++] = v; }
        }
    }
    free(q);
    return 0;
}

/* ---------- bipartite matching ---------- */
typedef struct { const int *off, *adj; int *ml, *mr; char *seen; } BM;

static int try_aug(BM *b, int u) {
    for (int k = b->off[u]; k < b->off[u + 1]; k++) {
        int v = b->adj[k];
        if (b->seen[v]) continue;
        b->seen[v] = 1;
        if (b->mr[v] == -1 || try_aug(b, b->mr[v])) {  /* free, or its owner can move */
            b->mr[v] = u;
            b->ml[u] = v;
            return 1;
        }
    }
    return 0;
}

int bipartite_matching(int nl, int nr, const int *eu, const int *ev, int m,
                       int *match_l, int *match_r, int *size) {
    if (size == NULL || match_l == NULL || match_r == NULL) return -1;
    *size = 0;
    if (nl < 1 || nr < 1 || m < 0 || (m > 0 && (eu == NULL || ev == NULL))) return -1;
    for (int k = 0; k < m; k++)
        if (eu[k] < 0 || eu[k] >= nl || ev[k] < 0 || ev[k] >= nr) return -1;

    int *off = calloc((size_t)nl + 1, sizeof(int));    /* adjacency lists in compact form */
    int *adj = malloc((size_t)(m > 0 ? m : 1) * sizeof(int));
    int *pos = malloc((size_t)nl * sizeof(int));
    char *seen = malloc((size_t)nr);
    if (!off || !adj || !pos || !seen) { free(off); free(adj); free(pos); free(seen); return -3; }

    for (int k = 0; k < m; k++) off[eu[k] + 1]++;
    for (int i = 0; i < nl; i++) off[i + 1] += off[i];
    for (int i = 0; i < nl; i++) pos[i] = off[i];
    for (int k = 0; k < m; k++) adj[pos[eu[k]]++] = ev[k];

    for (int i = 0; i < nl; i++) match_l[i] = -1;
    for (int j = 0; j < nr; j++) match_r[j] = -1;
    BM b = {off, adj, match_l, match_r, seen};
    for (int u = 0; u < nl; u++) {
        memset(seen, 0, (size_t)nr);
        if (try_aug(&b, u)) (*size)++;
    }
    free(off); free(adj); free(pos); free(seen);
    return 0;
}

int matching_by_flow(int nl, int nr, const int *eu, const int *ev, int m, int *size) {
    if (size == NULL) return -1;
    *size = 0;
    if (nl < 1 || nr < 1 || m < 0 || (m > 0 && (eu == NULL || ev == NULL))) return -1;
    for (int k = 0; k < m; k++)
        if (eu[k] < 0 || eu[k] >= nl || ev[k] < 0 || ev[k] >= nr) return -1;

    Graph g;
    int s = nl + nr, t = s + 1;
    if (graph_init(&g, nl + nr + 2) != 0) return -3;
    int rc = 0;
    for (int u = 0; u < nl && rc == 0; u++) rc = flow_add_edge(&g, s, u, 1);
    for (int v = 0; v < nr && rc == 0; v++) rc = flow_add_edge(&g, nl + v, t, 1);
    for (int k = 0; k < m && rc == 0; k++) rc = flow_add_edge(&g, eu[k], nl + ev[k], 1);
    long long f = 0;
    if (rc == 0) rc = max_flow(&g, s, t, &f);
    if (rc == 0) *size = (int)f;
    graph_free(&g);
    return rc;
}