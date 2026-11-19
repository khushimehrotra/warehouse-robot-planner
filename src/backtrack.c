#include <stdlib.h>
#include <string.h>
#include "backtrack.h"

/* ---------- N-Queens ----------
   Place one queen per row. A column or diagonal already used is rejected.
   col[c], d1[r+c] and d2[r-c+n-1] mark what is taken. */
typedef struct {
    int n, have;
    int *pos, *first;
    char *col, *d1, *d2;
    long long count;
} NQ;

static void nq_place(NQ *s, int r) {
    if (r == s->n) {                                   /* all rows filled: a solution */
        s->count++;
        if (s->first != NULL && !s->have) {
            memcpy(s->first, s->pos, (size_t)s->n * sizeof(int));
            s->have = 1;
        }
        return;
    }
    for (int c = 0; c < s->n; c++) {
        int a = r + c, b = r - c + s->n - 1;
        if (s->col[c] || s->d1[a] || s->d2[b]) continue;   /* attacked: skip */
        s->col[c] = s->d1[a] = s->d2[b] = 1;               /* choose */
        s->pos[r] = c;
        nq_place(s, r + 1);                                /* recurse */
        s->col[c] = s->d1[a] = s->d2[b] = 0;               /* undo */
    }
}

int nqueens(int n, long long *count, int *first) {
    *count = 0;
    if (n < 1 || n > NQ_MAX_N) return -1;
    NQ s;
    s.n = n; s.have = 0; s.first = first; s.count = 0;
    s.pos = malloc((size_t)n * sizeof(int));
    s.col = calloc((size_t)n, 1);
    s.d1 = calloc((size_t)2 * n, 1);
    s.d2 = calloc((size_t)2 * n, 1);
    if (!s.pos || !s.col || !s.d1 || !s.d2) {
        free(s.pos); free(s.col); free(s.d1); free(s.d2);
        return -3;
    }
    nq_place(&s, 0);
    *count = s.count;
    free(s.pos); free(s.col); free(s.d1); free(s.d2);
    return 0;
}

/* ---------- Subset sum ----------
   Decide item i: include it (only if the sum stays <= target) or skip it.
   Prune when even taking every remaining item cannot reach the target. */
typedef struct {
    const int *a;
    int n, have;
    long long target, count, nodes;
    long long *suf;                 /* suf[i] = a[i] + ... + a[n-1] */
    char *cur, *out;
} SS;

static void ss_rec(SS *s, int i, long long sum) {
    s->nodes++;
    if (sum + s->suf[i] < s->target) return;           /* can never reach the target */
    if (i == s->n) {                                   /* here sum == target */
        s->count++;
        if (s->out != NULL && !s->have) {
            memcpy(s->out, s->cur, (size_t)s->n);
            s->have = 1;
        }
        return;
    }
    if (sum + s->a[i] <= s->target) {                  /* include item i */
        s->cur[i] = 1;
        ss_rec(s, i + 1, sum + s->a[i]);
        s->cur[i] = 0;
    }
    ss_rec(s, i + 1, sum);                             /* skip item i */
}

int subset_sum(const int *a, int n, int target, char *chosen,
               long long *count, long long *nodes) {
    *count = 0;
    if (nodes) *nodes = 0;
    if (n < 0 || n > 64 || target < 0 || (n > 0 && a == NULL)) return -1;
    for (int i = 0; i < n; i++)
        if (a[i] < 0) return -1;

    SS s;
    s.a = a; s.n = n; s.have = 0; s.target = target;
    s.count = 0; s.nodes = 0; s.out = chosen;
    s.suf = malloc((size_t)(n + 1) * sizeof(long long));
    s.cur = calloc((size_t)n + 1, 1);
    if (!s.suf || !s.cur) { free(s.suf); free(s.cur); return -3; }
    s.suf[n] = 0;
    for (int i = n - 1; i >= 0; i--) s.suf[i] = s.suf[i + 1] + a[i];
    if (chosen != NULL && n > 0) memset(chosen, 0, (size_t)n);

    ss_rec(&s, 0, 0);
    *count = s.count;
    if (nodes) *nodes = s.nodes;
    free(s.suf); free(s.cur);
    return 0;
}