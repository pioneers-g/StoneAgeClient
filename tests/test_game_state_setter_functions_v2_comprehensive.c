/*
 * Stone Age Client - Game State Setter Functions Unit Tests
 * Tests for FUN_00479bc0, FUN_00479be0 - Game state management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated game state variables */
static s32 DAT_04630dd8 = 0;
static s32 DAT_04630df0 = 0;
static s32 DAT_04630df8 = 0;
static s32 DAT_04630de8 = 0;

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
    DAT_04630df8 = 0;
    DAT_04630de8 = 0;
}

/*
 * FUN_00479bc0 - Set game state
 * Sets DAT_04630df0 = 0, DAT_04630dd8 = param_1
 */
static void FUN_00479bc0(u32 param_1) {
    DAT_04630df0 = 0;
    DAT_04630dd8 = param_1;
}

/*
 * FUN_00479be0 - Set game state with sub-state
 * Sets DAT_04630dd8 = param_1, DAT_04630df0 = param_2
 */
static void FUN_00479be0(u32 param_1, u32 param_2) {
    DAT_04630dd8 = param_1;
    DAT_04630df0 = param_2;
}

/*
 * FUN_00479c00 - Set game state 2
 * Sets DAT_04630de8 = 0, DAT_04630df8 = param_1
 */
static void FUN_00479c00(u32 param_1) {
    DAT_04630de8 = 0;
    DAT_04630df8 = param_1;
}

/*
 * FUN_00479c20 - Set game state 2 with sub-state
 * Sets DAT_04630df8 = param_1, DAT_04630de8 = param_2
 */
static void FUN_00479c20(u32 param_1, u32 param_2) {
    DAT_04630df8 = param_1;
    DAT_04630de8 = param_2;
}

/* ========================================
 * Test Cases for FUN_00479bc0
 * ======================================== */

TEST(set_state_zero) {
    reset_test_state();
    DAT_04630df0 = 100;  /* Pre-set to non-zero */

    FUN_00479bc0(0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_one) {
    reset_test_state();

    FUN_00479bc0(1);

    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_five) {
    reset_test_state();

    FUN_00479bc0(5);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_ten) {
    reset_test_state();

    FUN_00479bc0(10);

    ASSERT(DAT_04630dd8 == 10);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_max) {
    reset_test_state();

    FUN_00479bc0(0xFFFFFFFF);

    ASSERT(DAT_04630dd8 == 0xFFFFFFFF);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_resets_df0) {
    reset_test_state();
    DAT_04630df0 = 999;

    FUN_00479bc0(3);

    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_overwrites) {
    reset_test_state();
    DAT_04630dd8 = 5;
    DAT_04630df0 = 100;

    FUN_00479bc0(7);

    ASSERT(DAT_04630dd8 == 7);
    ASSERT(DAT_04630df0 == 0);
}

/* ========================================
 * Test Cases for FUN_00479be0
 * ======================================== */

TEST(set_state_sub_zero_zero) {
    reset_test_state();

    FUN_00479be0(0, 0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_sub_one_zero) {
    reset_test_state();

    FUN_00479be0(1, 0);

    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_sub_one_one) {
    reset_test_state();

    FUN_00479be0(1, 1);

    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 1);
}

TEST(set_state_sub_five_hundred) {
    reset_test_state();

    FUN_00479be0(5, 500);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 500);
}

TEST(set_state_sub_ten_hundred) {
    reset_test_state();

    FUN_00479be0(10, 100);

    ASSERT(DAT_04630dd8 == 10);
    ASSERT(DAT_04630df0 == 100);
}

TEST(set_state_sub_max_values) {
    reset_test_state();

    FUN_00479be0(0xFFFFFFFF, 0xFFFFFFFF);

    ASSERT(DAT_04630dd8 == 0xFFFFFFFF);
    ASSERT(DAT_04630df0 == 0xFFFFFFFF);
}

TEST(set_state_sub_overwrites) {
    reset_test_state();
    DAT_04630dd8 = 100;
    DAT_04630df0 = 200;

    FUN_00479be0(3, 4);

    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 4);
}

TEST(set_state_sub_multiple_calls) {
    reset_test_state();

    FUN_00479be0(1, 10);
    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 10);

    FUN_00479be0(2, 20);
    ASSERT(DAT_04630dd8 == 2);
    ASSERT(DAT_04630df0 == 20);

    FUN_00479be0(3, 30);
    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 30);
}

/* ========================================
 * Test Cases for FUN_00479c00
 * ======================================== */

TEST(set_state2_zero) {
    reset_test_state();
    DAT_04630de8 = 100;

    FUN_00479c00(0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(set_state2_five) {
    reset_test_state();

    FUN_00479c00(5);

    ASSERT(DAT_04630df8 == 5);
    ASSERT(DAT_04630de8 == 0);
}

TEST(set_state2_resets_de8) {
    reset_test_state();
    DAT_04630de8 = 999;

    FUN_00479c00(3);

    ASSERT(DAT_04630de8 == 0);
}

TEST(set_state2_overwrites) {
    reset_test_state();
    DAT_04630df8 = 100;
    DAT_04630de8 = 200;

    FUN_00479c00(7);

    ASSERT(DAT_04630df8 == 7);
    ASSERT(DAT_04630de8 == 0);
}

/* ========================================
 * Test Cases for FUN_00479c20
 * ======================================== */

TEST(set_state2_sub_zero_zero) {
    reset_test_state();

    FUN_00479c20(0, 0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(set_state2_sub_one_one) {
    reset_test_state();

    FUN_00479c20(1, 1);

    ASSERT(DAT_04630df8 == 1);
    ASSERT(DAT_04630de8 == 1);
}

TEST(set_state2_sub_five_hundred) {
    reset_test_state();

    FUN_00479c20(5, 500);

    ASSERT(DAT_04630df8 == 5);
    ASSERT(DAT_04630de8 == 500);
}

TEST(set_state2_sub_overwrites) {
    reset_test_state();
    DAT_04630df8 = 100;
    DAT_04630de8 = 200;

    FUN_00479c20(3, 4);

    ASSERT(DAT_04630df8 == 3);
    ASSERT(DAT_04630de8 == 4);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Game State Setter Functions Unit Tests ===\n\n");

    printf("FUN_00479bc0 (Set Game State) Tests:\n");
    RUN_TEST(set_state_zero);
    RUN_TEST(set_state_one);
    RUN_TEST(set_state_five);
    RUN_TEST(set_state_ten);
    RUN_TEST(set_state_max);
    RUN_TEST(set_state_resets_df0);
    RUN_TEST(set_state_overwrites);

    printf("\nFUN_00479be0 (Set Game State with Sub-State) Tests:\n");
    RUN_TEST(set_state_sub_zero_zero);
    RUN_TEST(set_state_sub_one_zero);
    RUN_TEST(set_state_sub_one_one);
    RUN_TEST(set_state_sub_five_hundred);
    RUN_TEST(set_state_sub_ten_hundred);
    RUN_TEST(set_state_sub_max_values);
    RUN_TEST(set_state_sub_overwrites);
    RUN_TEST(set_state_sub_multiple_calls);

    printf("\nFUN_00479c00 (Set Game State 2) Tests:\n");
    RUN_TEST(set_state2_zero);
    RUN_TEST(set_state2_five);
    RUN_TEST(set_state2_resets_de8);
    RUN_TEST(set_state2_overwrites);

    printf("\nFUN_00479c20 (Set Game State 2 with Sub-State) Tests:\n");
    RUN_TEST(set_state2_sub_zero_zero);
    RUN_TEST(set_state2_sub_one_one);
    RUN_TEST(set_state2_sub_five_hundred);
    RUN_TEST(set_state2_sub_overwrites);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
