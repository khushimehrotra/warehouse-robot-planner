#include <stdlib.h>
#include <string.h>
#include "strmatch.h"
#include "dnc.h"

#define RK_BASE 256ULL
#define RK_MOD  1000000007ULL

static int check(const char *t, size_t n, const char *p, size_t m,
                 size_t *pos, size_t cap, size_t *count) {
    if (count == NULL) return -1;
    *count = 0;
    if (p == NULL || m == 0 || (t == NULL && n > 0) || (pos == NULL && cap > 0)) return -1;
    return 0;
}

static void record(size_t *pos, size_t cap, size_t *count, size_t at) {
    if (*count < cap) pos[*count] = at;
    (*count)++;
}

/* ---------- naive: O(n*m) worst case ---------- */
int match_naive(const char *t, size_t n, const char *p, size_t m,
                size_t *pos, size_t cap, size_t *count) {
    int rc = check(t, n, p, m, pos, cap, count);
    if (rc != 0) return rc;
    if (m > n) return 0;
    for (size_t i = 0; i + m <= n; i++) {
        size_t j = 0;
        while (j < m && t[i + j] == p[j]) j++;
        if (j == m) record(pos, cap, count, i);
    }
    return 0;
}

/* ---------- Rabin-Karp: O(n + m) expected ---------- */
int match_rabin_karp_mod(const char *t, size_t n, const char *p, size_t m,
                         size_t *pos, size_t cap, size_t *count,
                         unsigned long long q) {
    int rc = check(t, n, p, m, pos, cap, count);
    if (rc != 0) return rc;
    if (q < 1 || q > 4294967295ULL) return -1;
    if (m > n) return 0;

    unsigned long long h, ph = 0, th = 0;               /* h = 256^(m-1) mod q */
    if (power_mod(RK_BASE, (unsigned long long)(m - 1), q, &h) != 0) return -1;
    for (size_t i = 0; i < m; i++) {
        ph = (ph * RK_BASE + (unsigned char)p[i]) % q;
        th = (th * RK_BASE + (unsigned char)t[i]) % q;
    }
    for (size_t i = 0;; i++) {
        if (ph == th && memcmp(t + i, p, m) == 0)        /* verify: hashes may collide */
            record(pos, cap, count, i);
        if (i + m >= n) break;
        /* slide: drop t[i], add t[i+m] */
        th = (th + q - ((unsigned char)t[i] * h) % q) % q;
        th = (th * RK_BASE + (unsigned char)t[i + m]) % q;
    }
    return 0;
}

int match_rabin_karp(const char *t, size_t n, const char *p, size_t m,
                     size_t *pos, size_t cap, size_t *count) {
    return match_rabin_karp_mod(t, n, p, m, pos, cap, count, RK_MOD);
}

/* ---------- finite automaton: O(m*256) build, O(n) search ----------
   d[state*256 + c] = next state. State j means "the last j characters match p[0..j-1]".
   x is the state we would be in after reading p[1..j-1]; row j starts as a copy of row x. */
int match_automaton(const char *t, size_t n, const char *p, size_t m,
                    size_t *pos, size_t cap, size_t *count) {
    int rc = check(t, n, p, m, pos, cap, count);
    if (rc != 0) return rc;
    if (m > AUTOMATON_MAX_M) return -1;
    if (m > n) return 0;

    int *d = calloc((m + 1) * 256, sizeof(int));
    if (d == NULL) return -3;
    d[(unsigned char)p[0]] = 1;
    size_t x = 0;
    for (size_t j = 1; j <= m; j++) {
        memcpy(d + j * 256, d + x * 256, 256 * sizeof(int));
        if (j < m) {
            d[j * 256 + (unsigned char)p[j]] = (int)(j + 1);
            x = (size_t)d[x * 256 + (unsigned char)p[j]];
        }
    }
    size_t st = 0;
    for (size_t i = 0; i < n; i++) {
        st = (size_t)d[st * 256 + (unsigned char)t[i]];
        if (st == m) record(pos, cap, count, i + 1 - m);
    }
    free(d);
    return 0;
}

/* ---------- KMP: O(m) build, O(n) search ---------- */
int match_kmp(const char *t, size_t n, const char *p, size_t m,
              size_t *pos, size_t cap, size_t *count) {
    int rc = check(t, n, p, m, pos, cap, count);
    if (rc != 0) return rc;
    if (m > n) return 0;

    size_t *lps = malloc(m * sizeof(size_t));
    if (lps == NULL) return -3;
    lps[0] = 0;
    size_t k = 0;
    for (size_t i = 1; i < m; i++) {
        while (k > 0 && p[k] != p[i]) k = lps[k - 1];    /* fall back */
        if (p[k] == p[i]) k++;
        lps[i] = k;
    }
    size_t q = 0;
    for (size_t i = 0; i < n; i++) {
        while (q > 0 && p[q] != t[i]) q = lps[q - 1];
        if (p[q] == t[i]) q++;
        if (q == m) {
            record(pos, cap, count, i + 1 - m);
            q = lps[q - 1];                              /* keep going: overlaps count */
        }
    }
    free(lps);
    return 0;
}