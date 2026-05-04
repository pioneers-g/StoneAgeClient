/*
 * Stone Age Client - Party Flag Clear Functions Unit Tests
 * Tests for FUN_00478150 - Clear party flag bit 0x800
 * Simple bit manipulation function
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

/* Simulated global flag variable */
static s32 DAT_0462bf2c = 0;

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
}

/*
 * FUN_00478150 - Clear party flag bit 0x800 (bit 11)
 * Clears bit 11 of DAT_0462bf2c
 */
static void FUN_00478150(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffff7ff;
}

/* ========================================
 * Test Cases for FUN_00478150
 * ======================================== */

TEST(clear_bit_already_clear) {
    reset_test_state();
    DAT_0462bf2c = 0;

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_set) {
    reset_test_state();
    DAT_0462bf2c = 0x800;

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_preserves_lower_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x7FF;  /* All bits below bit 11 */

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0x7FF);
}

TEST(clear_bit_preserves_upper_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFF800;  /* Bit 11 and above */

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0xFFFFF000);  /* Bit 11 cleared */
}

TEST(clear_bit_preserves_bit_12_and_above) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFFFF;

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0xFFFFF7FF);
}

TEST(clear_bit_mixed_flags) {
    reset_test_state();
    DAT_0462bf2c = 0x00001800;  /* Bits 11 and 12 set */

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0x00001000);  /* Only bit 12 remains */
}

TEST(clear_bit_multiple_calls) {
    reset_test_state();
    DAT_0462bf2c = 0x800;

    FUN_00478150();
    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_with_other_flag_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x10000 | 0x800;  /* Bits 16 and 11 */

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0x10000);  /* Bit 16 preserved */
}

TEST(clear_bit_preserves_all_except_11) {
    reset_test_state();
    DAT_0462bf2c = 0xABCDF7FF;  /* Pattern without bit 11 */

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0xABCDF7FF);
}

TEST(clear_bit_pattern_with_bit_11) {
    reset_test_state();
    DAT_0462bf2c = 0xABCDFFFF;

    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0xABCDF7FF);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Party Flag Clear Functions Unit Tests ===\n\n");

    printf("FUN_00478150 (Clear Party Flag Bit 0x800) Tests:\n");
    RUN_TEST(clear_bit_already_clear);
    RUN_TEST(clear_bit_set);
    RUN_TEST(clear_bit_preserves_lower_bits);
    RUN_TEST(clear_bit_preserves_upper_bits);
    RUN_TEST(clear_bit_preserves_bit_12_and_above);
    RUN_TEST(clear_bit_mixed_flags);
    RUN_TEST(clear_bit_multiple_calls);
    RUN_TEST(clear_bit_with_other_flag_bits);
    RUN_TEST(clear_bit_preserves_all_except_11);
    RUN_TEST(clear_bit_pattern_with_bit_11);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
