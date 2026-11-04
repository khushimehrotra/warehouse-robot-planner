#include <stdlib.h>
#include "test.h"
#include "dnc.h"

static long long brute_max(const int *a, int n) {
    long long best = a[0];
    for (int i = 0; i < n; i++) {
        long long s = 0;
        for (int j = i; j < n; j++) { s += a[j]; if (s > best) best = s; }
    }
    return best;
}

static int brute_celeb(const char *k, int n) {
    for (int c = 0; c < n; c++) {
        int ok = 1;
        for (int i = 0; i < n && ok; i++)
            if (i != c && (k[c * n + i] || !k[i * n + c])) ok = 0;
        if (ok) return c;
    }
    return -1;
}

int main(void) {
    /* ---- max subarray: known cases ---- */
    SubArr r;
    int a1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    ASSERT_EQ(max_subarray(a1, 9, &r), 0);
    ASSERT_EQ(r.sum, 6); ASSERT_EQ(r.lo, 3); ASSERT_EQ(r.hi, 6);
    int a2[] = {-3, -1, -2};                 /* all negative: best is -1 */
    ASSERT_EQ(max_subarray(a2, 3, &r), 0);
    ASSERT_EQ(r.sum, -1);
    int a3[] = {7};
    ASSERT_EQ(max_subarray(a3, 1, &r), 0);
    ASSERT_EQ(r.sum, 7);
    ASSERT_EQ(max_subarray(a3, 0, &r), -1);  /* empty input */

    srand(7);
    for (int t = 0; t < 500; t++) {
        int n = 1 + rand() % 40, a[40];
        for (int i = 0; i < n; i++) a[i] = rand() % 41 - 20;
        ASSERT_EQ(max_subarray(a, n, &r), 0);
        ASSERT_EQ(r.sum, brute_max(a, n));
        long long s = 0;                     /* reported range must add up */
        for (int i = r.lo; i <= r.hi; i++) s += a[i];
        ASSERT_EQ(s, r.sum);
    }

    /* ---- power_mod ---- */
    unsigned long long p;
    ASSERT_EQ(power_mod(2, 10, 1000, &p), 0); ASSERT_EQ(p, 24);
    ASSERT_EQ(power_mod(3, 0, 7, &p), 0);     ASSERT_EQ(p, 1);
    ASSERT_EQ(power_mod(5, 3, 1, &p), 0);     ASSERT_EQ(p, 0);
    ASSERT_EQ(power_mod(2, 5, 0, &p), -1);    /* invalid modulus */
    for (int t = 0; t < 1000; t++) {
        unsigned long long base = rand() % 100, e = rand() % 60, m = 1 + rand() % 100000;
        unsigned long long naive = 1 % m;
        for (unsigned long long i = 0; i < e; i++) naive = (naive * base) % m;
        ASSERT_EQ(power_mod(base, e, m, &p), 0);
        ASSERT_EQ(p == naive, 1);
    }

    /* ---- celebrity ---- */
    ASSERT_EQ(celebrity(NULL, 0), -1);
    char one[1] = {0};
    ASSERT_EQ(celebrity(one, 1), 0);         /* a single person qualifies */
    for (int t = 0; t < 1000; t++) {
        int n = 1 + rand() % 12;
        char k[144];
        for (int i = 0; i < n * n; i++) k[i] = rand() % 2;
        if (rand() % 2) {                    /* plant a celebrity half the time */
            int c = rand() % n;
            for (int i = 0; i < n; i++) { k[c * n + i] = 0; k[i * n + c] = 1; }
        }
        ASSERT_EQ(celebrity(k, n), brute_celeb(k, n));
    }
    TEST_SUMMARY();
}