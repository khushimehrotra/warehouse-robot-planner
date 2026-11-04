#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sort.h"
#include "matrix.h"


static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void time_sorts(const char *name, const int *src, int n) {
    int *a = malloc(n * sizeof(int)), *b = malloc(n * sizeof(int));
    memcpy(a, src, n * sizeof(int));
    memcpy(b, src, n * sizeof(int));
    double t0 = now_ms(); merge_sort(a, n); double t1 = now_ms();
    quick_sort(b, n);                        double t2 = now_ms();
    printf("%-14s n=%-7d merge %8.2f ms   quick %8.2f ms\n", name, n, t1 - t0, t2 - t1);
    free(a); free(b);
}

int main(void) {
    srand(1);
    int n = 200000;
    int *d = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) d[i] = rand();
    time_sorts("random", d, n);
    for (int i = 0; i < n; i++) d[i] = i;
    time_sorts("already sorted", d, n);
    int m = 30000;                           /* all equal: quick sort goes O(n^2) */
    for (int i = 0; i < m; i++) d[i] = 5;
    time_sorts("all equal", d, m);
    free(d);

    printf("\nMatrix multiplication (Strassen cutoff 64)\n");
    for (int sz = 64; sz <= 512; sz *= 2) {
        long long *X = malloc((size_t)sz * sz * 8), *Y = malloc((size_t)sz * sz * 8);
        long long *R1 = malloc((size_t)sz * sz * 8), *R2 = malloc((size_t)sz * sz * 8);
        for (int i = 0; i < sz * sz; i++) { X[i] = rand() % 10; Y[i] = rand() % 10; }
        double t0 = now_ms(); mat_mul_classic(X, Y, R1, sz);       double t1 = now_ms();
        mat_mul_strassen(X, Y, R2, sz, 64);                         double t2 = now_ms();
        printf("n=%-4d classic %9.2f ms   strassen %9.2f ms   same=%s\n", sz,
               t1 - t0, t2 - t1, memcmp(R1, R2, (size_t)sz * sz * 8) == 0 ? "yes" : "NO");
        free(X); free(Y); free(R1); free(R2);
    }
    return 0;
}