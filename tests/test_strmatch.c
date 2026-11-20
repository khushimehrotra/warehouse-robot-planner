#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "strmatch.h"

typedef int (*Matcher)(const char *, size_t, const char *, size_t,
                       size_t *, size_t, size_t *);

static int rk_mod7(const char *t, size_t n, const char *p, size_t m,
                   size_t *pos, size_t cap, size_t *cnt) {
    return match_rabin_karp_mod(t, n, p, m, pos, cap, cnt, 7);
}
static int rk_mod1(const char *t, size_t n, const char *p, size_t m,
                   size_t *pos, size_t cap, size_t *cnt) {
    return match_rabin_karp_mod(t, n, p, m, pos, cap, cnt, 1);  /* every hash is 0 */
}

static Matcher fns[] = { match_naive, match_rabin_karp, rk_mod7, rk_mod1,
                         match_automaton, match_kmp };
#define NF 6

int main(void) {
    size_t pos[64], cnt;

    /* ---- known: overlapping matches ---- */
    for (int f = 0; f < NF; f++) {
        ASSERT_EQ(fns[f]("abababab", 8, "abab", 4, pos, 64, &cnt), 0);
        ASSERT_EQ(cnt, 3);
        ASSERT_EQ(pos[0], 0); ASSERT_EQ(pos[1], 2); ASSERT_EQ(pos[2], 4);

        ASSERT_EQ(fns[f]("abababab", 8, "abab", 4, pos, 1, &cnt), 0);   /* cap < matches */
        ASSERT_EQ(cnt, 3); ASSERT_EQ(pos[0], 0);
        ASSERT_EQ(fns[f]("abababab", 8, "abab", 4, NULL, 0, &cnt), 0);  /* count only */
        ASSERT_EQ(cnt, 3);

        ASSERT_EQ(fns[f]("aaaa", 4, "aa", 2, pos, 64, &cnt), 0);        /* a a a a */
        ASSERT_EQ(cnt, 3);
        ASSERT_EQ(fns[f]("abc", 3, "abcd", 4, pos, 64, &cnt), 0);       /* pattern longer */
        ASSERT_EQ(cnt, 0);
        ASSERT_EQ(fns[f]("", 0, "a", 1, pos, 64, &cnt), 0);             /* empty text */
        ASSERT_EQ(cnt, 0);
        ASSERT_EQ(fns[f]("abc", 3, "abc", 3, pos, 64, &cnt), 0);        /* whole text */
        ASSERT_EQ(cnt, 1); ASSERT_EQ(pos[0], 0);

        /* binary data with zero bytes: matching must use lengths, not strlen */
        const char bin[7] = {'a', 0, 'b', 0, 'b', 0, 'b'};
        const char bp[2] = {0, 'b'};
        ASSERT_EQ(fns[f](bin, 7, bp, 2, pos, 64, &cnt), 0);
        ASSERT_EQ(cnt, 3);
        ASSERT_EQ(pos[0], 1); ASSERT_EQ(pos[1], 3); ASSERT_EQ(pos[2], 5);

        /* invalid arguments */
        ASSERT_EQ(fns[f]("abc", 3, "", 0, pos, 64, &cnt), -1);          /* empty pattern */
        ASSERT_EQ(fns[f]("abc", 3, NULL, 1, pos, 64, &cnt), -1);
        ASSERT_EQ(fns[f](NULL, 3, "a", 1, pos, 64, &cnt), -1);
        ASSERT_EQ(fns[f]("abc", 3, "a", 1, NULL, 5, &cnt), -1);         /* cap but no buffer */
        ASSERT_EQ(fns[f]("abc", 3, "a", 1, pos, 64, NULL), -1);
    }

    /* ---- automaton size limit ---- */
    size_t big = AUTOMATON_MAX_M + 1;
    char *bt = malloc(big), *bpat = malloc(big);
    memset(bt, 'a', big); memset(bpat, 'a', big);
    ASSERT_EQ(match_automaton(bt, big, bpat, big, pos, 64, &cnt), -1);
    ASSERT_EQ(match_kmp(bt, big, bpat, big, pos, 64, &cnt), 0);
    ASSERT_EQ(cnt, 1);
    free(bt); free(bpat);

    /* ---- random: every matcher equals the naive one ---- */
    srand(121);
    for (int trial = 0; trial < 2000; trial++) {
        size_t n = (size_t)(rand() % 61), m = 1 + (size_t)(rand() % 8);
        int alpha = 2 + rand() % 2;
        char t[64], p[8];
        for (size_t i = 0; i < n; i++) t[i] = (char)('a' + rand() % alpha);
        for (size_t i = 0; i < m; i++) p[i] = (char)('a' + rand() % alpha);

        size_t want[64], wc;
        ASSERT_EQ(match_naive(t, n, p, m, want, 64, &wc), 0);
        for (int f = 1; f < NF; f++) {
            ASSERT_EQ(fns[f](t, n, p, m, pos, 64, &cnt), 0);
            ASSERT_EQ(cnt, wc);
            ASSERT_EQ(memcmp(pos, want, wc * sizeof(size_t)), 0);
        }
    }

    /* ---- one long text: the rolling hash must survive thousands of slides ---- */
    {
        size_t n = 5000;
        char *t = malloc(n);
        size_t *w = malloc(n * sizeof(size_t)), *g = malloc(n * sizeof(size_t));
        for (size_t i = 0; i < n; i++) t[i] = (char)('a' + rand() % 2);
        size_t wc, gc;
        ASSERT_EQ(match_naive(t, n, t + 1234, 20, w, n, &wc), 0);
        for (int f = 1; f < NF; f++) {
            ASSERT_EQ(fns[f](t, n, t + 1234, 20, g, n, &gc), 0);
            ASSERT_EQ(gc, wc);
            ASSERT_EQ(memcmp(g, w, wc * sizeof(size_t)), 0);
        }
        free(t); free(w); free(g);
    }
    TEST_SUMMARY();
}