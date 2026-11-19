#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "dp.h"

static long long brute_knap(const int *w, const int *v, int n, int cap) {
    long long best = 0;
    for (int mask = 0; mask < (1 << n); mask++) {
        long long wt = 0, val = 0;
        for (int i = 0; i < n; i++)
            if ((mask >> i) & 1) { wt += w[i]; val += v[i]; }
        if (wt <= cap && val > best) best = val;
    }
    return best;
}

static long long brute_chain(const int *p, int i, int j) {
    if (i == j) return 0;
    long long best = -1;
    for (int k = i; k < j; k++) {
        long long c = brute_chain(p, i, k) + brute_chain(p, k + 1, j)
                    + (long long)p[i] * p[k + 1] * p[j + 1];
        if (best < 0 || c < best) best = c;
    }
    return best;
}

static int is_subseq(const char *s, const char *t) {
    while (*t && *s) { if (*s == *t) s++; t++; }
    return *s == '\0';
}

static int brute_lcs(const char *a, const char *b) {
    int la = (int)strlen(a), best = 0;
    for (int mask = 0; mask < (1 << la); mask++) {
        char sub[16];
        int k = 0;
        for (int i = 0; i < la; i++)
            if ((mask >> i) & 1) sub[k++] = a[i];
        sub[k] = '\0';
        if (k > best && is_subseq(sub, b)) best = k;
    }
    return best;
}

static long long brute_bst(const int *f, int i, int j) {
    if (i >= j) return 0;
    long long sum = 0, best = -1;
    for (int k = i; k < j; k++) sum += f[k];
    for (int r = i; r < j; r++) {
        long long c = brute_bst(f, i, r) + brute_bst(f, r + 1, j);
        if (best < 0 || c < best) best = c;
    }
    return best + sum;
}

static long long cost_from_root(const int *f, const int *root, int n, int i, int j, int depth) {
    if (i >= j) return 0;
    int r = root[i * (n + 1) + j];
    return (long long)f[r] * depth + cost_from_root(f, root, n, i, r, depth + 1)
                                   + cost_from_root(f, root, n, r + 1, j, depth + 1);
}

int main(void) {
    long long best;
    char chosen[16];

    /* ---- knapsack: known cases ---- */
    int w1[] = {1, 3, 4, 5}, v1[] = {1, 4, 5, 7};
    ASSERT_EQ(knapsack01(w1, v1, 4, 7, &best, chosen), 0);
    ASSERT_EQ(best, 9);
    ASSERT_EQ(chosen[0], 0); ASSERT_EQ(chosen[1], 1);
    ASSERT_EQ(chosen[2], 1); ASSERT_EQ(chosen[3], 0);

    int w2[] = {10, 20, 30}, v2[] = {60, 100, 120};      /* greedy by ratio gives 160 */
    ASSERT_EQ(knapsack01(w2, v2, 3, 50, &best, NULL), 0);
    ASSERT_EQ(best, 220);

    ASSERT_EQ(knapsack01(NULL, NULL, 0, 10, &best, NULL), 0);   /* no items */
    ASSERT_EQ(best, 0);
    ASSERT_EQ(knapsack01(w1, v1, 4, 0, &best, NULL), 0);        /* capacity 0 */
    ASSERT_EQ(best, 0);
    ASSERT_EQ(knapsack01(w1, v1, 4, -1, &best, NULL), -1);
    int wneg[] = {-1}, vone[] = {5};
    ASSERT_EQ(knapsack01(wneg, vone, 1, 5, &best, NULL), -1);

    srand(41);
    for (int t = 0; t < 500; t++) {
        int n = rand() % 11, cap = rand() % 31, w[12], v[12];
        for (int i = 0; i < n; i++) { w[i] = rand() % 11; v[i] = rand() % 21; }
        ASSERT_EQ(knapsack01(w, v, n, cap, &best, chosen), 0);
        ASSERT_EQ(best, brute_knap(w, v, n, cap));
        long long wt = 0, val = 0;                        /* the chosen set must be real */
        for (int i = 0; i < n; i++) if (chosen[i]) { wt += w[i]; val += v[i]; }
        ASSERT_EQ(wt <= cap, 1);
        ASSERT_EQ(val, best);
    }

    /* ---- matrix chain: textbook example ---- */
    int p1[] = {30, 35, 15, 5, 10, 20, 25};
    long long cost;
    char paren[64], tiny[4];
    ASSERT_EQ(matrix_chain(p1, 6, &cost, paren, 64), 0);
    ASSERT_EQ(cost, 15125);
    ASSERT_EQ(strcmp(paren, "((A0(A1A2))((A3A4)A5))"), 0);
    ASSERT_EQ(matrix_chain(p1, 6, &cost, tiny, 4), -4);   /* buffer too small */
    ASSERT_EQ(matrix_chain(p1, 1, &cost, paren, 64), 0);  /* one matrix: no work */
    ASSERT_EQ(cost, 0);
    ASSERT_EQ(strcmp(paren, "A0"), 0);
    ASSERT_EQ(matrix_chain(p1, 0, &cost, NULL, 0), -1);
    int pbad[] = {3, 0, 4};
    ASSERT_EQ(matrix_chain(pbad, 2, &cost, NULL, 0), -1);

    for (int t = 0; t < 300; t++) {
        int n = 1 + rand() % 7, p[9];
        for (int i = 0; i <= n; i++) p[i] = 1 + rand() % 10;
        ASSERT_EQ(matrix_chain(p, n, &cost, NULL, 0), 0);
        ASSERT_EQ(cost, brute_chain(p, 0, n - 1));
    }

    /* ---- LCS ---- */
    int len;
    char out[32];
    ASSERT_EQ(lcs("ABCBDAB", "BDCABA", &len, out, 32), 0);
    ASSERT_EQ(len, 4);
    ASSERT_EQ(is_subseq(out, "ABCBDAB") && is_subseq(out, "BDCABA"), 1);
    ASSERT_EQ(lcs("", "abc", &len, out, 32), 0);
    ASSERT_EQ(len, 0); ASSERT_EQ(out[0], '\0');
    ASSERT_EQ(lcs("abc", "xyz", &len, out, 32), 0);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(lcs("abc", "abc", &len, out, 2), -4);       /* needs 4 bytes */
    ASSERT_EQ(lcs(NULL, "a", &len, out, 32), -1);

    for (int t = 0; t < 500; t++) {
        char a[16], b[16];
        int la = rand() % 11, lb = rand() % 13;
        for (int i = 0; i < la; i++) a[i] = (char)('a' + rand() % 3);
        for (int i = 0; i < lb; i++) b[i] = (char)('a' + rand() % 3);
        a[la] = b[lb] = '\0';
        ASSERT_EQ(lcs(a, b, &len, out, 32), 0);
        ASSERT_EQ(len, brute_lcs(a, b));
        ASSERT_EQ((int)strlen(out), len);
        ASSERT_EQ(is_subseq(out, a) && is_subseq(out, b), 1);
    }

    /* ---- optimal BST: keys with counts 34, 8, 50 -> root is key 2, cost 142 ---- */
    int f1[] = {34, 8, 50}, root[16];
    ASSERT_EQ(optimal_bst(f1, 3, &cost, root), 0);
    ASSERT_EQ(cost, 142);
    ASSERT_EQ(root[0 * 4 + 3], 2);
    ASSERT_EQ(optimal_bst(NULL, 0, &cost, NULL), 0);
    ASSERT_EQ(cost, 0);
    ASSERT_EQ(optimal_bst(f1, -1, &cost, NULL), -1);

    for (int t = 0; t < 300; t++) {
        int n = rand() % 8, f[8], rt[81];
        for (int i = 0; i < n; i++) f[i] = rand() % 20;
        ASSERT_EQ(optimal_bst(f, n, &cost, rt), 0);
        ASSERT_EQ(cost, brute_bst(f, 0, n));
        if (n > 0) ASSERT_EQ(cost_from_root(f, rt, n, 0, n, 1), cost);  /* tree matches cost */
    }
    TEST_SUMMARY();
}