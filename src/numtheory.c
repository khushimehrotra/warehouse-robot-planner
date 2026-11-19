#include <stdlib.h>
#include <limits.h>
#include "numtheory.h"

long long gcd_ll(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) { long long t = a % b; a = b; b = t; }
    return a;
}

/* Extended Euclid: returns g = gcd(a,b) and x,y with a*x + b*y = g */
static long long egcd(long long a, long long b, long long *x, long long *y) {
    if (b == 0) { *x = 1; *y = 0; return a; }
    long long x1, y1;
    long long g = egcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

int mod_inverse(long long a, long long m, long long *inv) {
    if (m < 1) return -1;
    long long x, y;
    a = ((a % m) + m) % m;
    if (egcd(a, m, &x, &y) != 1) return -2;
    *inv = ((x % m) + m) % m;
    return 0;
}

int crt(const long long *r, const long long *m, int n, long long *x, long long *lcm) {
    if (n < 1 || r == NULL || m == NULL || x == NULL || lcm == NULL) return -1;
    for (int i = 0; i < n; i++)
        if (m[i] < 1 || m[i] > CRT_MAX_MOD) return -1;

    long long cx = ((r[0] % m[0]) + m[0]) % m[0], cl = m[0];
    for (int i = 1; i < n; i++) {
        long long ri = ((r[i] % m[i]) + m[i]) % m[i], mi = m[i];
        long long g = gcd_ll(cl, mi);
        long long diff = ri - cx;
        if (diff % g != 0) return -2;                  /* the two conditions clash */
        long long mg = mi / g;
        if (cl / g > LLONG_MAX / mi) return -3;        /* new lcm would overflow */
        long long t = 0, inv;
        if (mg > 1) {
            mod_inverse((cl / g) % mg, mg, &inv);      /* always exists here */
            t = (((diff / g) % mg + mg) % mg) * inv % mg;
        }
        long long nl = cl / g * mi;
        cx = (cx + cl * t) % nl;                       /* cl*t < nl, so no overflow */
        cl = nl;
    }
    *x = cx;
    *lcm = cl;
    return 0;
}