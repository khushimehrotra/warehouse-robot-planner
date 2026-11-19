#ifndef BACKTRACK_H
#define BACKTRACK_H

#define NQ_MAX_N 16

/* Count all N-Queens solutions on an n x n board (1 <= n <= NQ_MAX_N).
   first (may be NULL, else n ints) receives one solution: first[row] = column.
   Return: 0 ok, -1 bad n, -3 out of memory. */
int nqueens(int n, long long *count, int *first);

/* Subset sum over non-negative items (0 <= n <= 64).
   *count = number of subsets (by position) whose sum equals target.
   chosen (may be NULL, else n bytes) receives the first subset found (all 0 if none).
   nodes (may be NULL) = search-tree nodes visited.
   Return: 0 ok, -1 bad arguments (negative item or target), -3 out of memory. */
int subset_sum(const int *a, int n, int target, char *chosen,
               long long *count, long long *nodes);

#endif