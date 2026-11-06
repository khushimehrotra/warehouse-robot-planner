#include <stdlib.h>
#include "test.h"
#include "greedy.h"

static int overlap(const Interval *a, const Interval *b) {
    return a->start < b->finish && b->start < a->finish;
}

static int brute_activity(const Interval *iv, int n) {
    int best = 0;
    for (int mask = 0; mask < (1 << n); mask++) {
        int ok = 1, cnt = 0;
        for (int i = 0; i < n && ok; i++) {
            if (((mask >> i) & 1) == 0) continue;
            cnt++;
            for (int j = i + 1; j < n; j++)
                if (((mask >> j) & 1) && overlap(&iv[i], &iv[j])) { ok = 0; break; }
        }
        if (ok && cnt > best) best = cnt;
    }
    return best;
}

static int max_depth(const Interval *iv, int n) {
    int best = 0;
    for (int i = 0; i < n; i++) {
        int d = 0;
        for (int j = 0; j < n; j++)
            if (iv[j].start <= iv[i].start && iv[i].start < iv[j].finish) d++;
        if (d > best) best = d;
    }
    return best;
}

int main(void) {
    int chosen[16], cnt;

    /* classic textbook example: {id, start, finish}; best answer has 4 jobs */
    Interval k[] = {{1,1,4},{2,3,5},{3,0,6},{4,5,7},{5,3,9},{6,5,9},
                    {7,6,10},{8,8,11},{9,8,12},{10,2,14},{11,12,16}};
    ASSERT_EQ(activity_select(k, 11, chosen, &cnt), 0);
    ASSERT_EQ(cnt, 4);
    ASSERT_EQ(chosen[0], 1); ASSERT_EQ(chosen[1], 4);
    ASSERT_EQ(chosen[2], 8); ASSERT_EQ(chosen[3], 11);

    /* edge cases */
    ASSERT_EQ(activity_select(NULL, 0, chosen, &cnt), 0);
    ASSERT_EQ(cnt, 0);
    Interval bad[] = {{0, 5, 5}};                      /* zero length is invalid */
    ASSERT_EQ(activity_select(bad, 1, chosen, &cnt), -2);
    int room[16], rooms;
    ASSERT_EQ(interval_partition(bad, 1, room, &rooms), -2);

    /* random tests against the oracles */
    srand(5);
    for (int t = 0; t < 500; t++) {
        int n = rand() % 11;                           /* 0..10 */
        Interval iv[12];
        for (int i = 0; i < n; i++) {
            iv[i].id = i;
            iv[i].start = rand() % 20;
            iv[i].finish = iv[i].start + 1 + rand() % 8;
        }
        ASSERT_EQ(activity_select(iv, n, chosen, &cnt), 0);
        ASSERT_EQ(cnt, brute_activity(iv, n));
        for (int a = 0; a < cnt; a++)                  /* chosen ones must not overlap */
            for (int b = a + 1; b < cnt; b++)
                ASSERT_EQ(overlap(&iv[chosen[a]], &iv[chosen[b]]), 0);

        ASSERT_EQ(interval_partition(iv, n, room, &rooms), 0);
        ASSERT_EQ(rooms, max_depth(iv, n));
        for (int i = 0; i < n; i++) {
            ASSERT_EQ(room[i] >= 0 && room[i] < rooms, 1);
            for (int j = i + 1; j < n; j++)            /* overlapping jobs never share a room */
                if (overlap(&iv[i], &iv[j])) ASSERT_EQ(room[i] != room[j], 1);
        }
    }
    TEST_SUMMARY();
}