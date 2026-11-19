#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "dp.h"

/* ---------- 0/1 knapsack: O(n * cap) time and memory ---------- */
int knapsack01(const int *w, const int *v, int n, int cap, long long *best, char *chosen) {
    *best = 0;
    if (n < 0 || cap < 0 || (n > 0 && (w == NULL || v == NULL))) return -1;
    for (int i = 0; i < n; i++)
        if (w[i] < 0 || v[i] < 0) return -1;

    size_t cols = (size_t)cap + 1, rows = (size_t)n + 1;
    if (rows > SIZE_MAX / cols / sizeof(long long)) return -3;
    long long *T = calloc(rows * cols, sizeof(long long));   /* row 0 = no items = 0 */
    if (T == NULL) return -3;

    for (int i = 1; i <= n; i++)
        for (int c = 0; c <= cap; c++) {
            long long best_here = T[(size_t)(i - 1) * cols + c];          /* skip item i */
            if (w[i - 1] <= c) {                                          /* or take it */
                long long take = T[(size_t)(i - 1) * cols + (c - w[i - 1])] + v[i - 1];
                if (take > best_here) best_here = take;
            }
            T[(size_t)i * cols + c] = best_here;
        }
    *best = T[(size_t)n * cols + cap];

    if (chosen != NULL) {              /* walk back: if the value changed, item i was taken */
        int c = cap;
        for (int i = n; i >= 1; i--) {
            if (T[(size_t)i * cols + c] != T[(size_t)(i - 1) * cols + c]) {
                chosen[i - 1] = 1;
                c -= w[i - 1];
            } else {
                chosen[i - 1] = 0;
            }
        }
    }
    free(T);
    return 0;
}

/* ---------- matrix chain: O(n^3) time, O(n^2) memory ---------- */
static int build_paren(const int *s, size_t N, int i, int j, char *buf, int cap, int *pos) {
    if (i == j) {
        char tmp[16];
        int l = snprintf(tmp, sizeof tmp, "A%d", i);
        if (*pos + l >= cap) return -1;                 /* keep room for the final NUL */
        memcpy(buf + *pos, tmp, (size_t)l);
        *pos += l;
        return 0;
    }
    if (*pos + 1 >= cap) return -1;
    buf[(*pos)++] = '(';
    int k = s[(size_t)i * N + j];                       /* best split point */
    if (build_paren(s, N, i, k, buf, cap, pos) != 0) return -1;
    if (build_paren(s, N, k + 1, j, buf, cap, pos) != 0) return -1;
    if (*pos + 1 >= cap) return -1;
    buf[(*pos)++] = ')';
    return 0;
}

int matrix_chain(const int *p, int n, long long *cost, char *paren, int cap) {
    *cost = 0;
    if (n < 1 || n > 3000 || p == NULL) return -1;
    for (int i = 0; i <= n; i++)
        if (p[i] <= 0) return -1;

    size_t N = (size_t)n;
    long long *m = calloc(N * N, sizeof(long long));    /* m[i][i] = 0 */
    int *s = calloc(N * N, sizeof(int));
    if (m == NULL || s == NULL) { free(m); free(s); return -3; }

    for (int len = 2; len <= n; len++)                  /* chain length, shortest first */
        for (int i = 0; i + len <= n; i++) {
            int j = i + len - 1;
            m[i * N + j] = LLONG_MAX;
            for (int k = i; k < j; k++) {
                long long q = m[i * N + k] + m[(size_t)(k + 1) * N + j]
                            + (long long)p[i] * p[k + 1] * p[j + 1];
                if (q < m[i * N + j]) { m[i * N + j] = q; s[i * N + j] = k; }
            }
        }
    *cost = m[0 * N + (N - 1)];

    int rc = 0;
    if (paren != NULL) {
        int pos = 0;
        if (cap < 1 || build_paren(s, N, 0, n - 1, paren, cap, &pos) != 0) rc = -4;
        else paren[pos] = '\0';
    }
    free(m); free(s);
    return rc;
}

/* ---------- LCS: O(la * lb) ---------- */
int lcs(const char *a, const char *b, int *len, char *out, int outcap) {
    *len = 0;
    if (a == NULL || b == NULL) return -1;
    size_t la = strlen(a), lb = strlen(b), cols = lb + 1;
    if (la + 1 > SIZE_MAX / cols / sizeof(int)) return -3;
    int *T = calloc((la + 1) * cols, sizeof(int));      /* row 0 and column 0 stay 0 */
    if (T == NULL) return -3;

    for (size_t i = 1; i <= la; i++)
        for (size_t j = 1; j <= lb; j++) {
            if (a[i - 1] == b[j - 1]) {
                T[i * cols + j] = T[(i - 1) * cols + (j - 1)] + 1;
            } else {
                int up = T[(i - 1) * cols + j], left = T[i * cols + (j - 1)];
                T[i * cols + j] = up > left ? up : left;
            }
        }
    int L = T[la * cols + lb];
    int rc = 0;

    if (out != NULL) {
        if (outcap < L + 1) {
            rc = -4;
        } else {
            out[L] = '\0';
            size_t i = la, j = lb;
            int k = L;
            while (i > 0 && j > 0) {                    /* walk back from the corner */
                if (a[i - 1] == b[j - 1]) { out[--k] = a[i - 1]; i--; j--; }
                else if (T[(i - 1) * cols + j] >= T[i * cols + (j - 1)]) i--;
                else j--;
            }
        }
    }
    *len = L;
    free(T);
    return rc;
}

/* ---------- optimal BST: O(n^3) time, O(n^2) memory ---------- */
int optimal_bst(const int *freq, int n, long long *cost, int *root) {
    *cost = 0;
    if (n < 0 || n > 3000 || (n > 0 && freq == NULL)) return -1;
    for (int i = 0; i < n; i++)
        if (freq[i] < 0) return -1;
    if (n == 0) return 0;

    size_t W = (size_t)n + 1;
    long long *c = calloc(W * W, sizeof(long long));    /* c[i][i] = 0 (empty range) */
    long long *pre = malloc(W * sizeof(long long));     /* pre[i] = freq[0] + ... + freq[i-1] */
    if (c == NULL || pre == NULL) { free(c); free(pre); return -3; }
    pre[0] = 0;
    for (int i = 0; i < n; i++) pre[i + 1] = pre[i] + freq[i];

    for (int len = 1; len <= n; len++)
        for (int i = 0; i + len <= n; i++) {
            int j = i + len;
            long long best = LLONG_MAX;
            int best_r = i;
            for (int r = i; r < j; r++) {               /* try every key in [i, j) as root */
                long long t = c[i * W + r] + c[(size_t)(r + 1) * W + j];
                if (t < best) { best = t; best_r = r; }
            }
            c[i * W + j] = best + (pre[j] - pre[i]);
            if (root != NULL) root[i * W + j] = best_r;
        }
    *cost = c[0 * W + n];
    free(c); free(pre);
    return 0;
}