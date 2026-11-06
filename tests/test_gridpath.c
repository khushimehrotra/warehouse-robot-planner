#include <stdlib.h>
#include "test.h"
#include "grid.h"
#include "gridpath.h"

/* BFS distance in steps, or -1 if unreachable */
static int bfs_dist(const Grid *g, int sr, int sc, int tr, int tc) {
    int n = g->rows * g->cols;
    int *d = malloc((size_t)n * sizeof(int)), *q = malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) d[i] = -1;
    int head = 0, tail = 0, dr[4] = {1, -1, 0, 0}, dc[4] = {0, 0, 1, -1};
    d[sr * g->cols + sc] = 0;
    q[tail++] = sr * g->cols + sc;
    while (head < tail) {
        int u = q[head++], r = u / g->cols, c = u % g->cols;
        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k], nc = c + dc[k];
            if (!grid_is_walkable(g, nr, nc)) continue;
            int v = nr * g->cols + nc;
            if (d[v] == -1) { d[v] = d[u] + 1; q[tail++] = v; }
        }
    }
    int res = d[tr * g->cols + tc];
    free(d); free(q);
    return res;
}

int main(void) {
    Grid g;
    int path[64], len;
    ASSERT_EQ(grid_load("data/small_grid.txt", &g), 0);

    /* charger (0,0) -> shelf (2,5): 7 steps = 8 cells (hand-checked) */
    ASSERT_EQ(grid_shortest_path(&g, 0, 0, 2, 5, path, 64, &len), 0);
    ASSERT_EQ(len, 8);
    ASSERT_EQ(path[0], 0);
    ASSERT_EQ(path[len - 1], 2 * 8 + 5);

    /* charger -> second shelf (4,6): 10 steps */
    ASSERT_EQ(grid_shortest_path(&g, 0, 0, 4, 6, path, 64, &len), 0);
    ASSERT_EQ(len, 11);

    ASSERT_EQ(grid_shortest_path(&g, 0, 0, 0, 0, path, 64, &len), 0);   /* same cell */
    ASSERT_EQ(len, 1);
    ASSERT_EQ(grid_shortest_path(&g, 0, 3, 2, 5, path, 64, &len), -1);  /* start is '#' */
    ASSERT_EQ(grid_shortest_path(&g, 0, 0, 2, 5, path, 3, &len), -3);   /* buffer too small */
    grid_free(&g);

    /* a wall splits the map: no route */
    char wall[] = ".#..#..#.";
    Grid w;
    w.rows = 3; w.cols = 3; w.cells = wall;
    ASSERT_EQ(grid_shortest_path(&w, 0, 0, 0, 2, path, 64, &len), -2);

    /* random grids: Dijkstra route length must equal BFS */
    srand(33);
    for (int t = 0; t < 300; t++) {
        char cells[64];
        Grid rg;
        rg.rows = 1 + rand() % 8; rg.cols = 1 + rand() % 8; rg.cells = cells;
        for (int i = 0; i < rg.rows * rg.cols; i++) cells[i] = (rand() % 4 == 0) ? '#' : '.';
        int sr = rand() % rg.rows, sc = rand() % rg.cols;
        int tr = rand() % rg.rows, tc = rand() % rg.cols;
        if (!grid_is_walkable(&rg, sr, sc) || !grid_is_walkable(&rg, tr, tc)) continue;

        int expect = bfs_dist(&rg, sr, sc, tr, tc);
        int rc = grid_shortest_path(&rg, sr, sc, tr, tc, path, 64, &len);
        if (expect < 0) {
            ASSERT_EQ(rc, -2);
        } else {
            ASSERT_EQ(rc, 0);
            ASSERT_EQ(len - 1, expect);
        }
    }
    TEST_SUMMARY();
}