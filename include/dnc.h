#ifndef DNC_H
#define DNC_H

typedef struct { long long sum; int lo, hi; } SubArr;

/* Best consecutive stretch a[lo..hi]. Returns 0 ok, -1 if n <= 0. */
int max_subarray(const int *a, int n, SubArr *out);

/* (base^exp) mod m by repeated squaring. m must be 1..4294967295.
   Returns 0 ok, -1 if m is out of range. */
int power_mod(unsigned long long base, unsigned long long exp,
              unsigned long long m, unsigned long long *out);

/* knows[i*n + j] == 1 means person i knows person j (diagonal ignored).
   Returns the celebrity's index, or -1 if there is none (or n <= 0). */
int celebrity(const char *knows, int n);

#endif