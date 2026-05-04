/*
 * Stone Age Client - Game State Setter Functions Unit Tests
 * Tests for FUN_00479bc0, FUN_00479be0, FUN_00479c00, FUN_00479c20, FUN_004792c0
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

/* Game state variables */
static s32 DAT_04630dd8 = 0;
static s32 DAT_04630df0 = 0;
static s32 DAT_04630de8 = 0;
static s32 DAT_04630df8 = 0;
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
    DAT_04630dd8 = 0;
    DAT_04630df0 = 0;
    DAT_04630de8 = 0;
    DAT_04630df8 = 0;
    DAT_0462e3b4 = 0;
}

/*
 * FUN_00479bc0 - Set game state with reset
 * param_1: new state value
 * Resets df0 to 0 and sets dd8 to param_1
 */
static void FUN_00479bc0(u32 param_1) {
    DAT_04630df0 = 0;
    DAT_04630dd8 = (s32)param_1;
}

/*
 * FUN_00479be0 - Set game state with substate
 * param_1: state value
 * param_2: substate value
 */
static void FUN_00479be0(u32 param_1, u32 param_2) {
    DAT_04630dd8 = (s32)param_1;
    DAT_04630df0 = (s32)param_2;
}

/*
 * FUN_00479c00 - Set pending state with reset
 * param_1: pending state value
 * Sets de8 to 0 and df8 to param_1
 */
static void FUN_00479c00(u32 param_1) {
    DAT_04630de8 = 0;
    DAT_04630df8 = (s32)param_1;
}

/*
 * FUN_00479c20 - Set pending state values
 * param_1: df8 value
 * param_2: de8 value
 */
static void FUN_00479c20(u32 param_1, u32 param_2) {
    DAT_04630df8 = (s32)param_1;
    DAT_04630de8 = (s32)param_2;
}

/*
 * FUN_004792c0 - Calculate time-based value
 * Returns: DAT_0462e3b4 * 1800000 + 1000000
 */
static s32 FUN_004792c0(void) {
    return DAT_0462e3b4 * 1800000 + 1000000;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00479bc0 tests */
TEST(set_state_reset_basic) {
    reset_test_state();
    DAT_04630df0 = 5;

    FUN_00479bc0(3);

    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_reset_zero) {
    reset_test_state();

    FUN_00479bc0(0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_reset_large) {
    reset_test_state();

    FUN_00479bc0(100);

    ASSERT(DAT_04630dd8 == 100);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_reset_overwrites) {
    reset_test_state();
    DAT_04630dd8 = 50;
    DAT_04630df0 = 25;

    FUN_00479bc0(10);

    ASSERT(DAT_04630dd8 == 10);
    ASSERT(DAT_04630df0 == 0);
}

/* FUN_00479be0 tests */
TEST(set_state_with_substate_basic) {
    reset_test_state();

    FUN_00479be0(5, 10);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 10);
}

TEST(set_state_with_substate_zero) {
    reset_test_state();

    FUN_00479be0(0, 0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_with_substate_overwrites) {
    reset_test_state();
    DAT_04630dd8 = 99;
    DAT_04630df0 = 99;

    FUN_00479be0(1, 2);

    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 2);
}

TEST(set_state_with_substate_large) {
    reset_test_state();

    FUN_00479be0(200, 300);

    ASSERT(DAT_04630dd8 == 200);
    ASSERT(DAT_04630df0 == 300);
}

/* FUN_00479c00 tests */
TEST(set_pending_state_reset_basic) {
    reset_test_state();
    DAT_04630de8 = 7;

    FUN_00479c00(15);

    ASSERT(DAT_04630de8 == 0);
    ASSERT(DAT_04630df8 == 15);
}

TEST(set_pending_state_reset_zero) {
    reset_test_state();

    FUN_00479c00(0);

    ASSERT(DAT_04630de8 == 0);
    ASSERT(DAT_04630df8 == 0);
}

TEST(set_pending_state_reset_overwrites) {
    reset_test_state();
    DAT_04630de8 = 100;
    DAT_04630df8 = 200;

    FUN_00479c00(50);

    ASSERT(DAT_04630de8 == 0);
    ASSERT(DAT_04630df8 == 50);
}

/* FUN_00479c20 tests */
TEST(set_pending_state_values_basic) {
    reset_test_state();

    FUN_00479c20(10, 20);

    ASSERT(DAT_04630df8 == 10);
    ASSERT(DAT_04630de8 == 20);
}

TEST(set_pending_state_values_zero) {
    reset_test_state();

    FUN_00479c20(0, 0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(set_pending_state_values_overwrites) {
    reset_test_state();
    DAT_04630df8 = 999;
    DAT_04630de8 = 888;

    FUN_00479c20(1, 2);

    ASSERT(DAT_04630df8 == 1);
    ASSERT(DAT_04630de8 == 2);
}

/* FUN_004792c0 tests */
TEST(calculate_time_value_zero) {
    reset_test_state();
    DAT_0462e3b4 = 0;

    s32 result = FUN_004792c0();

    ASSERT(result == 1000000);
}

TEST(calculate_time_value_one) {
    reset_test_state();
    DAT_0462e3b4 = 1;

    s32 result = FUN_004792c0();

    ASSERT(result == 2800000);
}

TEST(calculate_time_value_two) {
    reset_test_state();
    DAT_0462e3b4 = 2;

    s32 result = FUN_004792c0();

    ASSERT(result == 4600000);
}

TEST(calculate_time_value_five) {
    reset_test_state();
    DAT_0462e3b4 = 5;

    s32 result = FUN_004792c0();

    ASSERT(result == 10000000);
}

TEST(calculate_time_value_negative) {
    reset_test_state();
    DAT_0462e3b4 = -1;

    s32 result = FUN_004792c0();

    /* -1 * 1800000 + 1000000 = -1800000 + 1000000 = -800000 */
    ASSERT(result == -800000);
}

TEST(calculate_time_value_large) {
    reset_test_state();
    DAT_0462e3b4 = 100;

    s32 result = FUN_004792c0();

    ASSERT(result == 181000000);
}

/* Combined state management tests */
TEST(state_transition_sequence) {
    reset_test_state();

    /* Initial state */
    FUN_00479bc0(0);
    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);

    /* Transition to state 1 */
    FUN_00479bc0(1);
    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 0);

    /* Set substate */
    FUN_00479be0(1, 5);
    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 5);

    /* Transition to state 2 */
    FUN_00479bc0(2);
    ASSERT(DAT_04630dd8 == 2);
    ASSERT(DAT_04630df0 == 0);
}

TEST(pending_state_sequence) {
    reset_test_state();

    /* Set pending state */
    FUN_00479c00(10);
    ASSERT(DAT_04630df8 == 10);
    ASSERT(DAT_04630de8 == 0);

    /* Update pending values */
    FUN_00479c20(20, 5);
    ASSERT(DAT_04630df8 == 20);
    ASSERT(DAT_04630de8 == 5);

    /* Reset pending state */
    FUN_00479c00(0);
    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(all_setters_independence) {
    reset_test_state();

    /* Set all values */
    FUN_00479be0(10, 20);
    FUN_00479c20(30, 40);

    /* Check independence */
    ASSERT(DAT_04630dd8 == 10);
    ASSERT(DAT_04630df0 == 20);
    ASSERT(DAT_04630df8 == 30);
    ASSERT(DAT_04630de8 == 40);

    /* FUN_00479bc0 should only affect dd8 and df0 */
    FUN_00479bc0(5);
    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 0);
    ASSERT(DAT_04630df8 == 30);  /* Unchanged */
    ASSERT(DAT_04630de8 == 40);  /* Unchanged */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Game State Setter Functions Unit Tests ===\n\n");

    printf("FUN_00479bc0 (Set State with Reset) Tests:\n");
    RUN_TEST(set_state_reset_basic);
    RUN_TEST(set_state_reset_zero);
    RUN_TEST(set_state_reset_large);
    RUN_TEST(set_state_reset_overwrites);

    printf("\nFUN_00479be0 (Set State with Substate) Tests:\n");
    RUN_TEST(set_state_with_substate_basic);
    RUN_TEST(set_state_with_substate_zero);
    RUN_TEST(set_state_with_substate_overwrites);
    RUN_TEST(set_state_with_substate_large);

    printf("\nFUN_00479c00 (Set Pending State with Reset) Tests:\n");
    RUN_TEST(set_pending_state_reset_basic);
    RUN_TEST(set_pending_state_reset_zero);
    RUN_TEST(set_pending_state_reset_overwrites);

    printf("\nFUN_00479c20 (Set Pending State Values) Tests:\n");
    RUN_TEST(set_pending_state_values_basic);
    RUN_TEST(set_pending_state_values_zero);
    RUN_TEST(set_pending_state_values_overwrites);

    printf("\nFUN_004792c0 (Calculate Time Value) Tests:\n");
    RUN_TEST(calculate_time_value_zero);
    RUN_TEST(calculate_time_value_one);
    RUN_TEST(calculate_time_value_two);
    RUN_TEST(calculate_time_value_five);
    RUN_TEST(calculate_time_value_negative);
    RUN_TEST(calculate_time_value_large);

    printf("\nCombined Tests:\n");
    RUN_TEST(state_transition_sequence);
    RUN_TEST(pending_state_sequence);
    RUN_TEST(all_setters_independence);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
