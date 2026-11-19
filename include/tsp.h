#ifndef TSP_H
#define TSP_H

#define TSP_MAX_N 16            /* cities, including the start */
#define TSP_INF   1000000000    /* put this in dist[] for "no direct route" */

/* Shortest round trip starting and ending at city 0, visiting every city once.
   dist[i*n+j] = cost from i to j (non-negative; may be asymmetric).
   tour[] must hold n ints and receives the visiting order, starting with 0.
   *cost = total cost of the round trip.
   Return: 0 ok, -1 bad arguments (n < 1 or n > TSP_MAX_N), -2 no tour exists,
   -3 out of memory. */
int tsp_held_karp(const int *dist, int n, int *tour, long long *cost);

#endif