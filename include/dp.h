#ifndef DP_H
#define DP_H

/* 0/1 knapsack. w[i], v[i] >= 0, cap >= 0.
   *best = maximum value. chosen[i] = 1 if item i is taken (chosen may be NULL, else n entries).
   Return: 0 ok, -1 bad arguments, -3 out of memory. */
int knapsack01(const int *w, const int *v, int n, int cap, long long *best, char *chosen);

/* Matrix chain. Matrix i is p[i] x p[i+1]; p has n+1 positive entries, 1 <= n <= 3000.
   *cost = fewest scalar multiplications.
   paren (may be NULL, size cap) receives e.g. "((A0A1)A2)".
   Return: 0 ok, -1 bad arguments, -3 out of memory, -4 paren buffer too small. */
int matrix_chain(const int *p, int n, long long *cost, char *paren, int cap);

/* Longest common subsequence of two NUL-terminated strings.
   *len = its length. out (may be NULL) receives one LCS and needs len+1 bytes.
   Return: 0 ok, -1 bad arguments, -3 out of memory, -4 out buffer too small. */
int lcs(const char *a, const char *b, int *len, char *out, int outcap);

/* Optimal binary search tree over n sorted keys, freq[i] >= 0, 0 <= n <= 3000.
   *cost = sum of freq[i] * depth(i), root at depth 1.
   root (may be NULL) must hold (n+1)*(n+1) ints: root[i*(n+1)+j] is the best root
   key for the key range [i, j). The whole tree's root is root[0*(n+1)+n].
   Return: 0 ok, -1 bad arguments, -3 out of memory. */
int optimal_bst(const int *freq, int n, long long *cost, int *root);

#endif