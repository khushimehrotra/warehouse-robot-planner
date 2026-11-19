#include <stdlib.h>
#include "test.h"
#include "flow.h"

#define MAXN 8

static long long brute_cut(int n, const int *eu, const int *ev, const int *ec, int m) {
    long long best = -1;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (!(mask & 1)) continue;                     /* source 0 must be inside */
        if (mask & (1 << (n - 1))) continue;           /* sink must be outside */
        long long cap = 0;
        for (int k = 0; k < m; k++)
            if (((mask >> eu[k]) & 1) && !((mask >> ev[k]) & 1)) cap += ec[k];
        if (best < 0 || cap < best) best = cap;
    }
    return best;
}

static int brute_match(const int *eu, const int *ev, int m) {
    int best = 0;
    for (int mask = 0; mask < (1 << m); mask++) {
        int usedL = 0, usedR = 0, ok = 1, cnt = 0;
        for (int k = 0; k < m && ok; k++) {
            if (!((mask >> k) & 1)) continue;
            if (((usedL >> eu[k]) & 1) || ((usedR >> ev[k]) & 1)) ok = 0;
            usedL |= 1 << eu[k];
            usedR |= 1 << ev[k];
            cnt++;
        }
        if (ok && cnt > best) best = cnt;
    }
    return best;
}

int main(void) {
    Graph g;
    long long f;
    char side[MAXN];

    /* ---- known aisle network: the sink-side edges 3->4 (7) and 2->4 (3) limit it to 10 ---- */
    int ke[7][3] = {{0,1,10},{0,2,8},{1,3,5},{2,3,4},{1,2,2},{3,4,7},{2,4,3}};
    ASSERT_EQ(graph_init(&g, 5), 0);
    for (int k = 0; k < 7; k++) ASSERT_EQ(flow_add_edge(&g, ke[k][0], ke[k][1], ke[k][2]), 0);
    ASSERT_EQ(max_flow(&g, 0, 4, &f), 0);
    ASSERT_EQ(f, 10);
    ASSERT_EQ(flow_min_cut(&g, 0, side), 0);
    ASSERT_EQ(side[0], 1); ASSERT_EQ(side[4], 0);
    graph_free(&g);

    /* ---- no path: flow 0 ---- */
    ASSERT_EQ(graph_init(&g, 3), 0);
    flow_add_edge(&g, 0, 1, 5);
    ASSERT_EQ(max_flow(&g, 0, 2, &f), 0);
    ASSERT_EQ(f, 0);
    graph_free(&g);

    /* ---- invalid arguments ---- */
    ASSERT_EQ(graph_init(&g, 2), 0);
    ASSERT_EQ(flow_add_edge(&g, 0, 1, -1), -1);
    ASSERT_EQ(flow_add_edge(&g, 0, 9, 1), -1);
    ASSERT_EQ(max_flow(&g, 0, 0, &f), -1);
    ASSERT_EQ(max_flow(&g, 0, 5, &f), -1);
    graph_free(&g);

    srand(101);

    /* ---- random networks: flow == brute-force min cut, and the flow itself is legal ---- */
    for (int t = 0; t < 500; t++) {
        int n = 2 + rand() % 6, m = rand() % 17;
        int eu[16], ev[16], ec[16];
        ASSERT_EQ(graph_init(&g, n), 0);
        for (int k = 0; k < m; k++) {
            eu[k] = rand() % n; ev[k] = rand() % n; ec[k] = rand() % 10;
            ASSERT_EQ(flow_add_edge(&g, eu[k], ev[k], ec[k]), 0);
        }
        ASSERT_EQ(max_flow(&g, 0, n - 1, &f), 0);
        ASSERT_EQ(f, brute_cut(n, eu, ev, ec, m));

        long long net[MAXN] = {0};                     /* net outflow of every node */
        for (int k = 0; k < m; k++) {
            int fk = g.w[2 * k + 1];                   /* flow on edge k = its backward residual */
            ASSERT_EQ(fk >= 0 && fk <= ec[k], 1);
            net[eu[k]] += fk;
            net[ev[k]] -= fk;
        }
        for (int v = 1; v < n - 1; v++) ASSERT_EQ(net[v], 0);   /* conservation */
        ASSERT_EQ(net[0], f);
        ASSERT_EQ(net[n - 1], -f);

        ASSERT_EQ(flow_min_cut(&g, 0, side), 0);       /* the cut found has capacity == flow */
        ASSERT_EQ(side[0], 1); ASSERT_EQ(side[n - 1], 0);
        long long cut = 0;
        for (int k = 0; k < m; k++)
            if (side[eu[k]] && !side[ev[k]]) cut += ec[k];
        ASSERT_EQ(cut, f);
        graph_free(&g);
    }

    /* ---- matching: known case. Robot 1 only fits task 0, so a perfect matching exists ---- */
    int ml[8], mr[8], size, size2;
    int ku[] = {0, 0, 1, 2, 2}, kv[] = {0, 1, 0, 1, 2};
    ASSERT_EQ(bipartite_matching(3, 3, ku, kv, 5, ml, mr, &size), 0);
    ASSERT_EQ(size, 3);
    ASSERT_EQ(matching_by_flow(3, 3, ku, kv, 5, &size2), 0);
    ASSERT_EQ(size2, 3);
    ASSERT_EQ(bipartite_matching(2, 2, NULL, NULL, 0, ml, mr, &size), 0);   /* no edges */
    ASSERT_EQ(size, 0);
    int bad_u[] = {5}, bad_v[] = {0};
    ASSERT_EQ(bipartite_matching(2, 2, bad_u, bad_v, 1, ml, mr, &size), -1);
    ASSERT_EQ(matching_by_flow(2, 2, bad_u, bad_v, 1, &size2), -1);

    /* ---- random matching: Kuhn == flow reduction == brute force ---- */
    for (int t = 0; t < 500; t++) {
        int nl = 1 + rand() % 5, nr = 1 + rand() % 5, m = rand() % 11;
        int eu[10], ev[10];
        for (int k = 0; k < m; k++) { eu[k] = rand() % nl; ev[k] = rand() % nr; }
        ASSERT_EQ(bipartite_matching(nl, nr, eu, ev, m, ml, mr, &size), 0);
        ASSERT_EQ(size, brute_match(eu, ev, m));
        ASSERT_EQ(matching_by_flow(nl, nr, eu, ev, m, &size2), 0);
        ASSERT_EQ(size2, size);

        int matched = 0;                               /* the pairs reported must be real */
        for (int u = 0; u < nl; u++) {
            int v = ml[u];
            if (v < 0) continue;
            matched++;
            ASSERT_EQ(mr[v], u);
            int found = 0;
            for (int k = 0; k < m; k++) if (eu[k] == u && ev[k] == v) found = 1;
            ASSERT_EQ(found, 1);
        }
        ASSERT_EQ(matched, size);
    }
    TEST_SUMMARY();
}