#include <stdlib.h>
#include "test.h"
#include "graph.h"
#include "shortest.h"

#define MAXN 8

/* Assumes no negative cycle: Bellman-Ford == Floyd-Warshall (== Dijkstra if asked) */
static void compare_all(const Graph *g, int n, int with_dijkstra) {
    long long fw[MAXN * MAXN];
    int nx[MAXN * MAXN];
    ASSERT_EQ(floyd_warshall(g, fw, nx), 0);
    for (int s = 0; s < n; s++) {
        int bd[MAXN], bp[MAXN], dd[MAXN], dpv[MAXN];
        ASSERT_EQ(bellman_ford(g, s, bd, bp), 0);
        if (with_dijkstra) ASSERT_EQ(dijkstra(g, s, dd, dpv), 0);
        for (int v = 0; v < n; v++) {
            if (fw[s * n + v] >= FW_INF) {
                ASSERT_EQ(bd[v] == DIST_INF, 1);
                if (with_dijkstra) ASSERT_EQ(dd[v] == DIST_INF, 1);
            } else {
                ASSERT_EQ(bd[v], fw[s * n + v]);
                if (with_dijkstra) ASSERT_EQ(dd[v], fw[s * n + v]);
            }
        }
    }
}

int main(void) {
    int bd[MAXN], bp[MAXN];
    long long fw[MAXN * MAXN];
    int nx[MAXN * MAXN];

    /* ---- known: negative cycle 1 -> 2 -> 1 costs -3 + 1 = -2, reachable from 0 ---- */
    Graph g;
    ASSERT_EQ(graph_init(&g, 3), 0);
    graph_add_edge(&g, 0, 1, 1); graph_add_edge(&g, 1, 2, -3); graph_add_edge(&g, 2, 1, 1);
    ASSERT_EQ(bellman_ford(&g, 0, bd, bp), 1);
    ASSERT_EQ(floyd_warshall(&g, fw, nx), 1);
    graph_free(&g);

    /* ---- known: negative edge, no cycle. 0->1 (4), 0->2 (5), 2->1 (-3): best to 1 is 2 ---- */
    ASSERT_EQ(graph_init(&g, 3), 0);
    graph_add_edge(&g, 0, 1, 4); graph_add_edge(&g, 0, 2, 5); graph_add_edge(&g, 2, 1, -3);
    ASSERT_EQ(bellman_ford(&g, 0, bd, bp), 0);
    ASSERT_EQ(bd[1], 2); ASSERT_EQ(bd[2], 5);
    ASSERT_EQ(bp[1], 2);                                /* reached through node 2 */
    ASSERT_EQ(dijkstra(&g, 0, bd, bp), -2);             /* Dijkstra refuses negatives */
    graph_free(&g);

    /* ---- negative cycle NOT reachable from the source ---- */
    ASSERT_EQ(graph_init(&g, 4), 0);
    graph_add_edge(&g, 0, 1, 1);
    graph_add_edge(&g, 2, 3, -1); graph_add_edge(&g, 3, 2, -1);
    ASSERT_EQ(bellman_ford(&g, 0, bd, bp), 0);          /* source never sees the cycle */
    ASSERT_EQ(bd[1], 1); ASSERT_EQ(bd[2] == DIST_INF, 1);
    ASSERT_EQ(bellman_ford(&g, 2, bd, bp), 1);          /* but from node 2 it is found */
    ASSERT_EQ(floyd_warshall(&g, fw, nx), 1);           /* Floyd finds it regardless */
    ASSERT_EQ(bellman_ford(&g, 9, bd, bp), -1);         /* bad source */
    graph_free(&g);

    srand(51);

    /* ---- random non-negative graphs: all three methods agree, paths are valid ---- */
    for (int t = 0; t < 500; t++) {
        int n = 1 + rand() % 7;
        ASSERT_EQ(graph_init(&g, n), 0);
        long long wm[MAXN][MAXN];
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) wm[i][j] = FW_INF;
        int m = rand() % 20;
        for (int k = 0; k < m; k++) {
            int a = rand() % n, b = rand() % n, w = rand() % 15;   /* zero weights allowed */
            ASSERT_EQ(graph_add_edge(&g, a, b, w), 0);
            if (w < wm[a][b]) wm[a][b] = w;
        }
        compare_all(&g, n, 1);

        ASSERT_EQ(floyd_warshall(&g, fw, nx), 0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                if (fw[i * n + j] >= FW_INF) {
                    ASSERT_EQ(fw_path(nx, n, i, j, bp, MAXN), -1);
                    continue;
                }
                int path[MAXN + 1];
                int len = fw_path(nx, n, i, j, path, MAXN + 1);
                ASSERT_EQ(len >= 1, 1);
                ASSERT_EQ(path[0], i);
                ASSERT_EQ(path[len - 1], j);
                long long sum = 0;                      /* the route must really cost d[i][j] */
                for (int s = 0; s + 1 < len; s++) sum += wm[path[s]][path[s + 1]];
                ASSERT_EQ(sum, fw[i * n + j]);
            }
        graph_free(&g);
    }

    /* ---- random DAGs with negative edges (edges only go from low to high index) ---- */
    for (int t = 0; t < 500; t++) {
        int n = 1 + rand() % 7;
        ASSERT_EQ(graph_init(&g, n), 0);
        int m = rand() % 20;
        for (int k = 0; k < m; k++) {
            int a = rand() % n, b = rand() % n;
            if (a < b) graph_add_edge(&g, a, b, rand() % 21 - 10);
        }
        compare_all(&g, n, 0);
        graph_free(&g);
    }

    /* ---- random general graphs, negative cycles allowed ---- */
    for (int t = 0; t < 1000; t++) {
        int n = 1 + rand() % 6;
        ASSERT_EQ(graph_init(&g, n), 0);
        int m = rand() % 14;
        for (int k = 0; k < m; k++)
            graph_add_edge(&g, rand() % n, rand() % n, rand() % 16 - 5);
        int fwrc = floyd_warshall(&g, fw, nx);
        int any_bf = 0;
        for (int s = 0; s < n; s++) {
            int rc = bellman_ford(&g, s, bd, bp);
            ASSERT_EQ(rc == 0 || rc == 1, 1);
            if (rc == 1) any_bf = 1;
        }
        ASSERT_EQ(fwrc, any_bf);                        /* same verdict on negative cycles */
        if (fwrc == 0) compare_all(&g, n, 0);
        graph_free(&g);
    }
    TEST_SUMMARY();
}