#include <stdlib.h>
#include <limits.h>
#include "test.h"
#include "bnb.h"

static int brute(const int *c, int n, int row, int used) {
    if (row == n) return 0;
    int best = INT_MAX;
    for (int j = 0; j < n; j++)
        if (!((used >> j) & 1)) {
            int t = c[row * n + j] + brute(c, n, row + 1, used | (1 << j));
            if (t < best) best = t;
        }
    return best;
}

int main(void) {
    int as[BNB_MAX_N];
    long long tot;
    BnbStats st;

    /* ---- known 3x3 case: robot0->task1, robot1->task0, robot2->task2, total 9 ---- */
    int c3[] = {9, 2, 7,
                6, 4, 3,
                5, 8, 1};
    for (int mode = BNB_FIFO; mode <= BNB_LC; mode++) {
        ASSERT_EQ(assign_bnb(c3, 3, (BnbMode)mode, as, &tot, &st), 0);
        ASSERT_EQ(tot, 9);
        ASSERT_EQ(as[0], 1); ASSERT_EQ(as[1], 0); ASSERT_EQ(as[2], 2);
        ASSERT_EQ(st.expanded >= 1, 1);
    }
    int one[] = {7};
    ASSERT_EQ(assign_bnb(one, 1, BNB_LC, as, &tot, NULL), 0);
    ASSERT_EQ(tot, 7); ASSERT_EQ(as[0], 0);

    /* ---- invalid inputs ---- */
    ASSERT_EQ(assign_bnb(c3, 0, BNB_LC, as, &tot, NULL), -1);
    ASSERT_EQ(assign_bnb(c3, BNB_MAX_N + 1, BNB_LC, as, &tot, NULL), -1);
    ASSERT_EQ(assign_bnb(NULL, 3, BNB_LC, as, &tot, NULL), -1);
    ASSERT_EQ(assign_bnb(c3, 3, (BnbMode)7, as, &tot, NULL), -1);
    int neg[] = {1, -2, 3, 4};
    ASSERT_EQ(assign_bnb(neg, 2, BNB_LC, as, &tot, NULL), -1);

    /* ---- random: every strategy equals brute force; assignment is a real permutation ---- */
    srand(91);
    for (int t = 0; t < 300; t++) {
        int n = 1 + rand() % 7, c[49];
        int range = (rand() % 2) ? 4 : 30;             /* small range = many ties */
        for (int i = 0; i < n * n; i++) c[i] = rand() % range;
        int want = brute(c, n, 0, 0);
        for (int mode = BNB_FIFO; mode <= BNB_LC; mode++) {
            ASSERT_EQ(assign_bnb(c, n, (BnbMode)mode, as, &tot, &st), 0);
            ASSERT_EQ(tot, want);
            int used = 0;
            long long sum = 0;
            for (int i = 0; i < n; i++) {
                ASSERT_EQ(as[i] >= 0 && as[i] < n, 1);
                ASSERT_EQ((used >> as[i]) & 1, 0);
                used |= 1 << as[i];
                sum += c[i * n + as[i]];
            }
            ASSERT_EQ(sum, tot);
        }
    }
    TEST_SUMMARY();
}