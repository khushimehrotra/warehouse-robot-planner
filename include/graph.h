#ifndef GRAPH_H
#define GRAPH_H
#include <limits.h>

#define DIST_INF INT_MAX      /* "unreachable" */

/* Forward-star graph: arc a goes to[a] with weight w[a];
   next[a] is the next arc leaving the same node; head[u] is the first. */
typedef struct {
    int n;                    /* number of nodes */
    int m, cap;               /* arcs stored, arcs that fit */
    int *head, *next, *to, *w;
} Graph;

typedef struct { int u, v, w; } WEdge;

/* Return codes everywhere: 0 ok, -1 bad arguments, -2 negative weight (Dijkstra),
   -3 out of memory. */
int  graph_init(Graph *g, int n);
int  graph_add_edge(Graph *g, int u, int v, int w);        /* directed arc u -> v */
int  graph_add_undirected(Graph *g, int u, int v, int w);  /* two arcs */
void graph_free(Graph *g);

/* Minimum spanning tree. mst must have room for n-1 edges.
   If the graph is disconnected, *mst_count < n-1 (a spanning forest). */
int kruskal(int n, const WEdge *e, int m, WEdge *mst, int *mst_count, long long *total);

/* Prim from 'start'. parent[] gets the tree (-1 for start and unreached nodes).
   *visited = nodes reached; if < n the graph is disconnected. */
int prim(const Graph *g, int start, int *parent, long long *total, int *visited);

/* Shortest distances from src. dist[v] = DIST_INF if unreachable.
   dist and prev must have n ints. */
int dijkstra(const Graph *g, int src, int *dist, int *prev);

/* Rebuild the route src..dst from prev[]. Fills path[], returns its length (nodes).
   -1 = unreachable, -2 = path buffer too small. */
int path_extract(const int *prev, int src, int dst, int *path, int cap);

#endif