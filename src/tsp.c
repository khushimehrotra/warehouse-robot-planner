#include <stdlib.h>
#include <limits.h>
#include "tsp.h"

#define HK_INF (LLONG_MAX / 4)

/* Cities 1..n-1 map to bits 0..m-1 (m = n-1). City 0 is the fixed start. */
int tsp_held_karp(const int *dist, int n, int *tour, long long *cost) {
    *cost = 0;
    if (n < 1 || n > TSP_MAX_N || dist == NULL || tour == NULL) return -1;
    if (n == 1) { tour[0] = 0; return 0; }

    int m = n - 1;
    size_t full = (size_t)1 << m;
    long long *dp = malloc(full * (size_t)m * sizeof(long long));
    signed char *par = malloc(full * (size_t)m);        /* previous city, for the tour */
    if (dp == NULL || par == NULL) { free(dp); free(par); return -3; }
    for (size_t i = 0; i < full * (size_t)m; i++) { dp[i] = HK_INF; par[i] = -1; }

    for (int j = 0; j < m; j++)                         /* base: 0 -> j+1 directly */
        if (dist[j + 1] < TSP_INF) dp[((size_t)1 << j) * m + j] = dist[j + 1];

    for (size_t mask = 1; mask < full; mask++)          /* smaller sets come first numerically */
        for (int j = 0; j < m; j++) {
            if (!((mask >> j) & 1)) continue;           /* j must be in the set */
            long long cur = dp[mask * m + j];
            if (cur >= HK_INF) continue;
            for (int k = 0; k < m; k++) {
                if ((mask >> k) & 1) continue;          /* k must be new */
                int d = dist[(j + 1) * n + (k + 1)];
                if (d >= TSP_INF) continue;
                size_t nm = mask | ((size_t)1 << k);
                long long c = cur + d;
                if (c < dp[nm * m + k]) { dp[nm * m + k] = c; par[nm * m + k] = (signed char)j; }
            }
        }

    long long best = HK_INF;
    int bj = -1;
    for (int j = 0; j < m; j++) {                       /* close the loop back to city 0 */
        int d = dist[(j + 1) * n];
        long long c = dp[(full - 1) * m + j];
        if (d >= TSP_INF || c >= HK_INF) continue;
        if (c + d < best) { best = c + d; bj = j; }
    }
    if (bj < 0) { free(dp); free(par); return -2; }

    size_t mask = full - 1;                             /* walk the parents back */
    int j = bj;
    for (int pos = n - 1; pos >= 1; pos--) {
        tour[pos] = j + 1;
        int pj = par[mask * m + j];
        mask &= ~((size_t)1 << j);
        j = pj;
    }
    tour[0] = 0;
    *cost = best;
    free(dp); free(par);
    return 0;
}