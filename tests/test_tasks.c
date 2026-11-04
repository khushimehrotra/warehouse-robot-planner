#include <stdlib.h>
#include "test.h"
#include "task_io.h"

int main(void) {
    Task *t;
    int n;

    ASSERT_EQ(tasks_load("data/small_tasks.txt", &t, &n), 0);
    ASSERT_EQ(n, 4);
    ASSERT_EQ(t[0].id, 1);
    ASSERT_EQ(t[2].weight, 8);
    ASSERT_EQ(t[3].deadline, 25);
    free(t);

    /* invalid inputs must return errors, not crash */
    ASSERT_EQ(tasks_load("data/missing.txt", &t, &n), -1);
    TEST_SUMMARY();
}