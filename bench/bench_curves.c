#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sort.h"
#include "matrix.h"
#include "graph.h"
#include "shortest.h"
#include "tsp.h"
#include "strmatch.h"

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void line(const char *name, int n, double ms, double prev) {
    printf("%-14s n=%-8d %10.3f ms   ratio vs previous: ", name, n, ms);
    if (prev > 0) printf("%.2f\n", ms / prev); else printf("-\n");
}

int main(void) {
    srand(1);
    double prev = 0;
    for (int n = 100000; n <= 1600000; n *= 2) {
        int *a = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) a[i] = rand();
        double t0 = now_ms(); merge_sort(a, n); double t = now_ms() - t0;
        line("merge sort", n, t, prev); prev = t; free(a);
    }
    prev = 0;
    for (int n = 64; n <= 512; n *= 2) {
        long long *A = malloc((size_t)n * n * 8), *B = malloc((size_t)n * n * 8), *C = malloc((size_t)n * n * 8);
        for (int i = 0; i < n * n; i++) { A[i] = rand() % 10; B[i] = rand() % 10; }
        double t0 = now_ms(); mat_mul_classic(A, B, C, n); double t = now_ms() - t0;
        line("matrix classic", n, t, prev); prev = t; free(A); free(B); free(C);
    }
    prev = 0;
    for (int side = 20; side <= 160; side *= 2) {
        Graph g;
        int n = side * side;
        graph_init(&g, n);
        for (int r = 0; r < side; r++)
            for (int c = 0; c < side; c++) {
                int id = r * side + c;
                if (c + 1 < side) graph_add_undirected(&g, id, id + 1, 1);
                if (r + 1 < side) graph_add_undirected(&g, id, id + side, 1);
            }
        int *d = malloc(n * sizeof(int)), *p = malloc(n * sizeof(int));
        double t0 = now_ms(); dijkstra(&g, 0, d, p); double t = now_ms() - t0;
        line("dijkstra grid", n, t, prev); prev = t; free(d); free(p); graph_free(&g);
    }
    prev = 0;
    for (int n = 10; n <= TSP_MAX_N; n += 2) {
        int dist[TSP_MAX_N * TSP_MAX_N], tour[TSP_MAX_N];
        long long cost;
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) dist[i * n + j] = i == j ? 0 : 1 + rand() % 50;
        double t0 = now_ms(); tsp_held_karp(dist, n, tour, &cost); double t = now_ms() - t0;
        line("held-karp", n, t, prev); prev = t;
    }
    prev = 0;
    for (size_t n = 500000; n <= 4000000; n *= 2) {
        char *t = malloc(n);
        for (size_t i = 0; i < n; i++) t[i] = (char)('a' + rand() % 4);
        size_t cnt;
        double t0 = now_ms(); match_kmp(t, n, t + 1000, 8, NULL, 0, &cnt); double ms = now_ms() - t0;
        line("kmp", (int)n, ms, prev); prev = ms; free(t);
    }
    return 0;
}