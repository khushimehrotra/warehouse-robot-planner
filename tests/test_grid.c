/* tests/test_grid.c  (run from the project root folder) */
#include "test.h"
#include "grid.h"

int main(void) {
    Grid g;
    ASSERT_EQ(grid_load("data/small_grid.txt", &g), 0);
    ASSERT_EQ(g.rows, 5);
    ASSERT_EQ(g.cols, 8);
    ASSERT_EQ(grid_get(&g, 0, 0), 'C');
    ASSERT_EQ(grid_get(&g, 2, 5), 'S');
    ASSERT_EQ(grid_is_walkable(&g, 0, 3), 0);    /* '#' obstacle */
    ASSERT_EQ(grid_is_walkable(&g, -1, 0), 0);   /* out of bounds */
    ASSERT_EQ(grid_is_walkable(&g, 0, 1), 1);
    grid_free(&g);

    /* invalid inputs must return errors, not crash */
    ASSERT_EQ(grid_load("data/does_not_exist.txt", &g), -1);
    TEST_SUMMARY();
}