#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "sort.h"

static int cmp_int(const void *x, const void *y) {
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

int main(void) {
    srand(123);                                   /* fixed seed = repeatable */
    for (int trial = 0; trial < 500; trial++) {
        int n = rand() % 60;                      /* includes n = 0 and 1 */
        int *a = malloc((n + 1) * sizeof(int));
        int *b = malloc((n + 1) * sizeof(int));
        int *c = malloc((n + 1) * sizeof(int));
        for (int i = 0; i < n; i++) a[i] = rand() % 20 - 10;   /* many duplicates */
        memcpy(b, a, n * sizeof(int));
        memcpy(c, a, n * sizeof(int));

        qsort(a, n, sizeof(int), cmp_int);        /* the oracle */
        ASSERT_EQ(merge_sort(b, n), 0);
        quick_sort(c, n);
        ASSERT_EQ(memcmp(a, b, n * sizeof(int)), 0);
        ASSERT_EQ(memcmp(a, c, n * sizeof(int)), 0);

        /* binary search: every present value must be found */
        for (int i = 0; i < n; i++) {
            int k1 = bin_search_iter(a, n, a[i]);
            int k2 = bin_search_rec(a, 0, n - 1, a[i]);
            ASSERT_EQ(k1 >= 0 && a[k1] == a[i], 1);
            ASSERT_EQ(k2 >= 0 && a[k2] == a[i], 1);
        }
        ASSERT_EQ(bin_search_iter(a, n, 999), -1);   /* absent value */
        ASSERT_EQ(bin_search_rec(a, 0, n - 1, 999), -1);
        free(a); free(b); free(c);
    }
    TEST_SUMMARY();
}