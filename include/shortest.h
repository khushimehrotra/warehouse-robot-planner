#ifndef SHORTEST_H
#define SHORTEST_H
#include <limits.h>
#include "graph.h"

#define FW_INF (LLONG_MAX / 4)       /* "unreachable" for Floyd-Warshall */

/* Single-source shortest paths, negative edges allowed. Weights should be modest
   (|w| * n must fit in an int). dist and prev must hold n ints.
   dist[v] = DIST_INF if unreachable.
   Return: 0 ok, 1 negative cycle reachable from src (distances not valid),
   -1 bad arguments. */
int bellman_ford(const Graph *g, int src, int *dist, int *prev);

/* All-pairs shortest paths. d must hold n*n long longs (d[i*n+j]).
   next (may be NULL) must hold n*n ints: first step from i toward j, -1 if none.
   Return: 0 ok, 1 negative cycle exists, -1 bad arguments. */
int floyd_warshall(const Graph *g, long long *d, int *next);

/* Rebuild the route i..j from next[]. Fills path[], returns number of nodes.
   -1 = no route, -2 = cap too small. Only valid when floyd_warshall returned 0. */
int fw_path(const int *next, int n, int i, int j, int *path, int cap);

#endif