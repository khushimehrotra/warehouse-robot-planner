#include <stdlib.h>
#include "test.h"
#include "numtheory.h"

int main(void) {
    long long x, L, inv;

    ASSERT_EQ(gcd_ll(12, 18), 6);
    ASSERT_EQ(gcd_ll(0, 5), 5);
    ASSERT_EQ(gcd_ll(-4, 6), 2);
    ASSERT_EQ(gcd_ll(0, 0), 0);

    ASSERT_EQ(mod_inverse(3, 7, &inv), 0);  ASSERT_EQ(inv, 5);
    ASSERT_EQ(mod_inverse(10, 17, &inv), 0); ASSERT_EQ(inv, 12);
    ASSERT_EQ(mod_inverse(-3, 7, &inv), 0);  ASSERT_EQ(inv, 2);   /* -3 = 4, and 4*2 = 8 = 1 */
    ASSERT_EQ(mod_inverse(5, 1, &inv), 0);   ASSERT_EQ(inv, 0);
    ASSERT_EQ(mod_inverse(2, 4, &inv), -2);                       /* not coprime */
    ASSERT_EQ(mod_inverse(2, 0, &inv), -1);

    /* ---- classic: x=2 (3), x=3 (5), x=2 (7) gives 23 mod 105 ---- */
    long long r1[] = {2, 3, 2}, m1[] = {3, 5, 7};
    ASSERT_EQ(crt(r1, m1, 3, &x, &L), 0);
    ASSERT_EQ(x, 23); ASSERT_EQ(L, 105);

    /* ---- warehouse cycles: charge every 4 (offset 1), service every 9 (3), calibrate every 5 (2) ---- */
    long long r2[] = {1, 3, 2}, m2[] = {4, 9, 5};
    ASSERT_EQ(crt(r2, m2, 3, &x, &L), 0);
    ASSERT_EQ(x, 57); ASSERT_EQ(L, 180);

    /* ---- moduli that share a factor ---- */
    long long r3[] = {1, 3}, m3[] = {4, 6};
    ASSERT_EQ(crt(r3, m3, 2, &x, &L), 0);
    ASSERT_EQ(x, 9); ASSERT_EQ(L, 12);
    long long r4[] = {1, 2};
    ASSERT_EQ(crt(r4, m3, 2, &x, &L), -2);             /* odd vs even clash */

    long long r5[] = {-1}, m5[] = {5};                 /* negative residue, single condition */
    ASSERT_EQ(crt(r5, m5, 1, &x, &L), 0);
    ASSERT_EQ(x, 4); ASSERT_EQ(L, 5);

    /* ---- invalid and overflow ---- */
    ASSERT_EQ(crt(r1, m1, 0, &x, &L), -1);
    long long m0[] = {3, 0};
    ASSERT_EQ(crt(r3, m0, 2, &x, &L), -1);
    long long mbig[] = {CRT_MAX_MOD + 1};
    ASSERT_EQ(crt(r5, mbig, 1, &x, &L), -1);
    long long pr[] = {5, 7}, pm[] = {2147483647LL, 2147483629LL};   /* two big primes */
    ASSERT_EQ(crt(pr, pm, 2, &x, &L), 0);
    ASSERT_EQ(x % pm[0], 5); ASSERT_EQ(x % pm[1], 7);
    ASSERT_EQ(x >= 0 && x < L, 1);
    long long pr3[] = {1, 2, 3}, pm3[] = {2147483647LL, 2147483629LL, 2147483587LL};
    ASSERT_EQ(crt(pr3, pm3, 3, &x, &L), -3);

    /* ---- random: the answer must equal the smallest t found by trying every value ---- */
    srand(111);
    for (int t = 0; t < 1000; t++) {
        int n = 1 + rand() % 3;
        long long r[3], m[3], lcm = 1;
        for (int i = 0; i < n; i++) {
            m[i] = 1 + rand() % 12;
            r[i] = rand() % 40 - 10;
            lcm = lcm / gcd_ll(lcm, m[i]) * m[i];
        }
        long long want = -1;
        for (long long v = 0; v < lcm && want < 0; v++) {
            int ok = 1;
            for (int i = 0; i < n; i++)
                if (((v - r[i]) % m[i]) != 0) ok = 0;
            if (ok) want = v;
        }
        int rc = crt(r, m, n, &x, &L);
        if (want < 0) {
            ASSERT_EQ(rc, -2);
        } else {
            ASSERT_EQ(rc, 0);
            ASSERT_EQ(x, want);
            ASSERT_EQ(L, lcm);
        }
    }
    TEST_SUMMARY();
}