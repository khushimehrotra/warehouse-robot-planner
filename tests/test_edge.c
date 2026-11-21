#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "test.h"
#include "vec.h"
#include "minheap.h"
#include "grid.h"
#include "sort.h"
#include "graph.h"
#include "shortest.h"
#include "dp.h"
#include "tsp.h"
#include "flow.h"
#include "numtheory.h"
#include "huffman.h"
#include "task_io.h"
#include "robot_io.h"

static void write_file(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (f) { fputs(text, f); fclose(f); }
}

int main(void) {
    /* ---- empty and single-element inputs ---- */
    int one[1] = {7};
    ASSERT_EQ(merge_sort(NULL, 0), 0);
    ASSERT_EQ(merge_sort(one, 1), 0);
    quick_sort(NULL, 0);
    quick_sort(one, 1);
    ASSERT_EQ(bin_search_iter(NULL, 0, 5), -1);
    ASSERT_EQ(bin_search_rec(NULL, 0, -1, 5), -1);
    ASSERT_EQ(bin_search_iter(one, 1, 7), 0);

    /* ---- heap and vec on empty ---- */
    MinHeap h;
    HeapItem it;
    ASSERT_EQ(heap_init(&h, 0), 0);                    /* capacity 0 falls back to default */
    ASSERT_EQ(heap_pop(&h, &it), -1);
    heap_free(&h);
    heap_free(&h);                                     /* double free must be safe */
    Vec v;
    vec_init(&v);
    vec_free(&v);
    vec_free(&v);

    /* ---- malformed grid files ---- */
    Grid g;
    write_file("/tmp/g_empty.txt", "");
    ASSERT_EQ(grid_load("/tmp/g_empty.txt", &g), -2);
    write_file("/tmp/g_neg.txt", "-3 4\n....\n");
    ASSERT_EQ(grid_load("/tmp/g_neg.txt", &g), -2);
    write_file("/tmp/g_short.txt", "2 4\n....\n...\n");
    ASSERT_EQ(grid_load("/tmp/g_short.txt", &g), -4);
    write_file("/tmp/g_char.txt", "1 4\n..x.\n");
    ASSERT_EQ(grid_load("/tmp/g_char.txt", &g), -5);
    write_file("/tmp/g_missing.txt", "3 4\n....\n");
    ASSERT_EQ(grid_load("/tmp/g_missing.txt", &g), -4);
    ASSERT_EQ(g.cells == NULL, 1);                     /* nothing left allocated on error */

    /* ---- malformed task and robot files ---- */
    Task *t;
    Robot *r;
    int n;
    write_file("/tmp/t_neg.txt", "-1\n");
    ASSERT_EQ(tasks_load("/tmp/t_neg.txt", &t, &n), -2);
    write_file("/tmp/t_short.txt", "2\n1 0 1 5 0 40 2\n");
    ASSERT_EQ(tasks_load("/tmp/t_short.txt", &t, &n), -4);
    write_file("/tmp/t_bad.txt", "1\n1 0 1 0 0 40 2\n");          /* weight 0 */
    ASSERT_EQ(tasks_load("/tmp/t_bad.txt", &t, &n), -5);
    write_file("/tmp/t_zero.txt", "0\n");
    ASSERT_EQ(tasks_load("/tmp/t_zero.txt", &t, &n), 0);          /* zero tasks is legal */
    ASSERT_EQ(n, 0);
    free(t);
    write_file("/tmp/r_bad.txt", "1\n1 0 0 -5 10\n");              /* negative battery */
    ASSERT_EQ(robots_load("/tmp/r_bad.txt", &r, &n), -5);

    /* ---- graph edge cases ---- */
    Graph gr;
    ASSERT_EQ(graph_init(&gr, 1), 0);                  /* single node */
    int d[1], p[1];
    ASSERT_EQ(dijkstra(&gr, 0, d, p), 0);
    ASSERT_EQ(d[0], 0);
    graph_free(&gr);
    graph_free(&gr);                                   /* double free must be safe */
    ASSERT_EQ(graph_init(&gr, 3), 0);                  /* no edges: everything unreachable */
    int d3[3], p3[3];
    ASSERT_EQ(dijkstra(&gr, 0, d3, p3), 0);
    ASSERT_EQ(d3[1] == DIST_INF, 1);
    graph_add_edge(&gr, 0, 0, 5);                      /* self-loop */
    graph_add_edge(&gr, 0, 1, 0);                      /* zero weight */
    ASSERT_EQ(dijkstra(&gr, 0, d3, p3), 0);
    ASSERT_EQ(d3[1], 0);
    graph_free(&gr);

    /* ---- dp and tsp limits ---- */
    long long best;
    ASSERT_EQ(knapsack01(NULL, NULL, 0, 0, &best, NULL), 0);
    long long cost;
    int tour[TSP_MAX_N];
    ASSERT_EQ(tsp_held_karp(NULL, 0, tour, &cost), -1);

    /* ---- flow with zero capacity ---- */
    Graph fg;
    long long f;
    ASSERT_EQ(graph_init(&fg, 2), 0);
    ASSERT_EQ(flow_add_edge(&fg, 0, 1, 0), 0);
    ASSERT_EQ(max_flow(&fg, 0, 1, &f), 0);
    ASSERT_EQ(f, 0);
    graph_free(&fg);

    /* ---- number theory overflow ---- */
    long long x, L;
    long long big_r[] = {1, 2}, big_m[] = {CRT_MAX_MOD, CRT_MAX_MOD - 1};
    ASSERT_EQ(crt(big_r, big_m, 2, &x, &L), 0);
    ASSERT_EQ(x >= 0 && x < L, 1);

    /* ---- huffman on binary data with every byte value ---- */
    unsigned char all[256];
    for (int i = 0; i < 256; i++) all[i] = (unsigned char)i;
    Huff hf;
    ASSERT_EQ(huff_build(&hf, all, 256), 0);
    unsigned char enc[512], dec[256];
    size_t bits = huff_bits(&hf, all, 256);
    ASSERT_EQ(huff_encode(&hf, all, 256, enc), 0);
    ASSERT_EQ(huff_decode(&hf, enc, bits, dec, 256), 0);
    ASSERT_EQ(memcmp(all, dec, 256), 0);

    TEST_SUMMARY();
}