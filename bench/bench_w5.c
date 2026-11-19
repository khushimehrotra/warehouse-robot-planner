#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "backtrack.h"
#include "bnb.h"
#include "flow.h"
#include "numtheory.h"

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(void) {
    srand(1);

    printf("N-Queens: count all solutions\n");
    for (int n = 6; n <= 11; n++) {
        long long cnt;
        double t0 = now_ms();
        nqueens(n, &cnt, NULL);
        double t1 = now_ms();
        printf("n=%-3d solutions=%-7lld %9.3f ms\n", n, cnt, t1 - t0);
    }

    printf("\nSubset sum: 28 random items (1..50), target 350\n");
    {
        int a[28];
        long long cnt, nodes;
        for (int i = 0; i < 28; i++) a[i] = 1 + rand() % 50;
        double t0 = now_ms();
        subset_sum(a, 28, 350, NULL, &cnt, &nodes);
        double t1 = now_ms();
        printf("solutions=%lld  nodes visited=%lld  (full tree: %lld)  %.3f ms\n",
               cnt, nodes, (1LL << 29) - 1, t1 - t0);
    }

    printf("\nTask assignment by branch and bound (random costs 1..99)\n");
    printf("%-3s %-5s %10s %10s %10s %9s %6s\n",
           "n", "mode", "generated", "expanded", "pruned", "ms", "total");
    for (int n = 6; n <= 10; n++) {
        int c[100];
        for (int i = 0; i < n * n; i++) c[i] = 1 + rand() % 99;
        for (int mode = BNB_FIFO; mode <= BNB_LC; mode++) {
            int as[BNB_MAX_N];
            long long tot;
            BnbStats st;
            double t0 = now_ms();
            int rc = assign_bnb(c, n, (BnbMode)mode, as, &tot, &st);
            double t1 = now_ms();
            if (rc == 0)
                printf("%-3d %-5s %10lld %10lld %10lld %9.2f %6lld\n", n,
                       bnb_name((BnbMode)mode), st.generated, st.expanded, st.pruned, t1 - t0, tot);
            else
                printf("%-3d %-5s stopped with code %d (node limit)\n",
                       n, bnb_name((BnbMode)mode), rc);
        }
    }

    printf("\nAisle network (robots per minute)\n");
    {
        Graph g;
        int e[7][3] = {{0,1,10},{0,2,8},{1,3,5},{2,3,4},{1,2,2},{3,4,7},{2,4,3}};
        long long f;
        char side[5];
        graph_init(&g, 5);
        for (int k = 0; k < 7; k++) flow_add_edge(&g, e[k][0], e[k][1], e[k][2]);
        max_flow(&g, 0, 4, &f);
        flow_min_cut(&g, 0, side);
        printf("max throughput = %lld   source side of the bottleneck cut:", f);
        for (int v = 0; v < 5; v++) if (side[v]) printf(" %d", v);
        printf("\n");
        graph_free(&g);
    }

    printf("\nMatching: 200 robots x 200 tasks, 5 candidate tasks each\n");
    {
        int eu[1000], ev[1000], ml[200], mr[200], s1, s2, m = 0;
        for (int u = 0; u < 200; u++)
            for (int k = 0; k < 5; k++) { eu[m] = u; ev[m] = rand() % 200; m++; }
        double t0 = now_ms(); bipartite_matching(200, 200, eu, ev, m, ml, mr, &s1);
        double t1 = now_ms(); matching_by_flow(200, 200, eu, ev, m, &s2);
        double t2 = now_ms();
        printf("augmenting paths: %d pairs in %.3f ms | via max flow: %d pairs in %.3f ms\n",
               s1, t1 - t0, s2, t2 - t1);
    }

    printf("\nCRT: charge every 4 (offset 1), service every 9 (3), calibration every 5 (2)\n");
    {
        long long r[] = {1, 3, 2}, m[] = {4, 9, 5}, x, L;
        if (crt(r, m, 3, &x, &L) == 0)
            printf("first time all three coincide: t=%lld, then every %lld ticks\n", x, L);
    }
    return 0;
}