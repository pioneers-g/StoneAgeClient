/*
 * Stone Age Client - Time Calculation Function Unit Tests
 * Tests for FUN_004792c0 - Calculate time value
 * This function calculates: DAT_0462e3b4 * 1800000 + 1000000
 * Used for time-based calculations (likely timeout or interval)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated state variable */
static s32 DAT_0462e3b4 = 0;

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    test_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        test_failed++; \
        return; \
    } \
} while(0)

/* Reset test state */
static void reset_test_state(void) {
    DAT_0462e3b4 = 0;
}

/*
 * FUN_004792c0 - Calculate time value
 * Returns DAT_0462e3b4 * 1800000 + 1000000
 * 1800000 ms = 30 minutes
 * Base value = 1000000 ms
 */
static s32 FUN_004792c0(void) {
    return DAT_0462e3b4 * 1800000 + 1000000;
}

/* ========================================
 * Test Cases for FUN_004792c0
 * ======================================== */

TEST(calc_zero_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 0;

    s32 result = FUN_004792c0();

    ASSERT(result == 1000000);
}

TEST(calc_one_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 1;

    s32 result = FUN_004792c0();

    ASSERT(result == 2800000);
}

TEST(calc_two_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 2;

    s32 result = FUN_004792c0();

    ASSERT(result == 4600000);
}

TEST(calc_five_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 5;

    s32 result = FUN_004792c0();

    ASSERT(result == 10000000);
}

TEST(calc_ten_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 10;

    s32 result = FUN_004792c0();

    ASSERT(result == 19000000);
}

TEST(calc_negative_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = -1;

    s32 result = FUN_004792c0();

    /* -1800000 + 1000000 = -800000 */
    ASSERT(result == -800000);
}

TEST(calc_large_multiplier) {
    reset_test_state();
    DAT_0462e3b4 = 100;

    s32 result = FUN_004792c0();

    ASSERT(result == 181000000);
}

TEST(calc_small_positive) {
    reset_test_state();
    DAT_0462e3b4 = 3;

    s32 result = FUN_004792c0();

    ASSERT(result == 6400000);
}

TEST(calc_does_not_modify_global) {
    reset_test_state();
    DAT_0462e3b4 = 7;

    FUN_004792c0();

    ASSERT(DAT_0462e3b4 == 7);
}

TEST(calc_consistent_results) {
    reset_test_state();
    DAT_0462e3b4 = 4;

    s32 result1 = FUN_004792c0();
    s32 result2 = FUN_004792c0();

    ASSERT(result1 == result2);
    ASSERT(result1 == 8200000);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Time Calculation Function Unit Tests ===\n\n");

    printf("FUN_004792c0 (Time Calculation) Tests:\n");
    RUN_TEST(calc_zero_multiplier);
    RUN_TEST(calc_one_multiplier);
    RUN_TEST(calc_two_multiplier);
    RUN_TEST(calc_five_multiplier);
    RUN_TEST(calc_ten_multiplier);
    RUN_TEST(calc_negative_multiplier);
    RUN_TEST(calc_large_multiplier);
    RUN_TEST(calc_small_positive);
    RUN_TEST(calc_does_not_modify_global);
    RUN_TEST(calc_consistent_results);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
