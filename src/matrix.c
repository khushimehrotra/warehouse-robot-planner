#include <stdlib.h>
#include <string.h>
#include "matrix.h"

typedef long long ll;

/* T(n) = 8T(n/2) + Theta(n^2) = Theta(n^3).  Loop order i,k,j is cache friendly. */
int mat_mul_classic(const ll *A, const ll *B, ll *C, int n) {
    if (n <= 0) return -1;
    memset(C, 0, (size_t)n * n * sizeof(ll));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++) {
            ll aik = A[i * n + k];
            for (int j = 0; j < n; j++)
                C[i * n + j] += aik * B[k * n + j];
        }
    return 0;
}

static void madd(const ll *X, const ll *Y, ll *Z, size_t cnt) {
    for (size_t i = 0; i < cnt; i++) Z[i] = X[i] + Y[i];
}
static void msub(const ll *X, const ll *Y, ll *Z, size_t cnt) {
    for (size_t i = 0; i < cnt; i++) Z[i] = X[i] - Y[i];
}

/* T(n) = 7T(n/2) + Theta(n^2) = Theta(n^2.807).  n must be a power of two. */
static int strassen_rec(const ll *A, const ll *B, ll *C, int n, int cutoff) {
    if (n <= cutoff) return mat_mul_classic(A, B, C, n);

    int h = n / 2;
    size_t hs = (size_t)h * h;
    ll *buf = malloc(17 * hs * sizeof(ll));      /* 8 quadrants + 7 products + 2 temps */
    if (buf == NULL) return -1;
    ll *a11 = buf,      *a12 = a11 + hs, *a21 = a12 + hs, *a22 = a21 + hs;
    ll *b11 = a22 + hs, *b12 = b11 + hs, *b21 = b12 + hs, *b22 = b21 + hs;
    ll *m1 = b22 + hs, *m2 = m1 + hs, *m3 = m2 + hs, *m4 = m3 + hs;
    ll *m5 = m4 + hs, *m6 = m5 + hs, *m7 = m6 + hs;
    ll *t1 = m7 + hs, *t2 = t1 + hs;

    for (int i = 0; i < h; i++)                  /* copy the four quadrants out */
        for (int j = 0; j < h; j++) {
            size_t k = (size_t)i * h + j;
            a11[k] = A[i * n + j];       a12[k] = A[i * n + j + h];
            a21[k] = A[(i + h) * n + j]; a22[k] = A[(i + h) * n + j + h];
            b11[k] = B[i * n + j];       b12[k] = B[i * n + j + h];
            b21[k] = B[(i + h) * n + j]; b22[k] = B[(i + h) * n + j + h];
        }

    int rc = 0;
    madd(a11, a22, t1, hs); madd(b11, b22, t2, hs);
    if (rc == 0) rc = strassen_rec(t1, t2, m1, h, cutoff);   /* M1 */
    madd(a21, a22, t1, hs);
    if (rc == 0) rc = strassen_rec(t1, b11, m2, h, cutoff);  /* M2 */
    msub(b12, b22, t2, hs);
    if (rc == 0) rc = strassen_rec(a11, t2, m3, h, cutoff);  /* M3 */
    msub(b21, b11, t2, hs);
    if (rc == 0) rc = strassen_rec(a22, t2, m4, h, cutoff);  /* M4 */
    madd(a11, a12, t1, hs);
    if (rc == 0) rc = strassen_rec(t1, b22, m5, h, cutoff);  /* M5 */
    msub(a21, a11, t1, hs); madd(b11, b12, t2, hs);
    if (rc == 0) rc = strassen_rec(t1, t2, m6, h, cutoff);   /* M6 */
    msub(a12, a22, t1, hs); madd(b21, b22, t2, hs);
    if (rc == 0) rc = strassen_rec(t1, t2, m7, h, cutoff);   /* M7 */

    if (rc == 0) {
        for (int i = 0; i < h; i++)
            for (int j = 0; j < h; j++) {
                size_t k = (size_t)i * h + j;
                C[i * n + j]             = m1[k] + m4[k] - m5[k] + m7[k];
                C[i * n + j + h]         = m3[k] + m5[k];
                C[(i + h) * n + j]       = m2[k] + m4[k];
                C[(i + h) * n + j + h]   = m1[k] - m2[k] + m3[k] + m6[k];
            }
    }
    free(buf);
    return rc;
}

int mat_mul_strassen(const ll *A, const ll *B, ll *C, int n, int cutoff) {
    if (n <= 0) return -1;
    if (cutoff < 1) cutoff = 1;

    int m = 1;
    while (m < n) m *= 2;                        /* next power of two */
    if (m == n) return strassen_rec(A, B, C, n, cutoff);

    size_t sz = (size_t)m * m;
    ll *Ap = calloc(sz, sizeof(ll));             /* calloc = zero filled */
    ll *Bp = calloc(sz, sizeof(ll));
    ll *Cp = calloc(sz, sizeof(ll));
    if (!Ap || !Bp || !Cp) { free(Ap); free(Bp); free(Cp); return -1; }

    for (int i = 0; i < n; i++) {
        memcpy(Ap + (size_t)i * m, A + (size_t)i * n, n * sizeof(ll));
        memcpy(Bp + (size_t)i * m, B + (size_t)i * n, n * sizeof(ll));
    }
    int rc = strassen_rec(Ap, Bp, Cp, m, cutoff);
    if (rc == 0)
        for (int i = 0; i < n; i++)
            memcpy(C + (size_t)i * n, Cp + (size_t)i * m, n * sizeof(ll));
    free(Ap); free(Bp); free(Cp);
    return rc;
}
