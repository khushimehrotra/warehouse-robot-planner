#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "graph.h"
#include "shortest.h"
#include "tsp.h"

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/* side x side open grid, 4-neighbour edges of weight 1 */
static int make_grid(Graph *g, int side) {
    if (graph_init(g, side * side) != 0) return -1;
    for (int r = 0; r < side; r++)
        for (int c = 0; c < side; c++) {
            int id = r * side + c;
            if (c + 1 < side) graph_add_undirected(g, id, id + 1, 1);
            if (r + 1 < side) graph_add_undirected(g, id, id + side, 1);
        }
    return 0;
}

int main(void) {
    srand(1);
    printf("Shortest paths on an open grid (one source for Dijkstra and Bellman-Ford,\n"
           "all pairs for Floyd-Warshall)\n");
    for (int side = 10; side <= 30; side += 10) {
        Graph g;
        make_grid(&g, side);
        int n = side * side;
        int *d = malloc(n * sizeof(int)), *p = malloc(n * sizeof(int));
        long long *fw = malloc((size_t)n * n * sizeof(long long));
        double t0 = now_ms(); dijkstra(&g, 0, d, p);        double t1 = now_ms();
        bellman_ford(&g, 0, d, p);                          double t2 = now_ms();
        floyd_warshall(&g, fw, NULL);                       double t3 = now_ms();
        printf("n=%-5d dijkstra %8.3f ms  bellman-ford %8.3f ms  floyd-warshall %9.2f ms\n",
               n, t1 - t0, t2 - t1, t3 - t2);
        free(d); free(p); free(fw);
        graph_free(&g);
    }

    printf("\nHeld-Karp TSP (random distances)\n");
    for (int n = 8; n <= TSP_MAX_N; n += 2) {
        int dist[TSP_MAX_N * TSP_MAX_N], tour[TSP_MAX_N];
        long long cost;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) dist[i * n + j] = (i == j) ? 0 : 1 + rand() % 50;
        double t0 = now_ms();
        tsp_held_karp(dist, n, tour, &cost);
        double t1 = now_ms();
        printf("n=%-3d cost=%-5lld %9.3f ms\n", n, cost, t1 - t0);
    }
    return 0;
}