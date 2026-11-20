#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "gridpath.h"
#include "tsp.h"
#include "robot_io.h"
#include "task_io.h"
#include "sim.h"
#include "strmatch.h"
#include "huffman.h"

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

static void print_metrics(const char *name, const SimMetrics *m) {
    printf("%-24s done=%-2d fail=%-2d unserved=%-2d makespan=%-4d late=%-2d avg_lat=%-6.1f "
           "dist=%-5lld wait=%-4lld util=%.2f coll=%d\n",
           name, m->completed, m->failed, m->unserved, m->makespan, m->deadline_misses,
           m->avg_latency, m->total_distance, m->total_wait, m->utilization, m->collisions);
}

static void sim_demo(void) {
    Grid g; Robot *rb = NULL; Task *tk = NULL; int nr = 0, nt = 0;
    int rc1 = grid_load("data/sim_grid.txt", &g);
    int rc2 = robots_load("data/sim_robots.txt", &rb, &nr);
    int rc3 = tasks_load("data/sim_tasks.txt", &tk, &nt);
    if (rc1 || rc2 || rc3) {
        printf("Could not load the simulation data (%d %d %d)\n", rc1, rc2, rc3);
        goto out;
    }
    static char logbuf[4096];
    SimLog lg = {logbuf, (int)sizeof logbuf, 0, 0};
    SimConfig cfg;
    SimMetrics m;
    sim_default_config(&cfg);
    printf("%d robots, %d tasks, %dx%d warehouse\n\n", nr, nt, g.rows, g.cols);

    cfg.log = &lg;
    if (sim_run(&g, rb, nr, tk, nt, &cfg, &m, NULL) == 0) print_metrics("greedy nearest robot", &m);
    cfg.log = NULL;
    cfg.assign = SIM_ASSIGN_BNB;
    if (sim_run(&g, rb, nr, tk, nt, &cfg, &m, NULL) == 0) print_metrics("batch branch and bound", &m);
    cfg.assign = SIM_ASSIGN_GREEDY;
    cfg.avoid_collisions = 0;
    if (sim_run(&g, rb, nr, tk, nt, &cfg, &m, NULL) == 0) print_metrics("greedy, NO reservations", &m);

    printf("\nEvent log of robot 1 (%d ticks):\n%.*s\n", lg.len, lg.len, lg.buf);
    static const struct { const char *name; int (*f)(const char *, size_t, const char *, size_t,
                                                       size_t *, size_t, size_t *); } M[] = {
        {"naive", match_naive}, {"rabin-karp", match_rabin_karp},
        {"automaton", match_automaton}, {"kmp", match_kmp}};
    const char *pat[2] = {"MP", "WW"};      /* arrived-and-picked, and a stall of 2+ ticks */
    for (int p = 0; p < 2; p++) {
        printf("pattern \"%s\":", pat[p]);
        for (int i = 0; i < 4; i++) {
            size_t cnt = 0;
            M[i].f(lg.buf, (size_t)lg.len, pat[p], 2, NULL, 0, &cnt);
            printf("  %s=%zu", M[i].name, cnt);
        }
        printf("\n");
    }
    Huff h;
    if (lg.len > 0 && huff_build(&h, (const unsigned char *)lg.buf, (size_t)lg.len) == 0)
        printf("log size: %d bits plain, %zu bits with Huffman\n", 8 * lg.len,
               huff_bits(&h, (const unsigned char *)lg.buf, (size_t)lg.len));
out:
    free(rb); free(tk); grid_free(&g);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "sim") == 0) { sim_demo(); return 0; }
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