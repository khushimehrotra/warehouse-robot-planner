#include <stdlib.h>
#include "test.h"
#include "backtrack.h"

static int brute_count(const int *a, int n, int target) {
    int cnt = 0;
    for (int mask = 0; mask < (1 << n); mask++) {
        long long s = 0;
        for (int i = 0; i < n; i++)
            if ((mask >> i) & 1) s += a[i];
        if (s == target) cnt++;
    }
    return cnt;
}

int main(void) {
    /* ---- N-Queens: known solution counts ---- */
    long long known[] = {0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724};
    long long cnt;
    int first[NQ_MAX_N];
    for (int n = 1; n <= 10; n++) {
        ASSERT_EQ(nqueens(n, &cnt, first), 0);
        ASSERT_EQ(cnt, known[n]);
        if (known[n] > 0) {                            /* the reported board must be legal */
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++) {
                    ASSERT_EQ(first[i] != first[j], 1);
                    ASSERT_EQ(abs(first[i] - first[j]) != j - i, 1);
                }
        }
    }
    ASSERT_EQ(nqueens(0, &cnt, NULL), -1);
    ASSERT_EQ(nqueens(NQ_MAX_N + 1, &cnt, NULL), -1);
    ASSERT_EQ(nqueens(6, &cnt, NULL), 0);              /* first may be NULL */
    ASSERT_EQ(cnt, 4);

    /* ---- subset sum: known case. {4,5} and {3,4,2} both make 9 ---- */
    int a1[] = {3, 34, 4, 12, 5, 2};
    char chosen[8];
    long long nodes;
    ASSERT_EQ(subset_sum(a1, 6, 9, chosen, &cnt, &nodes), 0);
    ASSERT_EQ(cnt, 2);
    long long sum = 0;
    for (int i = 0; i < 6; i++) if (chosen[i]) sum += a1[i];
    ASSERT_EQ(sum, 9);
    ASSERT_EQ(nodes < 127, 1);                         /* pruning beat the full tree */

    ASSERT_EQ(subset_sum(a1, 6, 1000, chosen, &cnt, NULL), 0);   /* impossible */
    ASSERT_EQ(cnt, 0);
    ASSERT_EQ(chosen[0] == 0 && chosen[5] == 0, 1);
    ASSERT_EQ(subset_sum(NULL, 0, 0, NULL, &cnt, NULL), 0);      /* empty set sums to 0 */
    ASSERT_EQ(cnt, 1);
    ASSERT_EQ(subset_sum(NULL, 0, 5, NULL, &cnt, NULL), 0);
    ASSERT_EQ(cnt, 0);
    int zeros[] = {0, 0};
    ASSERT_EQ(subset_sum(zeros, 2, 0, NULL, &cnt, NULL), 0);     /* all 4 subsets */
    ASSERT_EQ(cnt, 4);

    /* ---- invalid inputs ---- */
    int neg[] = {-1, 2};
    ASSERT_EQ(subset_sum(neg, 2, 1, NULL, &cnt, NULL), -1);
    ASSERT_EQ(subset_sum(a1, 6, -1, NULL, &cnt, NULL), -1);
    ASSERT_EQ(subset_sum(a1, -1, 3, NULL, &cnt, NULL), -1);
    ASSERT_EQ(subset_sum(NULL, 3, 3, NULL, &cnt, NULL), -1);

    /* ---- random: count must equal trying every subset ---- */
    srand(81);
    for (int t = 0; t < 500; t++) {
        int n = rand() % 13, a[12], target = rand() % 40;
        char ch[13];
        for (int i = 0; i < n; i++) a[i] = rand() % 10;
        ASSERT_EQ(subset_sum(a, n, target, ch, &cnt, &nodes), 0);
        ASSERT_EQ(cnt, brute_count(a, n, target));
        if (cnt > 0) {
            long long s = 0;
            for (int i = 0; i < n; i++) if (ch[i]) s += a[i];
            ASSERT_EQ(s, target);
        }
    }
    TEST_SUMMARY();
}