#ifndef TEST_H
#define TEST_H
#include <stdio.h>

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_EQ(actual, expected) do {                                   \
    long _a = (long)(actual), _e = (long)(expected);                       \
    tests_run++;                                                           \
    if (_a != _e) {                                                        \
        tests_failed++;                                                    \
        printf("FAIL %s:%d  %s = %ld, expected %ld\n",                     \
               __FILE__, __LINE__, #actual, _a, _e);                       \
    }                                                                      \
} while (0)

#define TEST_SUMMARY() do {                                                \
    printf("%s: %d checks, %d failed\n", __FILE__, tests_run, tests_failed);\
    return tests_failed == 0 ? 0 : 1;                                      \
} while (0)

#endif