#ifndef GRIDPATH_H
#define GRIDPATH_H
#include "grid.h"
#include "graph.h"

/* Cell (r,c) -> node r*cols+c. Edges join walkable 4-neighbours, weight 1.
   Return 0 ok, -1 bad grid, -3 out of memory. Caller must graph_free(out). */
int grid_to_graph(const Grid *g, Graph *out);

/* First cell (row by row) holding 'ch'. Return 0 found, -1 not found. */
int grid_find(const Grid *g, char ch, int *r, int *c);

/* Shortest route from (sr,sc) to (tr,tc) as cell numbers in path[0..*len-1].
   Return: 0 ok, -1 start or target blocked / outside, -2 no route exists,
   -3 out of memory or path buffer (cap) too small. Use cap >= rows*cols to be safe. */
int grid_shortest_path(const Grid *g, int sr, int sc, int tr, int tc,
                       int *path, int cap, int *len);

/* Print the map with the route drawn as '*' (A = start, B = end on plain floor). */
void grid_print_path(const Grid *g, const int *path, int len);

#endif