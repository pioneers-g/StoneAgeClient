/*
 * Stone Age Client - Unit Test Framework
 * Simple test framework for C projects
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Test counters */
static int s_tests_run = 0;
static int s_tests_passed = 0;
static int s_tests_failed = 0;

/* Current test name */
static char s_current_test[256] = "";

/* Assert macros */
#define TEST_ASSERT(condition, message) do { \
    if (!(condition)) { \
        printf("  FAILED: %s\n    Line %d: %s\n", s_current_test, __LINE__, message); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

/* 1-parameter version for convenience */
#undef TEST_ASSERT
#define TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf("  FAILED: %s\n    Line %d: assertion failed\n", s_current_test, __LINE__); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQ(expected, actual, message) do { \
    if ((expected) != (actual)) { \
        printf("  FAILED: %s\n    Line %d: %s (expected %d, got %d)\n", \
               s_current_test, __LINE__, message, (int)(expected), (int)(actual)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

/* 2-parameter version for convenience */
#define TEST_ASSERT_EQ2(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("  FAILED: %s\n    Line %d: (expected %d, got %d)\n", \
               s_current_test, __LINE__, (int)(expected), (int)(actual)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

/* Override to use 2-param version by default */
#undef TEST_ASSERT_EQ
#define TEST_ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("  FAILED: %s\n    Line %d: (expected %d, got %d)\n", \
               s_current_test, __LINE__, (int)(expected), (int)(actual)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(expected, actual, message) do { \
    if (strcmp((expected), (actual)) != 0) { \
        printf("  FAILED: %s\n    Line %d: %s (expected \"%s\", got \"%s\")\n", \
               s_current_test, __LINE__, message, (expected), (actual)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

/* 2-parameter version for convenience */
#undef TEST_ASSERT_STR_EQ
#define TEST_ASSERT_STR_EQ(expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        printf("  FAILED: %s\n    Line %d: (expected \"%s\", got \"%s\")\n", \
               s_current_test, __LINE__, (expected), (actual)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_MEM_EQ(expected, actual, size, message) do { \
    if (memcmp((expected), (actual), (size)) != 0) { \
        printf("  FAILED: %s\n    Line %d: %s (memory mismatch)\n", \
               s_current_test, __LINE__, message); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

/* Test declaration */
#define TEST_BEGIN(name) do { \
    strncpy(s_current_test, name, sizeof(s_current_test) - 1); \
    s_tests_run++; \
    printf("  Running: %s... ", name); \
} while(0)

#define TEST_END() do { \
    s_tests_passed++; \
    printf("PASSED\n"); \
} while(0)

/* Test suite */
#define TEST_SUITE(name) void test_suite_##name(void)

#define RUN_SUITE(name) do { \
    printf("\n[%s]\n", #name); \
    test_suite_##name(); \
} while(0)

#define RUN_TEST_SUITE(name) do { \
    printf("\n[%s]\n", #name); \
    test_suite_##name(); \
} while(0)

/* Summary */
static inline void test_summary(void) {
    printf("\n========================================\n");
    printf("Tests Run:    %d\n", s_tests_run);
    printf("Tests Passed: %d\n", s_tests_passed);
    printf("Tests Failed: %d\n", s_tests_failed);
    printf("Coverage:     %.1f%%\n",
           s_tests_run > 0 ? (100.0 * s_tests_passed / s_tests_run) : 0.0);
    printf("========================================\n");
}

static inline int test_all_passed(void) {
    return s_tests_failed == 0;
}

/* File path helper - use original game data directory */
#define TEST_DATA_DIR "D:/Games/石器时代8.0/石器时代8.0单机版/data/"

#endif /* TEST_FRAMEWORK_H */
