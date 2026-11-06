#include <stdlib.h>
#include <limits.h>
#include "test.h"
#include "graph.h"
#include "unionfind.h"

#define MAXN 8
#define BIG 1000000000LL

/* Try every subset of n-1 edges; a subset with no cycle is a spanning tree. */
static long long brute_mst(int n, const WEdge *e, int m) {
    long long best = -1;
    for (int mask = 0; mask < (1 << m); mask++) {
        int bits = 0;
        for (int i = 0; i < m; i++) bits += (mask >> i) & 1;
        if (bits != n - 1) continue;
        UF u;
        uf_init(&u, n);
        int ok = 1;
        long long sum = 0;
        for (int i = 0; i < m && ok; i++)
            if ((mask >> i) & 1) {
                if (!uf_union(&u, e[i].u, e[i].v)) ok = 0;
                sum += e[i].w;
            }
        uf_free(&u);
        if (ok && (best < 0 || sum < best)) best = sum;
    }
    return best;
}

int main(void) {
    /* ---- union-find ---- */
    UF u;
    ASSERT_EQ(uf_init(&u, 5), 0);
    ASSERT_EQ(uf_union(&u, 0, 1), 1);
    ASSERT_EQ(uf_union(&u, 1, 0), 0);
    ASSERT_EQ(uf_union(&u, 3, 4), 1);
    ASSERT_EQ(uf_find(&u, 0) == uf_find(&u, 1), 1);
    ASSERT_EQ(uf_find(&u, 0) == uf_find(&u, 3), 0);
    ASSERT_EQ(uf_union(&u, 1, 4), 1);
    ASSERT_EQ(uf_find(&u, 0) == uf_find(&u, 3), 1);
    uf_free(&u);
    ASSERT_EQ(uf_init(&u, 0), -1);

    /* ---- known MST: edges 1 + 2 + 4 = 7 ---- */
    WEdge ke[] = {{0,1,1},{1,2,2},{0,2,3},{2,3,4}};
    WEdge mst[MAXN]; int cnt; long long total;
    ASSERT_EQ(kruskal(4, ke, 4, mst, &cnt, &total), 0);
    ASSERT_EQ(cnt, 3); ASSERT_EQ(total, 7);

    /* ---- disconnected graph: two separate pairs ---- */
    WEdge de[] = {{0,1,5},{2,3,7}};
    ASSERT_EQ(kruskal(4, de, 2, mst, &cnt, &total), 0);
    ASSERT_EQ(cnt, 2); ASSERT_EQ(total, 12);
    Graph dg; int parent[MAXN], vis; long long ptotal;
    ASSERT_EQ(graph_init(&dg, 4), 0);
    graph_add_undirected(&dg, 0, 1, 5);
    graph_add_undirected(&dg, 2, 3, 7);
    ASSERT_EQ(prim(&dg, 0, parent, &ptotal, &vis), 0);
    ASSERT_EQ(vis, 2); ASSERT_EQ(ptotal, 5);
    graph_free(&dg);

    /* ---- invalid arguments ---- */
    Graph bad;
    ASSERT_EQ(graph_init(&bad, 0), -1);
    ASSERT_EQ(graph_init(&bad, 3), 0);
    ASSERT_EQ(graph_add_edge(&bad, 0, 9, 1), -1);
    int d[3], p[3];
    ASSERT_EQ(dijkstra(&bad, 7, d, p), -1);
    graph_add_edge(&bad, 0, 1, -3);                    /* negative weight */
    ASSERT_EQ(dijkstra(&bad, 0, d, p), -2);
    graph_free(&bad);

    /* ---- random MST: Kruskal == Prim == brute force ---- */
    srand(21);
    for (int t = 0; t < 500; t++) {
        int n = 1 + rand() % 6, m = 0;
        WEdge e[16];
        for (int i = 1; i < n; i++) {                  /* a random tree keeps it connected */
            e[m].u = rand() % i; e[m].v = i; e[m].w = 1 + rand() % 20; m++;
        }
        int extra = (n > 1) ? rand() % 5 : 0;
        for (int k = 0; k < extra; k++) {
            int a = rand() % n, b = rand() % n;
            if (a == b) continue;
            e[m].u = a; e[m].v = b; e[m].w = 1 + rand() % 20; m++;
        }
        ASSERT_EQ(kruskal(n, e, m, mst, &cnt, &total), 0);
        ASSERT_EQ(cnt, n - 1);
        ASSERT_EQ(total, brute_mst(n, e, m));

        Graph g;
        ASSERT_EQ(graph_init(&g, n), 0);
        for (int i = 0; i < m; i++) graph_add_undirected(&g, e[i].u, e[i].v, e[i].w);
        ASSERT_EQ(prim(&g, 0, parent, &ptotal, &vis), 0);
        ASSERT_EQ(vis, n);
        ASSERT_EQ(ptotal, total);
        graph_free(&g);
    }

    /* ---- random directed graphs: Dijkstra == Floyd-Warshall ---- */
    for (int t = 0; t < 500; t++) {
        int n = 1 + rand() % 7;
        Graph g;
        ASSERT_EQ(graph_init(&g, n), 0);
        long long fw[MAXN][MAXN];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) fw[i][j] = (i == j) ? 0 : BIG;
        int m = rand() % 20;
        for (int k = 0; k < m; k++) {
            int a = rand() % n, b = rand() % n, w = 1 + rand() % 20;
            ASSERT_EQ(graph_add_edge(&g, a, b, w), 0);
            if (w < fw[a][b]) fw[a][b] = w;
        }
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    if (fw[i][k] + fw[k][j] < fw[i][j]) fw[i][j] = fw[i][k] + fw[k][j];

        for (int src = 0; src < n; src++) {
            int dist[MAXN], prev[MAXN];
            ASSERT_EQ(dijkstra(&g, src, dist, prev), 0);
            for (int v = 0; v < n; v++) {
                if (fw[src][v] >= BIG) {
                    ASSERT_EQ(dist[v] == DIST_INF, 1);
                } else {
                    ASSERT_EQ(dist[v], fw[src][v]);
                    int path[MAXN];
                    int len = path_extract(prev, src, v, path, MAXN);
                    ASSERT_EQ(len >= 1, 1);
                    ASSERT_EQ(path[0], src);
                    ASSERT_EQ(path[len - 1], v);
                }
            }
        }
        graph_free(&g);
    }
    TEST_SUMMARY();
}