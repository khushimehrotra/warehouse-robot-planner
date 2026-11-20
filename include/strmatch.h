#ifndef STRMATCH_H
#define STRMATCH_H
#include <stddef.h>

#define AUTOMATON_MAX_M 4096

/* Common contract for all matchers: find every (overlapping) occurrence of
   p[0..m-1] in t[0..n-1]. The start of occurrence k is written to pos[k] while
   k < cap (pos may be NULL when cap == 0). *count always receives the TOTAL
   number of occurrences, even if it exceeds cap.
   Return: 0 ok, -1 bad arguments (NULL pointer, m == 0), -3 out of memory. */
int match_naive(const char *t, size_t n, const char *p, size_t m,
                size_t *pos, size_t cap, size_t *count);
int match_rabin_karp(const char *t, size_t n, const char *p, size_t m,
                     size_t *pos, size_t cap, size_t *count);
/* Same, with your own modulus q (1 <= q < 2^32). Small q forces hash collisions,
   which is how the tests prove the verification step works. */
int match_rabin_karp_mod(const char *t, size_t n, const char *p, size_t m,
                         size_t *pos, size_t cap, size_t *count,
                         unsigned long long q);
/* Also returns -1 if m > AUTOMATON_MAX_M (the table has (m+1)*256 entries). */
int match_automaton(const char *t, size_t n, const char *p, size_t m,
                    size_t *pos, size_t cap, size_t *count);
int match_kmp(const char *t, size_t n, const char *p, size_t m,
              size_t *pos, size_t cap, size_t *count);

#endif