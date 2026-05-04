/*
 * Stone Age Client - State Setter Functions Unit Tests
 * Tests for FUN_00479bc0, FUN_00479be0, FUN_00479c00, FUN_00479c20
 * These functions set state variables for game state management
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

/* Simulated state variables */
static s32 DAT_04630df0 = 0;
static s32 DAT_04630dd8 = 0;
static s32 DAT_04630de8 = 0;
static s32 DAT_04630df8 = 0;

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
    DAT_04630df0 = 0;
    DAT_04630dd8 = 0;
    DAT_04630de8 = 0;
    DAT_04630df8 = 0;
}

/*
 * FUN_00479bc0 - Set state with sub-state reset
 * Sets DAT_04630df0 = 0, DAT_04630dd8 = param_1
 */
static void FUN_00479bc0(s32 param_1) {
    DAT_04630df0 = 0;
    DAT_04630dd8 = param_1;
}

/*
 * FUN_00479be0 - Set both state values
 * Sets DAT_04630dd8 = param_1, DAT_04630df0 = param_2
 */
static void FUN_00479be0(s32 param_1, s32 param_2) {
    DAT_04630dd8 = param_1;
    DAT_04630df0 = param_2;
}

/*
 * FUN_00479c00 - Set pending state with sub-state reset
 * Sets DAT_04630de8 = 0, DAT_04630df8 = param_1
 */
static void FUN_00479c00(s32 param_1) {
    DAT_04630de8 = 0;
    DAT_04630df8 = param_1;
}

/*
 * FUN_00479c20 - Set both pending state values
 * Sets DAT_04630df8 = param_1, DAT_04630de8 = param_2
 */
static void FUN_00479c20(s32 param_1, s32 param_2) {
    DAT_04630df8 = param_1;
    DAT_04630de8 = param_2;
}

/* ========================================
 * Test Cases for FUN_00479bc0
 * ======================================== */

TEST(bc0_set_state_zero) {
    reset_test_state();
    FUN_00479bc0(0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(bc0_set_state_one) {
    reset_test_state();
    FUN_00479bc0(1);

    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 0);
}

TEST(bc0_set_state_five) {
    reset_test_state();
    FUN_00479bc0(5);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 0);
}

TEST(bc0_reset_substate) {
    reset_test_state();
    DAT_04630df0 = 100;  /* Set substate to non-zero */
    FUN_00479bc0(3);

    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 0);  /* Should be reset */
}

TEST(bc0_negative_value) {
    reset_test_state();
    FUN_00479bc0(-1);

    ASSERT(DAT_04630dd8 == -1);
    ASSERT(DAT_04630df0 == 0);
}

/* ========================================
 * Test Cases for FUN_00479be0
 * ======================================== */

TEST(be0_set_both_zero) {
    reset_test_state();
    FUN_00479be0(0, 0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(be0_set_both_values) {
    reset_test_state();
    FUN_00479be0(5, 10);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 10);
}

TEST(be0_overwrite_existing) {
    reset_test_state();
    DAT_04630dd8 = 100;
    DAT_04630df0 = 200;
    FUN_00479be0(3, 7);

    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 7);
}

TEST(be0_negative_values) {
    reset_test_state();
    FUN_00479be0(-5, -10);

    ASSERT(DAT_04630dd8 == -5);
    ASSERT(DAT_04630df0 == -10);
}

TEST(be0_large_values) {
    reset_test_state();
    FUN_00479be0(1000, 2000);

    ASSERT(DAT_04630dd8 == 1000);
    ASSERT(DAT_04630df0 == 2000);
}

/* ========================================
 * Test Cases for FUN_00479c00
 * ======================================== */

TEST(c00_set_pending_zero) {
    reset_test_state();
    FUN_00479c00(0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(c00_set_pending_value) {
    reset_test_state();
    FUN_00479c00(5);

    ASSERT(DAT_04630df8 == 5);
    ASSERT(DAT_04630de8 == 0);
}

TEST(c00_reset_pending_substate) {
    reset_test_state();
    DAT_04630de8 = 100;
    FUN_00479c00(3);

    ASSERT(DAT_04630df8 == 3);
    ASSERT(DAT_04630de8 == 0);
}

TEST(c00_negative_value) {
    reset_test_state();
    FUN_00479c00(-1);

    ASSERT(DAT_04630df8 == -1);
    ASSERT(DAT_04630de8 == 0);
}

/* ========================================
 * Test Cases for FUN_00479c20
 * ======================================== */

TEST(c20_set_both_pending_zero) {
    reset_test_state();
    FUN_00479c20(0, 0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(c20_set_both_pending_values) {
    reset_test_state();
    FUN_00479c20(7, 14);

    ASSERT(DAT_04630df8 == 7);
    ASSERT(DAT_04630de8 == 14);
}

TEST(c20_overwrite_existing) {
    reset_test_state();
    DAT_04630df8 = 100;
    DAT_04630de8 = 200;
    FUN_00479c20(1, 2);

    ASSERT(DAT_04630df8 == 1);
    ASSERT(DAT_04630de8 == 2);
}

TEST(c20_negative_values) {
    reset_test_state();
    FUN_00479c20(-3, -6);

    ASSERT(DAT_04630df8 == -3);
    ASSERT(DAT_04630de8 == -6);
}

TEST(c20_large_values) {
    reset_test_state();
    FUN_00479c20(5000, 10000);

    ASSERT(DAT_04630df8 == 5000);
    ASSERT(DAT_04630de8 == 10000);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== State Setter Functions Unit Tests ===\n\n");

    printf("FUN_00479bc0 (Set state with reset) Tests:\n");
    RUN_TEST(bc0_set_state_zero);
    RUN_TEST(bc0_set_state_one);
    RUN_TEST(bc0_set_state_five);
    RUN_TEST(bc0_reset_substate);
    RUN_TEST(bc0_negative_value);

    printf("\nFUN_00479be0 (Set both state values) Tests:\n");
    RUN_TEST(be0_set_both_zero);
    RUN_TEST(be0_set_both_values);
    RUN_TEST(be0_overwrite_existing);
    RUN_TEST(be0_negative_values);
    RUN_TEST(be0_large_values);

    printf("\nFUN_00479c00 (Set pending state with reset) Tests:\n");
    RUN_TEST(c00_set_pending_zero);
    RUN_TEST(c00_set_pending_value);
    RUN_TEST(c00_reset_pending_substate);
    RUN_TEST(c00_negative_value);

    printf("\nFUN_00479c20 (Set both pending state values) Tests:\n");
    RUN_TEST(c20_set_both_pending_zero);
    RUN_TEST(c20_set_both_pending_values);
    RUN_TEST(c20_overwrite_existing);
    RUN_TEST(c20_negative_values);
    RUN_TEST(c20_large_values);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
