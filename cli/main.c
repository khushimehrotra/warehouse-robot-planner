#include <stdio.h>
#include <stdlib.h>
#include "grid.h"
#include "gridpath.h"
#include "tsp.h"

/* Charger first, then every shelf (up to TSP_MAX_N stops in total). */
static void pickup_tour_demo(const Grid *g) {
    int pr[TSP_MAX_N], pc[TSP_MAX_N], n = 1;
    if (grid_find(g, 'C', &pr[0], &pc[0]) != 0) return;
    for (int r = 0; r < g->rows && n < TSP_MAX_N; r++)
        for (int c = 0; c < g->cols && n < TSP_MAX_N; c++)
            if (grid_get(g, r, c) == 'S') { pr[n] = r; pc[n] = c; n++; }
    if (n < 2) return;

    int cells = g->rows * g->cols;
    int *path = malloc((size_t)cells * sizeof(int));
    int *dist = malloc((size_t)n * n * sizeof(int));
    if (path == NULL || dist == NULL) { free(path); free(dist); return; }

    for (int i = 0; i < n; i++)                         /* walking distance between every pair */
        for (int j = 0; j < n; j++) {
            int len;
            if (i == j) { dist[i * n + j] = 0; continue; }
            int rc = grid_shortest_path(g, pr[i], pc[i], pr[j], pc[j], path, cells, &len);
            dist[i * n + j] = (rc == 0) ? len - 1 : TSP_INF;
        }

    int tour[TSP_MAX_N];
    long long cost;
    int rc = tsp_held_karp(dist, n, tour, &cost);
    if (rc == 0) {
        printf("\nBest pickup tour (charger -> %d shelves -> charger), %lld steps:\n  ",
               n - 1, cost);
        for (int i = 0; i < n; i++) printf("(%d,%d) -> ", pr[tour[i]], pc[tour[i]]);
        printf("(%d,%d)\n", pr[0], pc[0]);
    } else {
        printf("\nNo pickup tour (code %d)\n", rc);
    }
    free(path);
    free(dist);
}

int main(int argc, char *argv[]) {
    const char *file = (argc > 1) ? argv[1] : "data/small_grid.txt";

    Grid g;
    int rc = grid_load(file, &g);
    if (rc != 0) {
        printf("Could not load '%s' (error %d)\n", file, rc);
        return 1;
    }
    printf("Loaded %d x %d warehouse:\n", g.rows, g.cols);
    grid_print(&g);

    int sr, sc, tr, tc;
    if (grid_find(&g, 'C', &sr, &sc) != 0 || grid_find(&g, 'S', &tr, &tc) != 0) {
        printf("Map needs at least one charger (C) and one shelf (S).\n");
        grid_free(&g);
        return 1;
    }

    int *route = malloc((size_t)g.rows * g.cols * sizeof(int));
    if (route == NULL) { grid_free(&g); return 1; }
    int len;
    rc = grid_shortest_path(&g, sr, sc, tr, tc, route, g.rows * g.cols, &len);
    if (rc == 0) {
        printf("\nRoute from charger (%d,%d) to shelf (%d,%d): %d steps\n",
               sr, sc, tr, tc, len - 1);
        grid_print_path(&g, route, len);
    } else {
        printf("\nNo route found (code %d)\n", rc);
    }
    free(route);

    pickup_tour_demo(&g);
    grid_free(&g);
    return 0;
}