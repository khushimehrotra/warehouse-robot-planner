#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "matrix.h"

int main(void) {
    /* known answer */
    long long A[4] = {1, 2, 3, 4}, B[4] = {5, 6, 7, 8}, C[4];
    long long want[4] = {19, 22, 43, 50};
    ASSERT_EQ(mat_mul_classic(A, B, C, 2), 0);
    ASSERT_EQ(memcmp(C, want, sizeof want), 0);
    ASSERT_EQ(mat_mul_strassen(A, B, C, 2, 1), 0);
    ASSERT_EQ(memcmp(C, want, sizeof want), 0);

    ASSERT_EQ(mat_mul_classic(A, B, C, 0), -1);      /* invalid size */
    ASSERT_EQ(mat_mul_strassen(A, B, C, -3, 1), -1);

    /* random: Strassen must equal classical, including sizes that are not 2^k */
    srand(99);
    for (int t = 0; t < 200; t++) {
        int n = 1 + rand() % 20;
        long long *X = malloc(n * n * sizeof(long long));
        long long *Y = malloc(n * n * sizeof(long long));
        long long *R1 = malloc(n * n * sizeof(long long));
        long long *R2 = malloc(n * n * sizeof(long long));
        for (int i = 0; i < n * n; i++) { X[i] = rand() % 11 - 5; Y[i] = rand() % 11 - 5; }
        ASSERT_EQ(mat_mul_classic(X, Y, R1, n), 0);
        for (int cutoff = 1; cutoff <= 4; cutoff *= 2) {
            ASSERT_EQ(mat_mul_strassen(X, Y, R2, n, cutoff), 0);
            ASSERT_EQ(memcmp(R1, R2, n * n * sizeof(long long)), 0);
        }
        free(X); free(Y); free(R1); free(R2);
    }
    TEST_SUMMARY();
}