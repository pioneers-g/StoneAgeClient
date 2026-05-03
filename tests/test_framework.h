/*
 * Stone Age Client - Unit Test Framework
 * Simple test framework for C projects
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
/* Disable Windows error popups (like assertion failures) */
#define DISABLE_ERROR_POPUPS() do { \
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX); \
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT); \
} while(0)
#else
#define DISABLE_ERROR_POPUPS() do {} while(0)
#endif

/* Test counters */
static int s_tests_run = 0;
static int s_tests_passed = 0;
static int s_tests_failed = 0;

/* Current test name */
static char s_current_test[256] = "";

/*
 * Assert macros
 * Support both 1 and 2 parameter forms for TEST_ASSERT
 * Support both 2 and 3 parameter forms for TEST_ASSERT_EQ and TEST_ASSERT_STR_EQ
 */

/* TEST_ASSERT - can be called with just condition or condition and message */
#define TEST_ASSERT_1(cond) do { \
    if (!(cond)) { \
        printf("  FAILED: %s\n    Line %d: assertion failed\n", s_current_test, __LINE__); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_2(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAILED: %s\n    Line %d: %s\n", s_current_test, __LINE__, msg); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_GET_MACRO(_1, _2, NAME, ...) NAME
#define TEST_ASSERT(...) TEST_ASSERT_GET_MACRO(__VA_ARGS__, TEST_ASSERT_2, TEST_ASSERT_1)(__VA_ARGS__)

/* TEST_ASSERT_EQ - can be called with (expected, actual) or (expected, actual, message) */
#define TEST_ASSERT_EQ_2(exp, act) do { \
    if ((exp) != (act)) { \
        printf("  FAILED: %s\n    Line %d: (expected %d, got %d)\n", \
               s_current_test, __LINE__, (int)(exp), (int)(act)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQ_3(exp, act, msg) do { \
    if ((exp) != (act)) { \
        printf("  FAILED: %s\n    Line %d: %s (expected %d, got %d)\n", \
               s_current_test, __LINE__, msg, (int)(exp), (int)(act)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQ_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define TEST_ASSERT_EQ(...) TEST_ASSERT_EQ_GET_MACRO(__VA_ARGS__, TEST_ASSERT_EQ_3, TEST_ASSERT_EQ_2)(__VA_ARGS__)

/* TEST_ASSERT_STR_EQ - can be called with (expected, actual) or (expected, actual, message) */
#define TEST_ASSERT_STR_EQ_2(exp, act) do { \
    if (strcmp((exp), (act)) != 0) { \
        printf("  FAILED: %s\n    Line %d: (expected \"%s\", got \"%s\")\n", \
               s_current_test, __LINE__, (exp), (act)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ_3(exp, act, msg) do { \
    if (strcmp((exp), (act)) != 0) { \
        printf("  FAILED: %s\n    Line %d: %s (expected \"%s\", got \"%s\")\n", \
               s_current_test, __LINE__, msg, (exp), (act)); \
        s_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define TEST_ASSERT_STR_EQ(...) TEST_ASSERT_STR_EQ_GET_MACRO(__VA_ARGS__, TEST_ASSERT_STR_EQ_3, TEST_ASSERT_STR_EQ_2)(__VA_ARGS__)

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
    fflush(stdout); \
} while(0)

/* Test initialization - call at the start of main() */
#define TEST_INIT() do { \
    DISABLE_ERROR_POPUPS(); \
    printf("=== Unit Tests ===\n\n"); \
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
