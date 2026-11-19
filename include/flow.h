#ifndef FLOW_H
#define FLOW_H
#include "graph.h"

/* Add edge u -> v with capacity cap >= 0 (use ONLY this, on a fresh Graph).
   Return 0 ok, -1 bad arguments, -3 out of memory. */
int flow_add_edge(Graph *g, int u, int v, int cap);

/* Maximum flow s -> t (Edmonds-Karp). Modifies the residual capacities in g.
   Return: 0 ok, -1 bad arguments (or s == t), -3 out of memory. */
int max_flow(Graph *g, int s, int t, long long *flow);

/* After max_flow: side[v] = 1 if v is on the source side of a minimum cut.
   side must hold g->n bytes. Return 0 ok, -1 bad arguments, -3 out of memory. */
int flow_min_cut(const Graph *g, int s, char *side);

/* Maximum bipartite matching by augmenting paths (Kuhn), O(V*E).
   Left nodes 0..nl-1, right nodes 0..nr-1, edge k joins eu[k] and ev[k].
   match_l has nl ints, match_r has nr ints (-1 = unmatched). *size = pairs found.
   Return 0 ok, -1 bad arguments, -3 out of memory. */
int bipartite_matching(int nl, int nr, const int *eu, const int *ev, int m,
                       int *match_l, int *match_r, int *size);

/* Same answer through the reduction to max flow
   (source -> left -> right -> sink, all capacities 1). */
int matching_by_flow(int nl, int nr, const int *eu, const int *ev, int m, int *size);

#endif