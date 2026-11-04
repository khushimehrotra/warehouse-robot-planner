#include <limits.h>
#include "dnc.h"

/* ---------- Maximum subarray ----------
   T(n) = 2T(n/2) + Theta(n)  ->  Theta(n log n)  */
static SubArr max_cross(const int *a, int lo, int mid, int hi) {
    long long s = 0, best_l = LLONG_MIN, best_r = LLONG_MIN;
    int li = mid, ri = mid + 1;
    for (int i = mid; i >= lo; i--) {          /* grow leftwards from the middle */
        s += a[i];
        if (s > best_l) { best_l = s; li = i; }
    }
    s = 0;
    for (int j = mid + 1; j <= hi; j++) {      /* grow rightwards */
        s += a[j];
        if (s > best_r) { best_r = s; ri = j; }
    }
    SubArr r = { best_l + best_r, li, ri };
    return r;
}

static SubArr max_rec(const int *a, int lo, int hi) {
    if (lo == hi) {
        SubArr one = { a[lo], lo, lo };
        return one;
    }
    int mid = lo + (hi - lo) / 2;
    SubArr L = max_rec(a, lo, mid);
    SubArr R = max_rec(a, mid + 1, hi);
    SubArr C = max_cross(a, lo, mid, hi);
    if (L.sum >= R.sum && L.sum >= C.sum) return L;
    if (R.sum >= C.sum) return R;
    return C;
}

int max_subarray(const int *a, int n, SubArr *out) {
    if (n <= 0) return -1;
    *out = max_rec(a, 0, n - 1);
    return 0;
}

/* ---------- Fast powering ----------
   T(n) = T(n/2) + O(1)  ->  O(log n)
   m < 2^32 so that (x*y) fits in 64 bits before the % m. */
int power_mod(unsigned long long base, unsigned long long exp,
              unsigned long long m, unsigned long long *out) {
    if (m == 0 || m > 4294967295ULL) return -1;
    unsigned long long result = 1 % m;         /* 1 % 1 = 0, correct for m = 1 */
    base %= m;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % m;   /* low bit set: use base */
        base = (base * base) % m;                    /* square */
        exp >>= 1;                                   /* halve the exponent */
    }
    *out = result;
    return 0;
}

/* ---------- Celebrity ----------
   Elimination: n-1 questions, then n-1 checks each way -> O(n). */
#define KNOWS(i, j) (knows[(i) * n + (j)])

int celebrity(const char *knows, int n) {
    if (n <= 0) return -1;
    int cand = 0;
    for (int i = 1; i < n; i++)
        if (KNOWS(cand, i)) cand = i;          /* cand knows someone: not celebrity */
    for (int i = 0; i < n; i++) {
        if (i == cand) continue;
        if (KNOWS(cand, i) || !KNOWS(i, cand)) return -1;
    }
    return cand;
}