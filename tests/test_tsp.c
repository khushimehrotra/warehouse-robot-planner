#include <stdlib.h>
#include <limits.h>
#include "test.h"
#include "tsp.h"

static long long brute_tsp(const int *d, int n, int last, int visited, int count) {
    if (count == n) return d[last * n + 0];
    long long best = LLONG_MAX / 4;
    for (int c = 1; c < n; c++)
        if (!((visited >> c) & 1)) {
            long long t = d[last * n + c] + brute_tsp(d, n, c, visited | (1 << c), count + 1);
            if (t < best) best = t;
        }
    return best;
}

int main(void) {
    int tour[TSP_MAX_N];
    long long cost;

    /* ---- known: 4 cities on a square, going around costs 4 ---- */
    int sq[16] = { 0, 1, 2, 1,
                   1, 0, 1, 2,
                   2, 1, 0, 1,
                   1, 2, 1, 0 };
    ASSERT_EQ(tsp_held_karp(sq, 4, tour, &cost), 0);
    ASSERT_EQ(cost, 4);
    ASSERT_EQ(tour[0], 0);

    /* ---- edge cases ---- */
    int one[1] = {0};
    ASSERT_EQ(tsp_held_karp(one, 1, tour, &cost), 0);
    ASSERT_EQ(cost, 0); ASSERT_EQ(tour[0], 0);
    int two[4] = {0, 5, 7, 0};                          /* 0->1 costs 5, 1->0 costs 7 */
    ASSERT_EQ(tsp_held_karp(two, 2, tour, &cost), 0);
    ASSERT_EQ(cost, 12);
    ASSERT_EQ(tsp_held_karp(two, 0, tour, &cost), -1);
    ASSERT_EQ(tsp_held_karp(two, TSP_MAX_N + 1, tour, &cost), -1);
    ASSERT_EQ(tsp_held_karp(NULL, 3, tour, &cost), -1);
    int none[9] = {0, TSP_INF, TSP_INF, TSP_INF, 0, TSP_INF, TSP_INF, TSP_INF, 0};
    ASSERT_EQ(tsp_held_karp(none, 3, tour, &cost), -2); /* no roads at all */

    /* ---- random asymmetric matrices against all-orders brute force ---- */
    srand(61);
    for (int t = 0; t < 400; t++) {
        int n = 1 + rand() % 8;
        int d[64];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) d[i * n + j] = (i == j) ? 0 : 1 + rand() % 30;
        ASSERT_EQ(tsp_held_karp(d, n, tour, &cost), 0);
        ASSERT_EQ(cost, brute_tsp(d, n, 0, 1, 1));

        int seen = 0;                                   /* the tour must be a real permutation... */
        long long sum = 0;
        ASSERT_EQ(tour[0], 0);
        for (int i = 0; i < n; i++) {
            ASSERT_EQ(tour[i] >= 0 && tour[i] < n, 1);
            ASSERT_EQ((seen >> tour[i]) & 1, 0);
            seen |= 1 << tour[i];
            sum += d[tour[i] * n + tour[(i + 1) % n]];
        }
        ASSERT_EQ(sum, cost);                           /* ...whose cost matches the answer */
    }
    TEST_SUMMARY();
}