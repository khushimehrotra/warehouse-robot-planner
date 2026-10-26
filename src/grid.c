/* src/grid.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"

#define MAX_COLS 1000

int grid_load(const char *path, Grid *g) {
    g->rows = g->cols = 0;
    g->cells = NULL;

    FILE *f = fopen(path, "r");
    if (f == NULL) return -1;

    int rows, cols;
    if (fscanf(f, "%d %d", &rows, &cols) != 2 || rows <= 0 || cols <= 0 || cols > MAX_COLS) {
        fclose(f);
        return -2;
    }

    g->cells = malloc((size_t)rows * cols);
    if (g->cells == NULL) { fclose(f); return -3; }

    char line[MAX_COLS + 2];
    for (int r = 0; r < rows; r++) {
        if (fscanf(f, "%1001s", line) != 1 || (int)strlen(line) != cols) {
            free(g->cells); g->cells = NULL; fclose(f);
            return -4;
        }
        for (int c = 0; c < cols; c++) {
            char ch = line[c];
            if (ch != '.' && ch != '#' && ch != 'S' && ch != 'C') {
                free(g->cells); g->cells = NULL; fclose(f);
                return -5;
            }
            g->cells[r * cols + c] = ch;   /* 2D position -> 1D index */
        }
    }
    fclose(f);
    g->rows = rows;
    g->cols = cols;
    return 0;
}

void grid_free(Grid *g) {
    free(g->cells);
    g->cells = NULL;
    g->rows = g->cols = 0;
}

int grid_in_bounds(const Grid *g, int r, int c) {
    return r >= 0 && r < g->rows && c >= 0 && c < g->cols;
}

char grid_get(const Grid *g, int r, int c) {
    return g->cells[r * g->cols + c];
}

int grid_is_walkable(const Grid *g, int r, int c) {
    return grid_in_bounds(g, r, c) && grid_get(g, r, c) != '#';
}

void grid_print(const Grid *g) {
    for (int r = 0; r < g->rows; r++) {
        for (int c = 0; c < g->cols; c++) putchar(grid_get(g, r, c));
        putchar('\n');
    }
}