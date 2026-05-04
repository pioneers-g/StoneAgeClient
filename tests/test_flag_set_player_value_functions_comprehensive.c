/*
 * Stone Age Client - Flag Set and Player Value Functions Unit Tests
 * Tests for FUN_004780b0, FUN_00477ca0 - Simple setter functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef uintptr_t uintptr;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global variables */
static s32 DAT_0462bf2c = 0;
static s32 DAT_0462be90 = 0;

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
    DAT_0462bf2c = 0;
    DAT_0462be90 = 0;
}

/*
 * FUN_004780b0 - Set flag bit 9 (0x200)
 */
static void FUN_004780b0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x200;
}

/*
 * FUN_00477ca0 - Set player value
 */
static void FUN_00477ca0(u32 param_1) {
    DAT_0462be90 = param_1;
}

/* ========================================
 * Test Cases for FUN_004780b0
 * ======================================== */

TEST(set_bit_9_from_zero) {
    reset_test_state();

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_bit_9_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_bit_9_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFDFF;

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0xFFFFFFFF);
}

TEST(set_bit_9_with_lower_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x1FF;

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x3FF);
}

TEST(set_bit_9_multiple_calls) {
    reset_test_state();

    FUN_004780b0();
    FUN_004780b0();
    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

/* ========================================
 * Test Cases for FUN_00477ca0
 * ======================================== */

TEST(set_player_value_zero) {
    reset_test_state();
    DAT_0462be90 = 999;

    FUN_00477ca0(0);

    ASSERT(DAT_0462be90 == 0);
}

TEST(set_player_value_positive) {
    reset_test_state();

    FUN_00477ca0(12345);

    ASSERT(DAT_0462be90 == 12345);
}

TEST(set_player_value_negative) {
    reset_test_state();

    FUN_00477ca0((u32)-100);

    ASSERT((s32)DAT_0462be90 == -100);
}

TEST(set_player_value_overwrites) {
    reset_test_state();
    DAT_0462be90 = 555;

    FUN_00477ca0(777);

    ASSERT(DAT_0462be90 == 777);
}

TEST(set_player_value_max) {
    reset_test_state();

    FUN_00477ca0(0xFFFFFFFF);

    ASSERT(DAT_0462be90 == 0xFFFFFFFF);
}

TEST(set_player_value_multiple_times) {
    reset_test_state();

    FUN_00477ca0(100);
    ASSERT(DAT_0462be90 == 100);

    FUN_00477ca0(200);
    ASSERT(DAT_0462be90 == 200);

    FUN_00477ca0(300);
    ASSERT(DAT_0462be90 == 300);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Flag Set and Player Value Functions Unit Tests ===\n\n");

    printf("FUN_004780b0 (Set Flag Bit 9) Tests:\n");
    RUN_TEST(set_bit_9_from_zero);
    RUN_TEST(set_bit_9_already_set);
    RUN_TEST(set_bit_9_preserves_other_bits);
    RUN_TEST(set_bit_9_with_lower_bits);
    RUN_TEST(set_bit_9_multiple_calls);

    printf("\nFUN_00477ca0 (Set Player Value) Tests:\n");
    RUN_TEST(set_player_value_zero);
    RUN_TEST(set_player_value_positive);
    RUN_TEST(set_player_value_negative);
    RUN_TEST(set_player_value_overwrites);
    RUN_TEST(set_player_value_max);
    RUN_TEST(set_player_value_multiple_times);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
