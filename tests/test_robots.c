#include <stdlib.h>
#include "test.h"
#include "robot_io.h"

int main(void) {
    Robot *r;
    int n;

    ASSERT_EQ(robots_load("data/small_robots.txt", &r, &n), 0);
    ASSERT_EQ(n, 3);
    ASSERT_EQ(r[0].id, 1);
    ASSERT_EQ(r[1].x, 4);
    ASSERT_EQ(r[2].battery, 60);
    ASSERT_EQ(r[2].payload_cap, 8);
    free(r);

    /* invalid inputs must return errors, not crash */
    ASSERT_EQ(robots_load("data/missing.txt", &r, &n), -1);
    ASSERT_EQ(r == NULL, 1);
    TEST_SUMMARY();
}