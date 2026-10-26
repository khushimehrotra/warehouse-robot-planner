/* include/grid.h */
#ifndef GRID_H
#define GRID_H

typedef struct {
    int rows, cols;
    char *cells;     /* rows*cols characters in one block */
} Grid;

/* Return codes: 0 ok, -1 cannot open file, -2 bad header,
   -3 out of memory, -4 bad row, -5 bad character */
int  grid_load(const char *path, Grid *g);
void grid_free(Grid *g);
char grid_get(const Grid *g, int r, int c);
int  grid_in_bounds(const Grid *g, int r, int c);
int  grid_is_walkable(const Grid *g, int r, int c);  /* in bounds and not '#' */
void grid_print(const Grid *g);

#endif