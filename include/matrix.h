#ifndef MATRIX_H
#define MATRIX_H

/* Square n x n matrices of long long stored row by row: element (i,j) is m[i*n + j].
   C must be a separate array from A and B (no aliasing).
   Return 0 ok, -1 invalid n or out of memory. */
int mat_mul_classic(const long long *A, const long long *B, long long *C, int n);

/* Strassen with padding to a power of two. Blocks of size <= cutoff use classical.
   cutoff < 1 is treated as 1. */
int mat_mul_strassen(const long long *A, const long long *B, long long *C,
                     int n, int cutoff);

#endif