/* tests/test_vec.c */
#include "test.h"
#include "vec.h"

int main(void) {
    Vec v;
    vec_init(&v);
    ASSERT_EQ(v.size, 0);

    for (int i = 0; i < 100; i++) ASSERT_EQ(vec_push(&v, i * 2), 0);
    ASSERT_EQ(v.size, 100);
    ASSERT_EQ(vec_get(&v, 0), 0);
    ASSERT_EQ(vec_get(&v, 99), 198);

    vec_free(&v);
    ASSERT_EQ(v.size, 0);
    TEST_SUMMARY();
}