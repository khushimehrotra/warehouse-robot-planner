#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gridpath.h"

int grid_to_graph(const Grid *g, Graph *out) {
    if (g == NULL || g->rows <= 0 || g->cols <= 0) return -1;
    if (graph_init(out, g->rows * g->cols) != 0) return -3;

    for (int r = 0; r < g->rows; r++)
        for (int c = 0; c < g->cols; c++) {
            if (!grid_is_walkable(g, r, c)) continue;
            int id = r * g->cols + c;
            /* only look right and down, so each edge is added exactly once */
            if (grid_is_walkable(g, r, c + 1) &&
                graph_add_undirected(out, id, id + 1, 1) != 0) { graph_free(out); return -3; }
            if (grid_is_walkable(g, r + 1, c) &&
                graph_add_undirected(out, id, id + g->cols, 1) != 0) { graph_free(out); return -3; }
        }
    return 0;
}

int grid_find(const Grid *g, char ch, int *r, int *c) {
    for (int rr = 0; rr < g->rows; rr++)
        for (int cc = 0; cc < g->cols; cc++)
            if (grid_get(g, rr, cc) == ch) { *r = rr; *c = cc; return 0; }
    return -1;
}

int grid_shortest_path(const Grid *g, int sr, int sc, int tr, int tc,
                       int *path, int cap, int *len) {
    *len = 0;
    if (!grid_is_walkable(g, sr, sc) || !grid_is_walkable(g, tr, tc)) return -1;

    Graph gr;
    if (grid_to_graph(g, &gr) != 0) return -3;
    int n = g->rows * g->cols, rc;
    int *dist = malloc((size_t)n * sizeof(int));
    int *prev = malloc((size_t)n * sizeof(int));
    if (dist == NULL || prev == NULL) {
        free(dist); free(prev); graph_free(&gr);
        return -3;
    }
    int src = sr * g->cols + sc, dst = tr * g->cols + tc;

    if (dijkstra(&gr, src, dist, prev) != 0) {
        rc = -3;
    } else {
        int l = path_extract(prev, src, dst, path, cap);
        if (l == -1) rc = -2;                          /* walled off */
        else if (l == -2) rc = -3;                     /* buffer too small */
        else { *len = l; rc = 0; }
    }
    free(dist); free(prev); graph_free(&gr);
    return rc;
}

void grid_print_path(const Grid *g, const int *path, int len) {
    size_t sz = (size_t)g->rows * g->cols;
    char *buf = malloc(sz);
    if (buf == NULL) return;
    memcpy(buf, g->cells, sz);
    for (int i = 0; i < len; i++)                      /* never overwrite C or S */
        if (buf[path[i]] == '.')
            buf[path[i]] = (i == 0) ? 'A' : (i == len - 1 ? 'B' : '*');
    for (int r = 0; r < g->rows; r++) {
        for (int c = 0; c < g->cols; c++) putchar(buf[r * g->cols + c]);
        putchar('\n');
    }
    free(buf);
}