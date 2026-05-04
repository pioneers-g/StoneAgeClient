/*
 * Stone Age Client - Utility Functions Unit Tests
 * Tests for FUN_004792c0, FUN_004789a0, and related functions
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

static u32 DAT_0462e3b4 = 0;
static char DAT_04630a00[136] = {0};  /* Two 68-byte slots for name data */

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
    memset(DAT_04630a00, 0, sizeof(DAT_04630a00));
}

/*
 * FUN_004792c0 - Calculate time-based value
 * Returns: DAT_0462e3b4 * 1800000 + 1000000
 */
static int FUN_004792c0(void) {
    return DAT_0462e3b4 * 1800000 + 1000000;
}

/*
 * FUN_004789a0 - Check if slot has name
 * param_1: slot index (0 or 1)
 * Returns: 1 if slot has name, 0 if not, 0xFFFFFFFF if invalid index
 */
static u32 FUN_004789a0(int param_1) {
    if ((-1 < param_1) && (param_1 < 2)) {
        return (u32)(DAT_04630a00[param_1 * 0x44] != '\0');
    }
    return 0xffffffff;
}

/* Helper function to set slot name */
static void set_slot_name(int slot, const char* name) {
    strncpy(&DAT_04630a00[slot * 0x44], name, 16);
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_004792c0 tests */
TEST(calc_time_value_zero) {
    reset_test_state();
    DAT_0462e3b4 = 0;

    int result = FUN_004792c0();

    ASSERT(result == 1000000);
}

TEST(calc_time_value_one) {
    reset_test_state();
    DAT_0462e3b4 = 1;

    int result = FUN_004792c0();

    ASSERT(result == 2800000);
}

TEST(calc_time_value_two) {
    reset_test_state();
    DAT_0462e3b4 = 2;

    int result = FUN_004792c0();

    ASSERT(result == 4600000);
}

TEST(calc_time_value_ten) {
    reset_test_state();
    DAT_0462e3b4 = 10;

    int result = FUN_004792c0();

    ASSERT(result == 19000000);
}

TEST(calc_time_value_max) {
    reset_test_state();
    DAT_0462e3b4 = 1000;

    int result = FUN_004792c0();

    ASSERT(result == 1801000000);
}

/* FUN_004789a0 tests */
TEST(slot_check_invalid_negative) {
    reset_test_state();

    u32 result = FUN_004789a0(-1);

    ASSERT(result == 0xffffffff);
}

TEST(slot_check_invalid_positive) {
    reset_test_state();

    u32 result = FUN_004789a0(2);

    ASSERT(result == 0xffffffff);
}

TEST(slot_check_invalid_large) {
    reset_test_state();

    u32 result = FUN_004789a0(100);

    ASSERT(result == 0xffffffff);
}

TEST(slot_check_slot0_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0);
}

TEST(slot_check_slot1_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(1);

    ASSERT(result == 0);
}

TEST(slot_check_slot0_has_name) {
    reset_test_state();
    set_slot_name(0, "TestName");

    u32 result = FUN_004789a0(0);

    ASSERT(result == 1);
}

TEST(slot_check_slot1_has_name) {
    reset_test_state();
    set_slot_name(1, "TestName");

    u32 result = FUN_004789a0(1);

    ASSERT(result == 1);
}

TEST(slot_check_both_slots) {
    reset_test_state();
    set_slot_name(0, "Name1");
    set_slot_name(1, "Name2");

    u32 result0 = FUN_004789a0(0);
    u32 result1 = FUN_004789a0(1);

    ASSERT(result0 == 1);
    ASSERT(result1 == 1);
}

TEST(slot_check_independent) {
    reset_test_state();
    set_slot_name(0, "Name1");

    u32 result0 = FUN_004789a0(0);
    u32 result1 = FUN_004789a0(1);

    ASSERT(result0 == 1);
    ASSERT(result1 == 0);
}

TEST(slot_check_after_clear) {
    reset_test_state();
    set_slot_name(0, "TestName");
    DAT_04630a00[0] = '\0';  /* Clear the name */

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0);
}

/* Edge case tests */
TEST(slot_check_boundary_0) {
    reset_test_state();

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0 || result == 1);  /* Valid result for valid index */
}

TEST(slot_check_boundary_1) {
    reset_test_state();

    u32 result = FUN_004789a0(1);

    ASSERT(result == 0 || result == 1);  /* Valid result for valid index */
}

TEST(calc_time_formula_verify) {
    reset_test_state();

    /* Verify formula: result = multiplier * 1800000 + 1000000 */
    for (int i = 0; i < 5; i++) {
        DAT_0462e3b4 = i;
        int result = FUN_004792c0();
        int expected = i * 1800000 + 1000000;
        ASSERT(result == expected);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Utility Functions Unit Tests ===\n\n");

    printf("FUN_004792c0 (Calculate Time Value) Tests:\n");
    RUN_TEST(calc_time_value_zero);
    RUN_TEST(calc_time_value_one);
    RUN_TEST(calc_time_value_two);
    RUN_TEST(calc_time_value_ten);
    RUN_TEST(calc_time_value_max);

    printf("\nFUN_004789a0 (Slot Name Check) Tests:\n");
    RUN_TEST(slot_check_invalid_negative);
    RUN_TEST(slot_check_invalid_positive);
    RUN_TEST(slot_check_invalid_large);
    RUN_TEST(slot_check_slot0_empty);
    RUN_TEST(slot_check_slot1_empty);
    RUN_TEST(slot_check_slot0_has_name);
    RUN_TEST(slot_check_slot1_has_name);
    RUN_TEST(slot_check_both_slots);
    RUN_TEST(slot_check_independent);
    RUN_TEST(slot_check_after_clear);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(slot_check_boundary_0);
    RUN_TEST(slot_check_boundary_1);
    RUN_TEST(calc_time_formula_verify);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
