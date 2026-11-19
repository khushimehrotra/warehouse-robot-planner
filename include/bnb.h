#ifndef BNB_H
#define BNB_H

#define BNB_MAX_N        12
#define BNB_MAX_COST     1000000
#define BNB_NODE_LIMIT   2000000     /* safety cap so FIFO cannot eat all memory */

typedef enum { BNB_FIFO = 0, BNB_LIFO = 1, BNB_LC = 2 } BnbMode;

typedef struct {
    long long generated;   /* child nodes created */
    long long expanded;    /* nodes whose children were generated */
    long long pruned;      /* nodes discarded by the bound */
} BnbStats;

/* Exact assignment of n robots (rows) to n distinct tasks (columns).
   cost[i*n+j] in 0..BNB_MAX_COST. assign[i] = task given to robot i (n ints).
   *total = minimum total cost. st may be NULL.
   Return: 0 ok, -1 bad arguments, -3 out of memory, -4 node limit reached. */
int assign_bnb(const int *cost, int n, BnbMode mode,
               int *assign, long long *total, BnbStats *st);

const char *bnb_name(BnbMode mode);

#endif